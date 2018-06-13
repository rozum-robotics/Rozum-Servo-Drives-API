#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include "usbcan_proto.h"
#include "usbcan_util.h"
#include "rb_tools.h"
#include "crc16-ccitt.h"
#include "logging.h"

#define TIME_DELTA_MS(x, y) ((x.tv_sec - y.tv_sec) * 1000 + (x.tv_usec - y.tv_usec) / 1000)


FILE *debug_log = NULL;

/*
 * PRIVATE functions
 */

static int usbcan_build_timestamp(uint8_t *dst, uint32_t ts);
static int usbcan_build_hb(uint8_t *dst, int id, usbcan_nmt_state_t state);
static int usbcan_build_nmt(uint8_t *dst, int id, usbcan_nmt_cmd_t cmd);
static int usbcan_build_com_frame(uint8_t *dst,  can_msg_t *m);
static int usbcan_build_sdo_req(uint8_t *dst,
		usbcan_sdo_t *sdo,
		void *data, 
		uint16_t len,
		sdo_resp_cb_t cb);
static int usbcan_rx(usbcan_instance_t *inst);

static int usbcan_send_sdo_req(usbcan_instance_t *inst, usbcan_sdo_t *sdo, void *data, uint16_t len, sdo_resp_cb_t cb);
static void usbcan_send_master_hb(usbcan_instance_t *inst);
static int usbcan_send_com_frame(usbcan_instance_t *inst, can_msg_t *m);
static int usbcan_send_nmt(usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd);
static int usbcan_send_hb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state) __attribute__((unused));
static int usbcan_send_timestamp(usbcan_instance_t *inst, uint32_t ts);

static bool is_valid_instance(const usbcan_instance_t *inst)
{
	if(!inst)
	{
		LOG_ERROR(debug_log, "null interface");
		return false;
	}
	if(!inst->running)
	{
		LOG_ERROR(debug_log, "interface thread not running");
		return false;
	}
	return true;
}

static bool is_valid_device(const usbcan_device_t *dev)
{
	if(!dev)
	{
		LOG_ERROR(debug_log, "null device");
		return false;
	}
	if(!dev->inst)
	{
		LOG_ERROR(debug_log, "device points to null interface");
		return false;
	}
	if(!dev->inst->running)
	{
		LOG_ERROR(debug_log, "device's corresponding interface thread not running");
		return false;
	}
	return true;
}

static int usbcan_write_fd(usbcan_instance_t *inst, uint8_t *b, int l)
{
	int ret;

	if(!inst->usbcan_udp)
	{
		ret = write(inst->fd, b, l);
	}
	else
	{
		ret = write(inst->fd, b + USB_CAN_HEAD_SZ, l - USB_CAN_OHEAD);
	}

	if(ret < 0)
	{
		LOG_ERROR(debug_log, "%s: usbcan write failed", __func__);
	}

	return ret;
}

static void ipc_create_link(usbcan_instance_t *inst)
{
    if(pipe(inst->to_master_pipe))
    {
        LOG_ERROR(debug_log, "%s: can't create master pipe", __func__);
        return;
    }

    if(pipe(inst->to_child_pipe))
    {
        LOG_ERROR(debug_log, "%s: can't create child pipe", __func__);
        return;
    }
}

static size_t read_sig_safe(int fd, void *data, size_t sz)
{
    size_t l, bytes = 0;

    while(sz)
    {
        l = read(fd, ((uint8_t *)data) + bytes, sz);
        if(l >= 0)
        {
            sz -= l;
            bytes += l;
        }
        else
        {
            if(errno == EINTR)
            {
                continue;
            }
            return -1;
        }
    }
    return bytes;
}

static size_t write_sig_safe(int fd, void *data, size_t sz)
{
    size_t l, bytes = 0;

    while(sz)
    {
        l = write(fd, ((uint8_t *)data) + bytes, sz);
        if(l >= 0)
        {
            sz -= l;
            bytes += l;
        }
        else
        {
            if(errno == EINTR)
            {
                continue;
            }
            return -1;
        }
    }
    return bytes;
}

static void sdo_resp_cb(usbcan_instance_t *inst, uint32_t abt, uint8_t *data, int len)
{
    ipc_sdo_resp_t r;

    r.sdo = inst->wait_sdo.sdo;
    r.abt = abt;
    r.data_len = len;
    write_sig_safe(inst->to_master_pipe[1], &r, sizeof(r));
    if(data && len)
    {
        write_sig_safe(inst->to_master_pipe[1], data, len);
    }
}

