#include <string.h>
#include <inttypes.h>
#include "usbcan_proto.h"
#include "usbcan_ipc.h"
#include "usbcan_util.h"
#include "rb_tools.h"
#include "crc16-ccitt.h"
#include "logging.h"

#define TIME_DELTA_MS(x, y) ((x.tv_sec - y.tv_sec) * 1000 + (x.tv_usec - y.tv_usec) / 1000)

const char *CAN_OPEN_CMD[] = 
{
	"COM_FRAME",
	"COM_NMT",
	"COM_HB",
	"COM_TIMESTAMP",
	"COM_SDO_TX_REQ",
	"COM_SDO_TX_RESP",
	"COM_SDO_RX_REQ",
	"COM_SDO_RX_RESP",
	"COM_SYNC",
	"COM_EMCY",
	"COM_PDO",
	""
};

static void dump_can_msg(const char *label, can_msg_t *m)
{
    char msg_name[4096];

    sprintf(msg_name, "%s 0x%X", label, m->id);

    dump(msg_name, m->data, m->dlc);
}

int usbcan_build_timestamp(uint8_t *dst, uint32_t ts);
int usbcan_build_hb(uint8_t *dst, int id, usbcan_nmt_state_t state);
int usbcan_build_nmt(uint8_t *dst, int id, usbcan_nmt_cmd_t cmd);
int usbcan_build_com_frame(uint8_t *dst,  can_msg_t *m);
int usbcan_build_sdo_req(uint8_t *dst,
		usbcan_sdo_t *sdo,
		void *data, 
		uint16_t len,
		sdo_resp_cb_t cb);
int usbcan_rx(usbcan_instance_t *inst);

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
		LOG_ERROR("usbcan write failed");
	}

	return ret;
}


static void usbcan_enable_udp(usbcan_instance_t *inst, bool en)
{
	inst->usbcan_udp = en;
}

void usbcan_init(usbcan_instance_t *inst)
{
	int i;

	for(i = 0; i < USB_CAN_MAX_DEV; i++)
	{
		inst->dev_alive[i] = -1;
		inst->dev_hb_ival[i] = -1;
		inst->dev_state[i] = _CO_NMT_HB_TIMEOUT;
	}
}

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
	return _CO_NMT_HB_TIMEOUT;
}

void usbcan_inhibit_master_hb(usbcan_instance_t *inst, bool inh)
{
	inst->inhibit_master_hb = inh;
}

void usbcan_poll(usbcan_instance_t *inst, int64_t delta_ms)
{
	int i;

	inst->master_hb_timer += delta_ms;

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
				((usbcan_nmt_state_cb_t)inst->usbcan_nmt_state_cb)(inst, i, _CO_NMT_HB_TIMEOUT);
			}
		}
	}

	if(inst->master_hb_timer >= inst->master_hb_ival)
	{
		if(!inst->inhibit_master_hb)
		{
			if(inst->usbcan_hb_tx_cb)
			{
				((usbcan_hb_tx_cb_t)inst->usbcan_hb_tx_cb)(inst);
			}
			send_master_hb(inst);
		}
		inst->master_hb_timer -= inst->master_hb_ival;
	}
	if(inst->wait_for.cb)
	{
		inst->wait_for.sdo.ttl = CLIPL(inst->wait_for.sdo.ttl - delta_ms, 0);
		if(!inst->wait_for.sdo.ttl)
		{
			((sdo_resp_cb_t)inst->wait_for.cb)(inst, -1, NULL, 0);
			inst->wait_for.cb = NULL;
		}
	}
}

static void usbcan_wait_for(usbcan_instance_t *inst, usbcan_sdo_t *sdo, sdo_resp_cb_t cb)
{
	inst->wait_for.sdo = *sdo;
	inst->wait_for.cb = cb;
}

/*************************************************************************************************/

int usbcan_wrap_inplace(uint8_t *dst, int payload_sz)
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

int usbcan_build_nmt(uint8_t *dst, int id, usbcan_nmt_cmd_t cmd)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, COM_NMT);
	set_ux_(msg, &p, 1, id);
	set_ux_(msg, &p, 1, (uint8_t)cmd);

	return usbcan_wrap_inplace(dst, p);
}

int usbcan_build_hb(uint8_t *dst, int id, usbcan_nmt_state_t state)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, COM_HB);
	set_ux_(msg, &p, 1, id);
	set_ux_(msg, &p, 1, (uint8_t)state);

	return usbcan_wrap_inplace(dst, p);
}

int usbcan_build_timestamp(uint8_t *dst, uint32_t ts)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, COM_TIMESTAMP);
	set_ux_(msg, &p, sizeof(ts), ts);

	return usbcan_wrap_inplace(dst, p);
}