static void ipc_process(usbcan_instance_t *inst)
{
    ipc_opcode_t opcode;

    read_sig_safe(inst->to_child_pipe[0], &opcode, sizeof(opcode));

    switch(opcode)
    {
    case IPC_SDO:
    {
        ipc_sdo_req_t r;

        read_sig_safe(inst->to_child_pipe[0], &r, sizeof(r));
        uint8_t data[r.data_len];
        read_sig_safe(inst->to_child_pipe[0], data, r.data_len);
        usbcan_send_sdo_req(inst, &r.sdo, data, r.data_len, sdo_resp_cb);
        break;
    }
    case IPC_COM_FRAME:
    {
        can_msg_t msg;

        read_sig_safe(inst->to_child_pipe[0], &msg, sizeof(msg));

        usbcan_send_com_frame(inst, &msg);
        break;
    }
    case IPC_TIMESTAMP:
    {
        uint32_t ts;

        read_sig_safe(inst->to_child_pipe[0], &ts, sizeof(ts));

        usbcan_send_timestamp(inst, ts);
        break;
    }
    case IPC_NMT:
    {
        ipc_nmt_t nmt;

        read_sig_safe(inst->to_child_pipe[0], &nmt, sizeof(nmt));

        usbcan_send_nmt(inst, nmt.id, nmt.cmd);
        break;
    }
	case IPC_WAIT_DEVICE:
	{
		ipc_wait_device_t wd;

		read_sig_safe(inst->to_child_pipe[0], &wd, sizeof(wd));
		inst->wait_device.id = wd.id;
		inst->wait_device.timer = wd.timeout_ms;

		break;
	}
    default:
        break;
    }
}


static void usbcan_enable_udp(usbcan_instance_t *inst, bool en)
{
	inst->usbcan_udp = en;
}


static void usbcan_poll(usbcan_instance_t *inst, int64_t delta_ms)
{
	int i;

	inst->master_hb_timer += delta_ms;

	/*Check if devices on bus*/
	for(i = 0; i < USB_CAN_MAX_DEV; i++)
	{
		if(inst->dev_alive[i] > 0)
		{
			inst->dev_alive[i] -= MIN(delta_ms, inst->dev_alive[i]);
		}
		if(inst->dev_alive[i] == 0)
		{
			inst->dev_alive[i] = -1;
			inst->dev_hb_ival[i] = -1;
			if(inst->usbcan_nmt_state_cb)
			{
				((usbcan_nmt_state_cb_t)inst->usbcan_nmt_state_cb)(inst, i, CO_NMT_HB_TIMEOUT);
			}
		}
	}

	/*Send master heart beat*/
	if(inst->master_hb_timer >= inst->master_hb_ival)
	{
		if(!inst->inhibit_master_hb)
		{
			if(inst->usbcan_hb_tx_cb)
			{
				((usbcan_hb_tx_cb_t)inst->usbcan_hb_tx_cb)(inst);
			}
			usbcan_send_master_hb(inst);
		}
		inst->master_hb_timer -= inst->master_hb_ival;
	}

	/*Wait for SDO responce*/
	if(inst->wait_sdo.cb)
	{
		inst->wait_sdo.sdo.ttl = CLIPL(inst->wait_sdo.sdo.ttl - delta_ms, 0);
		if(!inst->wait_sdo.sdo.ttl)
		{
			((sdo_resp_cb_t)inst->wait_sdo.cb)(inst, -1, NULL, 0);
			inst->wait_sdo.cb = NULL;
		}
	}

	/*Wait for device apeared on bus*/
	if(inst->wait_device.id > 0)
	{
		if(inst->dev_alive[inst->wait_device.id] > 0)
		{
			int resp = 1;
	    	write_sig_safe(inst->to_master_pipe[1], &resp, sizeof(resp));
			inst->wait_device.id = 0;
		}
		else if(inst->wait_device.timer <= 0)
		{
			int resp = 0;
	    	write_sig_safe(inst->to_master_pipe[1], &resp, sizeof(resp));
			inst->wait_device.id = 0;
		}

		inst->wait_device.timer -= delta_ms;
	}
}

static void usbcan_wait_sdo(usbcan_instance_t *inst, usbcan_sdo_t *sdo, sdo_resp_cb_t cb)
{
	inst->wait_sdo.sdo = *sdo;
	inst->wait_sdo.cb = cb;
}

static int usbcan_wrap_inplace(uint8_t *dst, int payload_sz)
{
	uint16_t crc = 0;
	int p = 0;

	set_ux_(dst, &p, 1, USB_CAN_STX);
	set_ux_(dst, &p, 2, payload_sz);
	crc = crc16_ccitt(dst + p, payload_sz, crc);
	p += payload_sz;
	set_ux_(dst, &p, 2, crc);

	return p;
}

static int usbcan_build_nmt(uint8_t *dst, int id, usbcan_nmt_cmd_t cmd)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, COM_NMT);
	set_ux_(msg, &p, 1, id);
	set_ux_(msg, &p, 1, (uint8_t)cmd);

	return usbcan_wrap_inplace(dst, p);
}

static int usbcan_build_hb(uint8_t *dst, int id, usbcan_nmt_state_t state)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, COM_HB);
	set_ux_(msg, &p, 1, id);
	set_ux_(msg, &p, 1, (uint8_t)state);

	return usbcan_wrap_inplace(dst, p);
}

static int usbcan_build_timestamp(uint8_t *dst, uint32_t ts)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, COM_TIMESTAMP);
	set_ux_(msg, &p, sizeof(ts), ts);

	return usbcan_wrap_inplace(dst, p);
}

static int usbcan_build_sdo_req(uint8_t *dst,
		usbcan_sdo_t *sdo,
		void *data, 
		uint16_t len,
		sdo_resp_cb_t cb)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, sdo->write ? COM_SDO_TX_REQ : COM_SDO_RX_REQ);
	set_ux_(msg, &p, 1, sdo->id);
	set_ux_(msg, &p, 2, sdo->idx);
	set_ux_(msg, &p, 1, sdo->sidx);
	set_ux_(msg, &p, 2, (sdo->tout & 0x1FFFU) |(sdo->re_txn & 0x7U) << 13);
	if(sdo->write)
	{
		memcpy(msg + p, data, len);
		p += len;
	}

	return usbcan_wrap_inplace(dst, p);
}

static int usbcan_build_com_frame(uint8_t *dst,  can_msg_t *m)
{
	int idlen = (m->id & 0x40000000u ? 4 : 2);
	//uint16_t total_len = m->dlc + 1 + idlen;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;
	int p = 0;

	set_ux_(msg, &p, 1, COM_FRAME);
	set_ux_(msg, &p, idlen, idlen == 2 ? m->id & 0x7ffu : m->id);
	memcpy(msg + p, m->data, m->dlc);
	p += m->dlc;

	return usbcan_wrap_inplace(dst, p);
}

static int usbcan_send_nmt(usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_nmt(dst, id, cmd);
	LOG_DUMP(inst->comm_log, "SERIAL TX(NMT)", dst, l);
	return usbcan_write_fd(inst, dst, l);
}

static int usbcan_send_hb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_hb(dst, id, state);
	LOG_DUMP(inst->comm_log, "SERIAL TX(HB)", dst, l);
	return usbcan_write_fd(inst, dst, l);
}


static int usbcan_send_timestamp(usbcan_instance_t *inst, uint32_t ts)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_timestamp(dst, ts);
	return usbcan_write_fd(inst, dst, l);
}

static int usbcan_send_sdo_req(usbcan_instance_t *inst, usbcan_sdo_t *sdo, void *data, uint16_t len, sdo_resp_cb_t cb)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_sdo_req(dst, sdo, data, len, cb);
	LOG_DUMP(inst->comm_log, "SERIAL TX(SDO)", dst, l);
	usbcan_wait_sdo(inst, sdo, cb);
	return usbcan_write_fd(inst, dst, l);
}

static int usbcan_send_com_frame(usbcan_instance_t *inst, can_msg_t *m)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_com_frame(dst, m);
	return usbcan_write_fd(inst, dst, l);
}

static void usbcan_send_master_hb(usbcan_instance_t *inst)
{
	can_msg_t msg = {USB_CAN_MASTER_HB_COM_FRAME_ID, 1, {CO_NMT_OPERATIONAL}};

	usbcan_send_com_frame(inst, &msg);
}

static void usbcan_parse_com_frame(can_msg_t *m, uint8_t *msg, int sz)
{
	int p = 0;

	if(get_ux_(msg, &p, 1) != COM_FRAME)
	{
		return;
	}


	if(msg[p] & U32_H8(USB_CAN_EID_FLAG))
	{	
		m->id = get_ux_(msg, &p, 4);
		m->dlc = sz - 4 - USB_CAN_FRAME_TYPE_SZ;
	}
	else
	{
		m->id = get_ux_(msg, &p, 2) & 0x7ffu;
		m->dlc = sz - 2 - USB_CAN_FRAME_TYPE_SZ;
	}
	memcpy(m->data, msg + p, m->dlc);
}