int usbcan_build_sdo_req(uint8_t *dst,
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

int usbcan_build_com_frame(uint8_t *dst,  can_msg_t *m)
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

/*************************************************************************************************/

int usbcan_send_nmt(usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_nmt(dst, id, cmd);
	dump("SERIAL TX(NMT)", dst, l);
	return usbcan_write_fd(inst, dst, l);
}


int usbcan_send_hb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_hb(dst, id, state);
	dump("SERIAL TX(HB)", dst, l);
	return usbcan_write_fd(inst, dst, l);
}


int usbcan_send_timestamp(usbcan_instance_t *inst, uint32_t ts)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_timestamp(dst, ts);
	return usbcan_write_fd(inst, dst, l);
}


int usbcan_send_sdo_req(usbcan_instance_t *inst, usbcan_sdo_t *sdo, void *data, uint16_t len, sdo_resp_cb_t cb)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_sdo_req(dst, sdo, data, len, cb);
	dump("SERIAL TX(SDO)", dst, l);
	usbcan_wait_for(inst, sdo, cb);
	return usbcan_write_fd(inst, dst, l);
}

int usbcan_send_com_frame(usbcan_instance_t *inst, can_msg_t *m)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_com_frame(dst, m);
//	dump("SERIAL TX(COM)", dst, l);
	dump_can_msg("CAN TX", m);
	return usbcan_write_fd(inst, dst, l);
}

void send_master_hb(usbcan_instance_t *inst)
{
	can_msg_t msg = {USB_CAN_MASTER_HB_COM_FRAME_ID, 1, {_CO_NMT_OPERATIONAL}};

	usbcan_send_com_frame(inst, &msg);
}

/*************************************************************************************************/

void usbcan_parse_com_frame(can_msg_t *m, uint8_t *msg, int sz)
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