static void usbcan_frame_receive_cb(usbcan_instance_t *inst, uint8_t *data, int len)
{
	switch(data[0])
	{
		case COM_FRAME:
			{
				can_msg_t m;
				usbcan_parse_com_frame(&m, data, len);
				if(inst->usbcan_com_frame_cb)
				{
					((usbcan_com_frame_cb_t)inst->usbcan_com_frame_cb)(inst, &m);
				}
			}
			break;	

		case COM_SDO_TX_REQ:
			{
			}
			break;

		case COM_SDO_RX_REQ:
			{	
			}
			break;

		case COM_SDO_TX_RESP:
			{	
				int p = 0;
				len--;
				data++;
				LOG_DUMP(inst->comm_log, "SDO write resp", data, len);

				uint8_t id = get_ux_(data, &p, 1);
				uint16_t idx = get_ux_(data, &p, 2);
				uint8_t sidx = get_ux_(data, &p, 1);
				uint32_t abt = get_ux_(data, &p, 4);

				if(inst->wait_sdo.cb)
				{
					if((inst->wait_sdo.sdo.id == id) &&
							(inst->wait_sdo.sdo.idx == idx) &&
							(inst->wait_sdo.sdo.sidx == sidx) &&
							(inst->wait_sdo.sdo.write == true))
					{
						((sdo_resp_cb_t)inst->wait_sdo.cb)(inst, abt, NULL, 0);
						inst->wait_sdo.cb = NULL;
					}
				}
			}
			break;

		case COM_SDO_RX_RESP:
			{	
				int p = 0;
				len--;
				data++;
				LOG_DUMP(inst->comm_log, "SDO read resp", data, len);

				uint8_t id = get_ux_(data, &p, 1);
				uint16_t idx = get_ux_(data, &p, 2);
				uint8_t sidx = get_ux_(data, &p, 1);
				uint32_t abt = get_ux_(data, &p, 4);

				LOG_DUMP(inst->comm_log, "SDO read data", data + p, len - p);

				if(inst->wait_sdo.cb)
				{
					if((inst->wait_sdo.sdo.id == id) &&
							(inst->wait_sdo.sdo.idx == idx) &&
							(inst->wait_sdo.sdo.sidx == sidx) &&
							(inst->wait_sdo.sdo.write == false))
					{
						((sdo_resp_cb_t)inst->wait_sdo.cb)(inst, abt, data + p, len - p);
						inst->wait_sdo.cb = NULL;
					}
				}
			}
			break;

		case COM_PDO:
			break;

		case COM_HB:
			{
				len--;
				data++;
				LOG_DUMP(inst->comm_log, "SERIAL RX(HB)", data, len);
				int p = 0;
				uint8_t id = get_ux_(data, &p, 1) & 0x7f;
				usbcan_nmt_state_t state = get_ux_(data, &p, 1);

				if((inst->dev_state[id] != state) || (inst->dev_alive[id] == -1))
				{
					inst->dev_state[id] = state;
					if(inst->usbcan_nmt_state_cb)
					{
						((usbcan_nmt_state_cb_t)inst->usbcan_nmt_state_cb)(inst, id, state);
					}
				}

				inst->dev_hb_ival[id] = inst->hb_alive_threshold - inst->dev_alive[id];
				inst->dev_alive[id] = inst->hb_alive_threshold;

				if(inst->usbcan_hb_rx_cb)
				{
					((usbcan_hb_rx_cb_t)inst->usbcan_hb_rx_cb)(inst, id, state);
				}
			}
			break;

		case COM_NMT:
			break;

		case COM_TIMESTAMP:
			break;

		case COM_SYNC:
			break;
		case COM_EMCY:
			{
				len--;
				data++;
				LOG_DUMP(inst->comm_log, "SERIAL RX(EMCY)", data, len);
				int p = 0;
				uint8_t id = get_ux_(data, &p, 1);
				uint16_t err_code = get_ux_(data, &p, 2);
				uint8_t err_reg = get_ux_(data, &p, 1);
				uint8_t err_bits = get_ux_(data, &p, 1);
				uint32_t err_info = get_ux_(data, &p, 4);
				if(inst->usbcan_emcy_cb)
				{
					((usbcan_emcy_cb_t)inst->usbcan_emcy_cb)(inst, id, err_code, err_reg, err_bits, err_info);
				}
			}
			break;
	}
}

static int usbcan_rx(usbcan_instance_t *inst)
{
	static uint8_t rb[USB_CAN_MAX_PAYLOAD];
	static int h = 0, t = 0;

	uint8_t b[USB_CAN_MAX_PAYLOAD];
	int l = read(inst->fd, b, sizeof(b));
	if(l < 0)
	{
		LOG_ERROR(debug_log, "%s: usbcan read failed", __func__);
		return l;
	}
	
	if(inst->usbcan_udp)
	{
		usbcan_frame_receive_cb(inst, b, l);
		return l;
	}
	

	h = rb_to_rb(rb, h, sizeof(rb), b, 0, l, l);


	while(1)
	{
		while(t != h)
		{
			if(rb[t] == USB_CAN_STX)
			{
				break;
			}
			t = (t + 1) % sizeof(rb);
			LOG_WARN(debug_log, "%s: malformed packed, skipping", __func__);
		}

		if(rb_dist(h, t, sizeof(rb)) >= 3)
		{
			uint16_t elen = 0;
			uint16_t ecrc = 0;

			elen = rb[(t + 1) % sizeof(rb)] << 8 | rb[(t + 2) % sizeof(rb)];

			if(elen >= sizeof(rb))
			{
				LOG_WARN(debug_log, "%s: too long message %d", __func__, elen);
				t = (t + 1) % sizeof(rb);
			}

			if(rb_dist(h, t, sizeof(rb)) >= (3 + elen + 2))
			{
				uint16_t crc = 0;
				uint8_t pload[sizeof(rb)];

				ecrc = rb[(t + 3 + elen) % sizeof(rb)] << 8 | rb[(t + 4 + elen) % sizeof(rb)];
				rb_to_rb(pload, 0, sizeof(pload), rb, t + 3, sizeof(rb), elen);
				crc = crc16_ccitt(pload, elen, crc);
				if(crc == ecrc)
				{
					usbcan_frame_receive_cb(inst, pload, elen);
					t = (t + 5 + elen) % sizeof(rb);
				}
				else
				{
					LOG_WARN(debug_log, "%s: crc error %x != %x\n", __func__, ecrc, crc);
					t = (t + 1) % sizeof(rb);
				}
				continue;
			}
		}
		break;
	}
	return l;
}


static void emcy_cb(usbcan_instance_t *inst, int id, uint16_t code, uint8_t reg, uint8_t bits, uint32_t info)
{
	LOG_WARN(debug_log, "Emergency frame received: id(%"PRId8") code(0x%"PRIX16") reg(0x%"PRIX8") bits(0x%"PRIX8") info(0x%"PRIX32")",
			id, code, reg, bits, info);
}

static void hb_tx_cb(usbcan_instance_t *inst)
{
}

static void hb_rx_cb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
}

static void nmt_state_cb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
	if(state == CO_NMT_HB_TIMEOUT)
	{
		LOG_WARN(debug_log, "Devide %d disappeared from bus", id);
	}
	else
	{
		LOG_INFO(debug_log, "Device %d state changed to %d", id, state);
	}
}

static void usbcan_flush_device(usbcan_instance_t *inst)
{
	if(inst->fd < 0)
	{
		return;
	}

	struct timeval tprev, tnow;
	struct pollfd pfds[1];

	pfds[0].fd = inst->fd;
	pfds[0].events = POLLIN | POLLERR;

	gettimeofday(&tnow, NULL);
	tprev = tnow;
	
	for(int t = USB_CAN_FLUSH_TOUT_MS; t > 0;)
	{
		int n;

		n = poll(pfds, 1, USB_CAN_POLL_GRANULARITY_MS);
		gettimeofday(&tnow, NULL);
		if(n > 0)
		{
			if(pfds[0].revents & POLLERR)
			{
				LOG_ERROR(debug_log, "%s: poll failed", __func__);
				inst->fd = -1;
				return;
			}
			if(pfds[0].revents & POLLIN)
			{
				uint8_t discard[USB_CAN_MAX_PAYLOAD];
				if(read(inst->fd, discard, sizeof(discard)) < 0)
				{
					LOG_ERROR(debug_log, "%s: read failed", __func__);
					inst->fd = -1;
					return;
				}
			}
		}
		else
		{
			break;
		}
		t -= TIME_DELTA_MS(tnow, tprev);
		tprev = tnow;
	}
}