void usbcan_frame_receive_cb(usbcan_instance_t *inst, uint8_t *data, int len)
{

	switch(data[0])
	{
		case COM_FRAME:
			{
				can_msg_t m;
				usbcan_parse_com_frame(&m, data, len);
				dump_can_msg("CAN RX", &m);
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
				dump("SDO write resp", data, len);

				uint8_t id = get_ux_(data, &p, 1);
				uint16_t idx = get_ux_(data, &p, 2);
				uint8_t sidx = get_ux_(data, &p, 1);
				uint32_t abt = get_ux_(data, &p, 4);

				if(inst->wait_for.cb)
				{
					if((inst->wait_for.sdo.id == id) &&
							(inst->wait_for.sdo.idx == idx) &&
							(inst->wait_for.sdo.sidx == sidx) &&
							(inst->wait_for.sdo.write == true))
					{
						((sdo_resp_cb_t)inst->wait_for.cb)(inst, abt, NULL, 0);
						inst->wait_for.cb = NULL;
					}
				}
			}
			break;

		case COM_SDO_RX_RESP:
			{	
				int p = 0;
				len--;
				data++;
				dump("SDO read resp", data, len);

				uint8_t id = get_ux_(data, &p, 1);
				uint16_t idx = get_ux_(data, &p, 2);
				uint8_t sidx = get_ux_(data, &p, 1);
				uint32_t abt = get_ux_(data, &p, 4);

				dump("SDO read data", data + p, len - p);

				if(inst->wait_for.cb)
				{
					if((inst->wait_for.sdo.id == id) &&
							(inst->wait_for.sdo.idx == idx) &&
							(inst->wait_for.sdo.sidx == sidx) &&
							(inst->wait_for.sdo.write == false))
					{
						((sdo_resp_cb_t)inst->wait_for.cb)(inst, abt, data + p, len - p);
						inst->wait_for.cb = NULL;
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
				dump("SERIAL RX(HB)", data, len);
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
				dump("SERIAL RX(EMCY)", data, len);
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

int usbcan_rx(usbcan_instance_t *inst)
{
	static uint8_t rb[USB_CAN_MAX_PAYLOAD];
	static int h = 0, t = 0;

	uint8_t b[USB_CAN_MAX_PAYLOAD];
	int l = read(inst->fd, b, sizeof(b));
	if(l < 0)
	{
		LOG_ERROR("usbcan read failed");
		return l;
	}
	//dump("SERIAL RX", b, l);
	
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
			LOG_WARN("malformed packed, skipping");
		}

		if(rb_dist(h, t, sizeof(rb)) >= 3)
		{
			uint16_t elen = 0;
			uint16_t ecrc = 0;

			elen = rb[(t + 1) % sizeof(rb)] << 8 | rb[(t + 2) % sizeof(rb)];

			if(elen >= sizeof(rb))
			{
				LOG_WARN("too long message %d", elen);
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
					LOG_WARN("crc error %x != %x\n", ecrc, crc);
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
	LOG_WARN("Emergency frame received: id(%"PRId8") code(0x%"PRIX16") reg(0x%"PRIX8") bits(0x%"PRIX8") info(0x%"PRIX32")",
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
	if(state == _CO_NMT_HB_TIMEOUT)
	{
		LOG_WARN("Devide %d disappeared from bus", id);
	}
	else
	{
		LOG_INFO("Device %d state changed to %d", id, state);
	}
}

void *usbcan_process(void *udata)
{
	struct timeval tprev, tnow;
	int n = 0;
	struct termios term;
	int flags;
	struct pollfd pfds[2];
	struct sockaddr_in host_addr;
	FILE *f = NULL;
	struct in_addr addr;

	usbcan_instance_t *inst = (usbcan_instance_t *)udata;

	if(inet_aton(inst->device, &addr) != 0)
	{
		usbcan_enable_udp(inst, true);
		inst->fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(inst->fd < 0)
		{
			LOG_ERROR("can't create socket\n");
			return 0;
		}
		host_addr.sin_family = AF_INET;
		host_addr.sin_port = htons(17701);
		host_addr.sin_addr.s_addr = INADDR_ANY;
		if(bind(inst->fd, (const struct sockaddr *)&host_addr, sizeof(struct sockaddr_in)) < 0)
		{
			LOG_ERROR("can't bind to port\n");
			return 0;
		}
		host_addr.sin_family = AF_INET;
		host_addr.sin_port = htons(17700);
		host_addr.sin_addr.s_addr = addr.s_addr;
		if(connect(inst->fd, (const struct sockaddr *)&host_addr, sizeof(struct sockaddr_in)) < 0)
		{
			LOG_ERROR("can't connect to port\n");
			return 0;
		}

		if(write(inst->fd, "hello", 5) != 5)
		{
			return 0;
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
		LOG_ERROR("can't open usbcan device %s", udata);
		return 0;
	}

	usbcan_setup_hb_tx_cb(inst, hb_tx_cb, 250);
	usbcan_setup_hb_rx_cb(inst, hb_rx_cb);
	usbcan_setup_emcy_cb(inst, emcy_cb);
	usbcan_setup_nmt_state_cb(inst, nmt_state_cb);

	pfds[0].fd = inst->fd;
	pfds[0].events = POLLIN;

	pfds[1].fd = inst->to_child_pipe[0];
	pfds[1].events = POLLIN;

	gettimeofday(&tnow, NULL);
	tprev = tnow;

	for(int t = USB_CAN_FLUSH_TOUT_MS; t > 0;)
	{
		int n;

		n = poll(pfds, 1, USB_CAN_POLL_GRANULARITY_MS);
		gettimeofday(&tnow, NULL);
		if(n > 0)
		{
			uint8_t discard[USB_CAN_MAX_PAYLOAD];
			if(read(inst->fd, discard, sizeof(discard)) < 0)
			{
				LOG_ERROR("read failed");
				return 0;
			}
		}
		else
		{
			break;
		}
		t -= TIME_DELTA_MS(tnow, tprev);
		tprev = tnow;
	}

	gettimeofday(&tnow, NULL);
	tprev = tnow;

	while(1)
	{
		n = poll(pfds, 2, USB_CAN_POLL_GRANULARITY_MS);
		gettimeofday(&tnow, NULL);
		usbcan_poll(inst, TIME_DELTA_MS(tnow, tprev));
		if(n > 0)
		{
			if(pfds[1].revents & POLLIN)
			{
				ipc_process(inst);
			}
			if(pfds[0].revents & POLLIN)
			{
				if(usbcan_rx(inst) < 0)
				{
					break;
				}
			}
		}
		tprev = tnow;
	}

	close(inst->fd);
	return 0;
}

int usbcan_instance_init(usbcan_instance_t *inst, const char *dev_name)
{
	memset(inst, 0, sizeof(usbcan_instance_t));
	inst->master_hb_ival = USB_CAN_MASTER_HB_IVAL_MS;
	inst->hb_alive_threshold = USB_CAN_HB_ALIVE_THRESHOLD_MS;
	usbcan_init(inst);
	ipc_create_link(inst);

	if(pthread_create(&inst->usbcan_thread, NULL, usbcan_process, inst))
	{
		LOG_ERROR("Can't run thread");
		return 0;
	}

	return 1;
}