static void usbcan_open_device(usbcan_instance_t *inst)
{
	struct termios term;
	int flags;
	struct sockaddr_in host_addr;
	FILE *f = NULL;
	struct in_addr addr;

	if(inet_aton(inst->device, &addr) != 0)
	{
		usbcan_enable_udp(inst, true);
		inst->fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(inst->fd < 0)
		{
			LOG_ERROR(debug_log, "%s: can't create UDP socket", __func__);
			return;
		}
		host_addr.sin_family = AF_INET;
		host_addr.sin_port = htons(USB_CAN_OUTGOING_UDP_PORT);
		host_addr.sin_addr.s_addr = INADDR_ANY;
		if(bind(inst->fd, (const struct sockaddr *)&host_addr, sizeof(struct sockaddr_in)) < 0)
		{
			LOG_ERROR(debug_log, "%s: can't bind to UDP port", __func__);
			inst->fd = -1;
			return;
		}
		host_addr.sin_family = AF_INET;
		host_addr.sin_port = htons(USB_CAN_INGOING_UDP_PORT);
		host_addr.sin_addr.s_addr = addr.s_addr;
		if(connect(inst->fd, (const struct sockaddr *)&host_addr, sizeof(struct sockaddr_in)) < 0)
		{
			LOG_ERROR(debug_log, "%s: can't connect to UDP port", __func__);
			inst->fd = -1;
			return;
		}

		if(write(inst->fd, "hello", 5) != 5)
		{
			LOG_ERROR(debug_log, "%s: can't write to UDP port", __func__);
			inst->fd = -1;
			return;
		}
	}
	else if((f = fopen(inst->device, "r+")) != NULL)
	{
		inst->fd = fileno(f);
		tcgetattr(inst->fd, &term);
		cfmakeraw(&term);
		tcsetattr(inst->fd, TCSANOW, &term);
		flags = fcntl(inst->fd, F_GETFL, 0);
		fcntl(inst->fd, F_SETFL, flags | O_NOCTTY);

		tcflush(inst->fd, TCIOFLUSH);
	}
	else
	{
		LOG_ERROR(debug_log, "%s: can't open serial device %s", __func__, inst->device);
		inst->fd = -1;
		return;
	}

	usbcan_flush_device(inst);
}

static void *usbcan_process(void *udata)
{
	struct timeval tprev, tnow;
	int n = 0;
	struct pollfd pfds[2];

	usbcan_instance_t *inst = (usbcan_instance_t *)udata;

	usbcan_setup_hb_tx_cb(inst, hb_tx_cb, 250);
	usbcan_setup_hb_rx_cb(inst, hb_rx_cb);
	usbcan_setup_emcy_cb(inst, emcy_cb);
	usbcan_setup_nmt_state_cb(inst, nmt_state_cb);

	do
	{
		usbcan_open_device(inst);
		if(inst->fd < 0)
		{
			LOG_ERROR(debug_log, "%s: open device '%s' failed", __func__, inst->device);
			break;
		}

		pfds[0].fd = inst->to_child_pipe[0];
		pfds[0].events = POLLIN;

		pfds[1].fd = inst->fd;
		pfds[1].events = POLLIN | POLLERR;

		gettimeofday(&tnow, NULL);
		tprev = tnow;

		inst->running = true;

		while(1)
		{
			n = poll(pfds, 2, USB_CAN_POLL_GRANULARITY_MS);
			gettimeofday(&tnow, NULL);
			usbcan_poll(inst, TIME_DELTA_MS(tnow, tprev));
			if(n > 0)
			{
				if(pfds[0].revents & POLLIN)
				{
					ipc_process(inst);
				}

				if(pfds[1].revents & POLLERR)
				{
					LOG_ERROR(debug_log, "%s: poll failed", __func__);
					break;
				}
				if(pfds[1].revents & POLLIN)
				{
					if(usbcan_rx(inst) < 0)
					{
						LOG_ERROR(debug_log, "%s: read failed", __func__);
						break;
					}
				}
			}
			tprev = tnow;
		}
		close(inst->fd);
	}
	while(0);

	inst->running = false;
	LOG_ERROR(debug_log, "%s: thread finished", __func__);

	return 0;
}

/*
 * PUBLIC functions
 */

void usbcan_setup_hb_tx_cb(usbcan_instance_t *inst, usbcan_hb_tx_cb_t cb, int64_t to)
{
	inst->master_hb_ival = to;
	inst->usbcan_hb_tx_cb = cb;
}

void usbcan_setup_hb_rx_cb(usbcan_instance_t *inst, usbcan_hb_rx_cb_t cb)
{
	inst->usbcan_hb_rx_cb = cb;
}

void usbcan_setup_emcy_cb(usbcan_instance_t *inst, usbcan_emcy_cb_t cb)
{
	inst->usbcan_emcy_cb = cb;
}

void usbcan_setup_nmt_state_cb(usbcan_instance_t *inst, usbcan_nmt_state_cb_t cb)
{
	inst->usbcan_nmt_state_cb = cb;
}

void usbcan_setup_com_frame_cb(usbcan_instance_t *inst, usbcan_com_frame_cb_t cb)
{
	inst->usbcan_com_frame_cb = cb;
}

int64_t usbcan_get_hb_interval(usbcan_instance_t *inst, int id)
{
	if(INRANGE(id, 0, USB_CAN_MAX_DEV))
	{
		return inst->dev_hb_ival[id];
	}
	return 0;
}

usbcan_nmt_state_t usbcan_get_device_state(usbcan_instance_t *inst, int id)
{
	if(INRANGE(id, 0, USB_CAN_MAX_DEV))
	{
		return inst->dev_state[id];
	}
	return CO_NMT_HB_TIMEOUT;
}

void usbcan_inhibit_master_hb(usbcan_instance_t *inst, bool inh)
{
	inst->inhibit_master_hb = inh;
}


void usbcan_set_comm_log_stream(usbcan_instance_t *inst, FILE *f)
{
	inst->comm_log = f;
}

void usbcan_set_debug_log_stream(FILE *f)
{
	debug_log = f;	
}

usbcan_instance_t *usbcan_instance_init(const char *dev_name)
{
	int i;

	usbcan_instance_t *inst = malloc(sizeof(usbcan_instance_t));
	if(!inst)
	{
		LOG_WARN(debug_log, "%s: can't allocate interface instance", __func__);
		return NULL;
	}
	memset(inst, 0, sizeof(usbcan_instance_t));
	inst->master_hb_ival = USB_CAN_MASTER_HB_IVAL_MS;
	inst->hb_alive_threshold = USB_CAN_HB_ALIVE_THRESHOLD_MS;
	inst->device = dev_name;

	for(i = 0; i < USB_CAN_MAX_DEV; i++)
	{
		inst->dev_alive[i] = -1;
		inst->dev_hb_ival[i] = -1;
		inst->dev_state[i] = CO_NMT_HB_TIMEOUT;
	}

	ipc_create_link(inst);

	if(pthread_create(&inst->usbcan_thread, NULL, usbcan_process, inst))
	{
		LOG_WARN(debug_log, "%s: can't run thread", __func__);
		free(inst);
		return NULL;
	}

	return inst;
}

int usbcan_instance_deinit(usbcan_instance_t **inst)
{
	if(*inst)
	{
		for(usbcan_device_t *dev = (*inst)->device_list; dev; dev = dev->next)
		{
			dev->inst = NULL;
		}

		pthread_cancel((*inst)->usbcan_thread);
		free(*inst);
		*inst = NULL;
		return 1;
	}
	else
	{
		LOG_WARN(debug_log, "%s: 'null' interface instance", __func__);
	}
	return 0;
}

usbcan_device_t *usbcan_device_init(usbcan_instance_t *inst, int id)
{
	if(!inst)
	{
		LOG_WARN(debug_log, "%s: can't init device with 'null' interface", __func__);
		return 0;
	}
	usbcan_device_t *dev = malloc(sizeof(usbcan_device_t));
	if(!dev)
	{
		LOG_WARN(debug_log, "%s: can't allocate device instance", __func__);
		return 0;
	}
	memset(dev, 0, sizeof(usbcan_device_t));
	dev->inst = inst;
	dev->id = id;
	dev->timeout = 1000;
	dev->retry = 1;

	usbcan_device_t *next_dev = inst->device_list;
	inst->device_list = dev;
	dev->next = next_dev;

	return dev;
}

int usbcan_device_deinit(usbcan_device_t **dev)
{
	if(*dev)
	{
		usbcan_instance_t *inst = (*dev)->inst;

		if(!inst)
		{
			LOG_WARN(debug_log, "%s: 'null' interface instance", __func__);
			return 0;
		}
		usbcan_device_t *prev_dev;

		for(prev_dev = inst->device_list; prev_dev; prev_dev = prev_dev->next)
		{
			if(prev_dev->next == *dev)
			{
				break;
			}
		}
		if(prev_dev)
		{
			prev_dev->next = (*dev)->next;
		}
		else
		{
			LOG_WARN(debug_log, "%s: there is no such device in interface", __func__);
		}
		free(*dev);
		*dev = NULL;	

		return 1;
	}
	return 0;
}

/*
 * User thread functions
 */

int wait_device(const usbcan_instance_t *inst, int id, int timeout_ms)
{
	if(!inst)
	{
		return 0;
	}

	if(id <= 0)
	{
		return 0;
	}

	if(timeout_ms <= 0)
	{
		return 1;
	}

    ipc_opcode_t opcode = IPC_WAIT_DEVICE;
    ipc_wait_device_t wd = {.id = id, .timeout_ms = timeout_ms};
	int resp;

    write_sig_safe(inst->to_child_pipe[1], &opcode, sizeof(opcode));
    write_sig_safe(inst->to_child_pipe[1], &wd, sizeof(wd));

    read_sig_safe(inst->to_master_pipe[0], &resp, sizeof(resp));

	if(!resp)
	{
		LOG_WARN(debug_log, "%s: device sent no heart beats during tmeout (%d) period", __func__, timeout_ms);
	}

	return resp ;
}

int write_nmt(const usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd)
{
	if(!is_valid_instance(inst))
	{
		return 0;
	}

    ipc_opcode_t opcode = IPC_NMT;
    ipc_nmt_t nmt = {.id = id, .cmd = cmd};

    write_sig_safe(inst->to_child_pipe[1], &opcode, sizeof(opcode));
    write_sig_safe(inst->to_child_pipe[1], &nmt, sizeof(nmt));

	return 1;
}

int write_timestamp(const usbcan_instance_t *inst, uint32_t ts)
{
	if(!is_valid_instance(inst))
	{
		return 0;
	}

    ipc_opcode_t opcode = IPC_TIMESTAMP;

    write_sig_safe(inst->to_child_pipe[1], &opcode, sizeof(opcode));
    write_sig_safe(inst->to_child_pipe[1], &ts, sizeof(ts));

	return 1;
}

int write_com_frame(const usbcan_instance_t *inst, can_msg_t *msg)
{
	if(!is_valid_instance(inst))
	{
		return 0;
	}

    ipc_opcode_t opcode = IPC_COM_FRAME;

    write_sig_safe(inst->to_child_pipe[1], &opcode, sizeof(opcode));
    write_sig_safe(inst->to_child_pipe[1], msg, sizeof(can_msg_t));

	return 1;
}

uint32_t write_raw_sdo(const usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int len, int retry, int timeout_ms)
{
	if(!is_valid_device(dev))
	{
		return -1;
	}

    ipc_sdo_req_t req;
    ipc_sdo_resp_t resp;
    ipc_opcode_t opcode = IPC_SDO;

    req.sdo.write = true;
    req.sdo.id = dev->id;
    req.sdo.idx = idx;
    req.sdo.sidx = sidx;
    req.sdo.tout = timeout_ms ? timeout_ms : dev->timeout;
    req.sdo.re_txn = retry ? retry : dev->retry;
    req.sdo.ttl = req.sdo.tout * 2; //????????????????
    req.data_len = len;

    write_sig_safe(dev->inst->to_child_pipe[1], &opcode, sizeof(opcode));
    write_sig_safe(dev->inst->to_child_pipe[1], &req, sizeof(req));
    
    if(data && len)
    {
        write_sig_safe(dev->inst->to_child_pipe[1], data, len);
    }
    
    read_sig_safe(dev->inst->to_master_pipe[0], &resp, sizeof(resp));
    if(resp.abt)
    {
        LOG_ERROR(debug_log, "%s: SDO failed idx(0x%X) sidx(%d), len(%d), re_txn(%d), tout(%d) with abort-code(0x%.X):\n    %s", __func__,
                  (unsigned int)idx, (int)sidx, len, req.sdo.re_txn, req.sdo.tout, (unsigned int)resp.abt, sdo_describe_error(resp.abt));
    }

    return resp.abt;
}

uint32_t read_raw_sdo(const usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int *len, int retry, int timeout_ms)
{
	if(!is_valid_device(dev))
	{
		return -1;
	}

    ipc_sdo_req_t req;
    ipc_sdo_resp_t resp;
    ipc_opcode_t opcode = IPC_SDO;

    req.sdo.write = false;
    req.sdo.id = dev->id;
    req.sdo.idx = idx;
    req.sdo.sidx = sidx;
    req.sdo.tout = timeout_ms ? timeout_ms : dev->timeout;
    req.sdo.re_txn = retry ? retry : dev->retry;
    req.sdo.ttl = req.sdo.tout * 2; //????????????????
    req.data_len = 0;

    write_sig_safe(dev->inst->to_child_pipe[1], &opcode, sizeof(opcode));
    write_sig_safe(dev->inst->to_child_pipe[1], &req, sizeof(req));

    read_sig_safe(dev->inst->to_master_pipe[0], &resp, sizeof(resp));

    if(!resp.abt)
    {
        if(resp.data_len > *len)
        {
            LOG_WARN(debug_log, "%s: supplied buffer of %d bytes to small, %d bytes required", __func__, *len, resp.data_len);
        }
        else
        {
            *len = resp.data_len;
        }
        read_sig_safe(dev->inst->to_master_pipe[0], data, *len);
    }
    else
    {
        LOG_ERROR(debug_log, "%s: SDO failed idx(0x%X) sidx(%d), len(%d), re_txn(%d), tout(%d) with abort-code(0x%.X):\n    %s", __func__,
                  (unsigned int)idx, (int)sidx, len, req.sdo.re_txn, req.sdo.tout, (unsigned int)resp.abt, sdo_describe_error(resp.abt));
    }

    return resp.abt;
}



