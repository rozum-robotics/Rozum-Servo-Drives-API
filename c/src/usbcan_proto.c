#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include "usbcan_proto.h"
#include "usbcan_util.h"
#include "rb_tools.h"
#include "crc16-ccitt.h"
#include "logging.h"


FILE *debug_log = NULL;

/*
 * PRIVATE functions
 */

static int usbcan_build_timestamp(uint8_t *dst, uint32_t ts);
static int usbcan_build_hb(uint8_t *dst, int id, usbcan_nmt_state_t state);
static int usbcan_build_nmt(uint8_t *dst, int id, usbcan_nmt_cmd_t cmd);
static int usbcan_build_com_frame(uint8_t *dst,  can_msg_t *m);
static int usbcan_build_sdo_req(uint8_t *dst, bool write, uint8_t id, 
		uint16_t idx, uint8_t sidx, uint32_t tout, uint8_t re_txn,
		void *data, uint16_t len);

static int usbcan_rx(usbcan_instance_t *inst);

void usbcan_send_traj_sync(usbcan_instance_t *inst);


/*
 * Check interface instance for consistency.
 */
static bool is_valid_instance(const usbcan_instance_t *inst)
{
	if(!inst)
	{
		LOG_ERROR(debug_log, "%s: null interface", __func__);
		return false;
	}
	if(!inst->running)
	{
		LOG_ERROR(debug_log, "%s: interface thread not running", __func__);
		return false;
	}
	return true;
}

/*
 * Check device instance for consistency.
 */
static bool is_valid_device(const usbcan_device_t *dev)
{
	if(!dev)
	{
		LOG_ERROR(debug_log, "%s: null device", __func__);
		return false;
	}
	if(!dev->inst)
	{
		LOG_ERROR(debug_log, "%s: device pointing to null interface", __func__);
		return false;
	}
	if(!dev->inst->running)
	{
		LOG_ERROR(debug_log, "%s: device's corresponding interface thread not running", __func__);
		return false;
	}
	return true;
}

/*
 * Writes to interface file descriptor. 
 * Notice: data will be unwrapped if connection type is UDP socket.
 */
static int usbcan_write_fd(usbcan_instance_t *inst, uint8_t *b, int l)
{
	int ret = 0;
	
	pthread_mutex_lock(&inst->mutex_write);

	if(!inst->usbcan_udp)
	{
#ifndef _WIN32
		ret = write(inst->fd, b, l);
#else
		DWORD written;
		if(!WriteFile(inst->commh, b, l, &written, &inst->overlap_write))
		{	
			switch(WaitForSingleObject(inst->overlap_write.hEvent, INFINITE))
			{
				case WAIT_OBJECT_0:
					GetOverlappedResult(inst->commh, &inst->overlap_write, &written, FALSE);
					ResetEvent(&inst->overlap_write.hEvent);
					break;
					
				case WAIT_TIMEOUT:
					written = 0;
					break;
					
				default:
					written = 0;
					break;		
			}
		}		
		ret = written;
		if(ret != l)
		{
			LOG_ERROR(debug_log, "%s: %d bytes of %d written", __func__, ret, l);
		}
#endif
	}
	else
	{
		ret = send(inst->fd, (char*)(b + USB_CAN_HEAD_SZ), l - USB_CAN_OHEAD, 0);
		if(ret < 0)
		{
			LOG_ERROR(debug_log, "%s: usbcan write failed", __func__);
		}
	}


	pthread_mutex_unlock(&inst->mutex_write);
	
	return ret;
}

/*
 * Callback for handling SDO response.
 */
static void sdo_resp_cb(usbcan_instance_t *inst, uint32_t abt, uint8_t *data, int len)
{
	inst->op.abt = abt;
	if(!inst->op.write)
	{
		memcpy(inst->op.data, data, len);
	}
	inst->op.len = len;
	pthread_mutex_lock(&inst->mutex);
	pthread_cond_signal(&inst->cond);
	pthread_mutex_unlock(&inst->mutex);
}

/*
 * Enables or disables USB<->CAN frames wrapping/unwrapping
 */
static void usbcan_enable_udp(usbcan_instance_t *inst, bool en)
{
	inst->usbcan_udp = en;
}

/*
 * Handles devices statuses, SDO reception & master heart beat transmission.
 */
static void usbcan_poll(usbcan_instance_t *inst, uint64_t delta_us)
{
	int i;
	uint32_t delta_ms = delta_us / 1000 + (delta_us % 1000 ? 1 : 0);

	inst->master_hb_timer += delta_us;
	inst->traj_sync_timer += delta_us;
	
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
			inst->dev_state[i] = CO_NMT_HB_TIMEOUT;
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

	/*Send sync message*/
	if(inst->traj_sync_timer >= inst->traj_sync_ival)
	{
		if(inst->send_traj_sync_enable)
		{
			usbcan_send_traj_sync(inst);
		}
		inst->traj_sync_timer -= inst->traj_sync_ival;
	}

	/*Wait for SDO response*/
	if(inst->op.code == OP_SDO)
	{
		inst->op.ttl = CLIPL(inst->op.ttl - delta_ms, 0);
		if(!inst->op.ttl)
		{
			inst->op.code = OP_NONE;
			sdo_resp_cb(inst, -1u, NULL, 0);
		}
	}

	/*Wait for device specific state*/
	if(inst->op.code == OP_WAIT_DEV_STATE)
	{
		if(inst->dev_alive[inst->op.id] > 0)
		{			
			if((inst->op.state == CO_NMT_ANY) || (inst->op.state == inst->dev_state[inst->op.id]))
			{
				inst->op.abt = 0;
				inst->op.code = OP_NONE;
				pthread_mutex_lock(&inst->mutex);
				pthread_cond_signal(&inst->cond);
				pthread_mutex_unlock(&inst->mutex);
			}
		}
		if(inst->op.ttl <= 0)
		{	
			inst->op.abt = -1u;
			inst->op.code = OP_NONE;
			pthread_mutex_lock(&inst->mutex);
			pthread_cond_signal(&inst->cond);
			pthread_mutex_unlock(&inst->mutex);
		}

		inst->op.ttl -= delta_ms;
	}

	/*Wait for device boot-up*/
	if(inst->op.code == OP_WAIT_DEV_BOOT_UP)
	{
		if(inst->dev_boot_up[inst->op.id])
		{			
			inst->dev_boot_up[inst->op.id] = false;
			inst->op.abt = 0;
			inst->op.code = OP_NONE;
			pthread_mutex_lock(&inst->mutex);
			pthread_cond_signal(&inst->cond);
			pthread_mutex_unlock(&inst->mutex);
		}
		if(inst->op.ttl <= 0)
		{	
			inst->op.abt = -1u;
			inst->op.code = OP_NONE;
			pthread_mutex_lock(&inst->mutex);
			pthread_cond_signal(&inst->cond);
			pthread_mutex_unlock(&inst->mutex);

		}

		inst->op.ttl -= delta_ms;
	}
}

/*
 * Wraps USB<->CAN frame with start byte (0x02), 2-byte length code & 2-byte CRC.
 * Notice: data to wrap sould be placed starting at dst[3] & dst size > (payload_sz + 5).
 */
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

/*
 * Builds request for sending NMT frame in USB<->CAN format.
 */
static int usbcan_build_nmt(uint8_t *dst, int id, usbcan_nmt_cmd_t cmd)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, COM_NMT);
	set_ux_(msg, &p, 1, id);
	set_ux_(msg, &p, 1, (uint8_t)cmd);

	return usbcan_wrap_inplace(dst, p);
}

/*
 * Builds request for sending heart beat frame in USB<->CAN format.
 */
static int usbcan_build_hb(uint8_t *dst, int id, usbcan_nmt_state_t state)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, COM_HB);
	set_ux_(msg, &p, 1, id);
	set_ux_(msg, &p, 1, (uint8_t)state);

	return usbcan_wrap_inplace(dst, p);
}

/*
 * Builds request for sending timestamp (SYNC) frame in USB<->CAN format.
 */
static int usbcan_build_timestamp(uint8_t *dst, uint32_t ts)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, COM_TIMESTAMP);
	set_ux_(msg, &p, sizeof(ts), ts);

	return usbcan_wrap_inplace(dst, p);
}

/*
 * Builds request for sending SDO request frame in USB<->CAN format.
 */
static int usbcan_build_sdo_req(uint8_t *dst, bool write, uint8_t id, 
		uint16_t idx, uint8_t sidx, uint32_t tout, uint8_t re_txn,
		void *data,	uint16_t len)
{
	int p = 0;
	uint8_t *msg = dst + USB_CAN_HEAD_SZ;

	set_ux_(msg, &p, 1, write ? COM_SDO_TX_REQ : COM_SDO_RX_REQ);
	set_ux_(msg, &p, 1, id);
	set_ux_(msg, &p, 2, idx);
	set_ux_(msg, &p, 1, sidx);
	set_ux_(msg, &p, 2, (tout & 0x1FFFU) |(re_txn & 0x7U) << 13);
	if(write)
	{
		memcpy(msg + p, data, len);
		p += len;
	}

	return usbcan_wrap_inplace(dst, p);
}

/*
 * Builds request for sending generic CAN frame in USB<->CAN format.
 */
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

/*
 * Sends NMT frame.
 */
int usbcan_send_nmt(usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_nmt(dst, id, cmd);
	LOG_DUMP(inst->comm_log, "SERIAL TX(NMT)", dst, l);
	return usbcan_write_fd(inst, dst, l);
}

/*
 * Sends heart beat.
 */
int usbcan_send_hb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_hb(dst, id, state);
	LOG_DUMP(inst->comm_log, "SERIAL TX(HB)", dst, l);
	return usbcan_write_fd(inst, dst, l);
}

/*
 * Sends timestamp (SYNC) frame.
 */
int usbcan_send_timestamp(usbcan_instance_t *inst, uint32_t ts)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_timestamp(dst, ts);
	return usbcan_write_fd(inst, dst, l);
}

/*
 * Sends SDO request.
 */
int usbcan_send_sdo_req(usbcan_instance_t *inst, bool write, uint8_t id, 
		uint16_t idx, uint8_t sidx, uint32_t tout, uint8_t re_txn,
		void *data, uint16_t len)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_sdo_req(dst, write, id, idx, sidx, tout, re_txn, data, len);
	LOG_DUMP(inst->comm_log, "SERIAL TX(SDO)", dst, l);
	return usbcan_write_fd(inst, dst, l);
}

/*
 * Sends non-CanOpen genaric CAN frame.
 */
int usbcan_send_com_frame(usbcan_instance_t *inst, can_msg_t *m)
{
	uint8_t dst[USB_CAN_MAX_PAYLOAD];
	int l = usbcan_build_com_frame(dst, m);
	return usbcan_write_fd(inst, dst, l);
}

/**
 * @brief Sends PDO message. 
 * 
 * @param inst Usbcan instance
 * @param cob_id CAN frame COB-ID
 * @param data pointer to data
 * @parma len length of the data
 * @return int status code
 */
int usbcan_send_pdo(usbcan_instance_t *inst, uint16_t cob_id, void *data, uint16_t len)
{
	if(len >= 8) return -1;

	can_msg_t msg;
	msg.id = cob_id;
	msg.dlc = len;
	memcpy(msg.data, data, msg.dlc);

	return usbcan_send_com_frame(inst, &msg);
}

/*
 * Sends master heart beat. 
 * Notice: emulataed using generic CAN frame.
 */
void usbcan_send_master_hb(usbcan_instance_t *inst)
{
	can_msg_t msg = {USB_CAN_MASTER_HB_COM_FRAME_ID, 1, {CO_NMT_OPERATIONAL}};

	usbcan_send_com_frame(inst, &msg);
}

/*
 * Sends sync message for trajectory execution synchronization. 
 */
void usbcan_send_traj_sync(usbcan_instance_t *inst)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	if(!inst->traj_sync_prev.tv_sec && !inst->traj_sync_prev.tv_usec)
	{
		inst->traj_sync_prev = now;
		return;
	}
	uint32_t s = TIME_DELTA_US(now, inst->traj_sync_prev) % 600000000ll;
	can_msg_t msg = {USB_CAN_TRAJ_SYNC_COM_FRAME_ID, sizeof(s)};
	memcpy(msg.data, &s, sizeof(s));

	usbcan_send_com_frame(inst, &msg);
}

/*
 * Parses non-CanOpen genaric CAN frame.
 */
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

/*
 * Handles USB<->CAN packets.
 */
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

				if(inst->op.code == OP_SDO)
				{
					inst->op.code = OP_NONE;
					if((inst->op.id == id) &&
							(inst->op.idx == idx) &&
							(inst->op.sidx == sidx) &&
							(inst->op.write == true))
					{
						sdo_resp_cb(inst, abt, NULL, 0);
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
			
				if(inst->op.code == OP_SDO)
				{
					inst->op.code = OP_NONE;
					if((inst->op.id == id) &&
							(inst->op.idx == idx) &&
							(inst->op.sidx == sidx) &&
							(inst->op.write == false))
					{
						sdo_resp_cb(inst, abt, data + p, len - p);
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
				usbcan_nmt_state_t state = (usbcan_nmt_state_t)get_ux_(data, &p, 1);

				if(state == CO_NMT_INITIALIZING)
				{
					inst->dev_boot_up[id] = true;
				}

				if((inst->dev_state[id] != state) || (inst->dev_alive[id] == -1))
				{
					inst->dev_state[id] = state;
					if(inst->usbcan_nmt_state_cb)
					{
						((usbcan_nmt_state_cb_t)inst->usbcan_nmt_state_cb)(inst, id, state);
					}
				}

				inst->dev_hb_ival[id] = inst->hb_alive_threshold - inst->dev_alive[id];
				if(inst->dev_min_hb_ival[id] < 0)
				{
					inst->dev_min_hb_ival[id] = inst->dev_hb_ival[id];
				}
				inst->dev_min_hb_ival[id] = MIN(inst->dev_min_hb_ival[id], inst->dev_hb_ival[id]);
				inst->dev_max_hb_ival[id] = MAX(inst->dev_max_hb_ival[id], inst->dev_hb_ival[id]);
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

/*
 * Deserializes incoming serial port data into packets.
 * Incomplete incoming data are stored in ring buffer.
 * Do noting for UDP socket connections.
 */
static int usbcan_rx(usbcan_instance_t *inst)
{
	if(inst->usbcan_udp)
	{
		usbcan_frame_receive_cb(inst, inst->rx_data.b, inst->rx_data.l);
		return inst->rx_data.l;
	}

	inst->rx_data.h = rb_to_rb(inst->rx_data.rb, inst->rx_data.h, USB_CAN_MAX_PAYLOAD, 
			inst->rx_data.b, 0, inst->rx_data.l, inst->rx_data.l);

	while(1)
	{
		while(inst->rx_data.t != inst->rx_data.h)
		{
			if(inst->rx_data.rb[inst->rx_data.t] == USB_CAN_STX)
			{
				break;
			}
			inst->rx_data.t = (inst->rx_data.t + 1) % USB_CAN_MAX_PAYLOAD;
			LOG_WARN(debug_log, "%s: malformed packed, skipping", __func__);
		}

		if(rb_dist(inst->rx_data.h, inst->rx_data.t, USB_CAN_MAX_PAYLOAD) >= 3)
		{
			uint16_t elen = 0;
			uint16_t ecrc = 0;

			elen = inst->rx_data.rb[(inst->rx_data.t + 1) % USB_CAN_MAX_PAYLOAD] << 8 | 
					inst->rx_data.rb[(inst->rx_data.t + 2) % USB_CAN_MAX_PAYLOAD];

			if(elen >= USB_CAN_MAX_PAYLOAD)
			{
				LOG_WARN(debug_log, "%s: too long message %d", __func__, elen);
				inst->rx_data.t = (inst->rx_data.t + 1) % USB_CAN_MAX_PAYLOAD;
			}

			if(rb_dist(inst->rx_data.h, inst->rx_data.t, USB_CAN_MAX_PAYLOAD) >= (3 + elen + 2))
			{
				uint16_t crc = 0;
				uint8_t pload[USB_CAN_MAX_PAYLOAD];

				ecrc = inst->rx_data.rb[(inst->rx_data.t + 3 + elen) % USB_CAN_MAX_PAYLOAD] << 8 | 
						inst->rx_data.rb[(inst->rx_data.t + 4 + elen) % USB_CAN_MAX_PAYLOAD];
				rb_to_rb(pload, 0, sizeof(pload), inst->rx_data.rb, inst->rx_data.t + 3, USB_CAN_MAX_PAYLOAD, elen);
				crc = crc16_ccitt(pload, elen, crc);
				if(crc == ecrc)
				{
					usbcan_frame_receive_cb(inst, pload, elen);
					inst->rx_data.t = (inst->rx_data.t + 5 + elen) % USB_CAN_MAX_PAYLOAD;
				}
				else
				{
					LOG_WARN(debug_log, "%s: crc error %x != %x\n", __func__, ecrc, crc);
					inst->rx_data.t = (inst->rx_data.t + 1) % USB_CAN_MAX_PAYLOAD;
				}
				continue;
			}
		}
		break;
	}
	return inst->rx_data.l;
}

/*
 * Default callback for handling emergency packets
 */
static void emcy_cb(usbcan_instance_t *inst, int id, uint16_t code, uint8_t reg, uint8_t bits, uint32_t info)
{
	LOG_WARN(debug_log, "Emergency frame received: id(%" PRId8 ") code(0x%" PRIX16 ") reg(0x%" PRIX8 ") bits(0x%" PRIX8 ") info(0x%" PRIX32 ")",
		id, code, reg, bits, info);
}

/*
 * Default callback running before master heart beat transmition
 */
static void hb_tx_cb(usbcan_instance_t *inst)
{
}

/*
 * Default callback for handling heart beat reception
 */
static void hb_rx_cb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
}

/*
 * Default callback for handling NMT state changes
 */
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

/*
 * Flushes unread data from interface
 */
static void usbcan_flush_device(usbcan_instance_t *inst)
{
#ifdef _WIN32
	if(inst->commh <= 0)
	{
		return;
	}

	PurgeComm(inst->commh, PURGE_RXCLEAR | 
						PURGE_TXCLEAR |
						PURGE_TXABORT |
						PURGE_RXABORT);	
#else
	if(inst->fd <= 0)
	{
		return;
	}

	uint8_t discard[USB_CAN_MAX_PAYLOAD];

	struct timeval tprev, tnow;
	struct pollfd pfds[1];

	pfds[0].fd = (int)inst->fd;
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
				inst->fd = (typeof(inst->fd)) -1;
				return;
			}
			if(pfds[0].revents & POLLIN)
			{
				if(read((int)inst->fd, discard, sizeof(discard)) < 0)
				{
					LOG_ERROR(debug_log, "%s: read failed", __func__);
					inst->fd = (typeof(inst->fd)) -1;
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
 #endif
}

#ifdef WIN32
static int inet_aton(const char *cp, struct in_addr *inp)
{
	if(cp == 0 || inp == 0)
	{
		return -1;
	}

	unsigned long addr = inet_addr(cp);
	if(addr == INADDR_NONE || addr == INADDR_ANY)
	{
		return -1;
	}

	inp->s_addr = addr;
	return 1;
}
#endif

/*
 * Detects type of interface (serial or socket) and opens it
 */
static void usbcan_open_device(usbcan_instance_t *inst)
{	
	struct sockaddr_in host_addr;
	struct in_addr addr;
	int in_port = USB_CAN_INGOING_UDP_PORT;

	char dev_str[128];

	char *dev_addr, *dev_port;

	if(strnlen(inst->device, sizeof(dev_str)) == sizeof(dev_str))
	{
			LOG_ERROR(debug_log, "%s: device string to long", __func__);
			return;
	}

	strcpy(dev_str, inst->device);

	dev_addr = strtok(dev_str, ":");
	dev_port = strtok(NULL, "");

	if(dev_port)
	{
		char *endptr;
		in_port = strtol(dev_port, &endptr, 0);
		if(*endptr)
		{
			LOG_ERROR(debug_log, "%s: wrong UDP port", __func__);
			return;
		}
	}

#ifdef _WIN32
	if(inet_aton(dev_addr, &addr) != -1)
	{
		usbcan_enable_udp(inst, true);

		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if(iResult != 0) 
		{
			LOG_ERROR(debug_log, "WSAStartup failed: %d\n", iResult);
			inst->fd = -1;
			return;
		}

		inst->fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(inst->fd < 0)
		{
			LOG_ERROR(debug_log, "%s: can't create UDP socket", __func__);
			return;
		}
		
		host_addr.sin_family = AF_INET;
		host_addr.sin_port = 0;//htons(USB_CAN_OUTGOING_UDP_PORT);
		host_addr.sin_addr.s_addr = INADDR_ANY;
		if(bind(inst->fd, (const struct sockaddr *)&host_addr, sizeof(struct sockaddr_in)) < 0)
		{
			LOG_ERROR(debug_log, "%s: can't bind to UDP port", __func__);
			inst->fd = -1;
			return;
		}
		host_addr.sin_family = AF_INET;
		host_addr.sin_port = htons(in_port);
		host_addr.sin_addr.s_addr = addr.s_addr;
		if(connect(inst->fd, (const struct sockaddr *)&host_addr, sizeof(struct sockaddr_in)) < 0)
		{
			LOG_ERROR(debug_log, "%s: can't connect to UDP port %s", __func__);
			inst->fd = -1;
			return;
		}

		if(send(inst->fd, "hello", 5, 0) != 5)
		{
			LOG_ERROR(debug_log, "%s: can't write to UDP port", __func__);
			inst->fd = -1;
			return;
		}
		LOG_INFO(debug_log, "Connected to UDP socket: %s port %d", dev_addr, in_port);
	}
	else
	{
		inst->commh = CreateFile(inst->device,
						GENERIC_READ | GENERIC_WRITE,
						0,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_OVERLAPPED,
						NULL);
							
		if(inst->commh == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR(debug_log, "%s: can't open serial device %s", __func__, inst->device);
			inst->fd = 0;
			return;
		}
		
		
		DCB dcbSerialParams = { 0 }; 
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		GetCommState(inst->commh, &dcbSerialParams);

		dcbSerialParams.BaudRate = CBR_9600;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity   = NOPARITY;
		
		SetCommState(inst->commh, &dcbSerialParams);
		
		COMMTIMEOUTS timeouts = { 0 };
		timeouts.ReadIntervalTimeout         = 0;
		timeouts.ReadTotalTimeoutConstant    = 0;
		timeouts.ReadTotalTimeoutMultiplier  = 0;
		timeouts.WriteTotalTimeoutConstant   = 0;
		timeouts.WriteTotalTimeoutMultiplier = 0;
		
		SetCommTimeouts(inst->commh, &timeouts);
		SetCommMask(inst->commh, EV_RXCHAR);
		
		memset(&inst->overlap_read, 0, sizeof(OVERLAPPED));
		memset(&inst->overlap_write, 0, sizeof(OVERLAPPED));
		memset(&inst->overlap_evt, 0, sizeof(OVERLAPPED));
		inst->overlap_read.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		inst->overlap_write.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		inst->overlap_evt.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		
		usbcan_enable_udp(inst, false);
	}

#else

	FILE *f = NULL;
	int flags;

	if(inet_aton(dev_addr, &addr) != 0)
	{
		usbcan_enable_udp(inst, true);
		inst->fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(inst->fd < 0)
		{
			LOG_ERROR(debug_log, "%s: can't create UDP socket", __func__);
			return;
		}
		host_addr.sin_family = AF_INET;
		host_addr.sin_port = 0;//htons(USB_CAN_OUTGOING_UDP_PORT);
		host_addr.sin_addr.s_addr = INADDR_ANY;
		if(bind(inst->fd, (const struct sockaddr *)&host_addr, sizeof(struct sockaddr_in)) < 0)
		{
			LOG_ERROR(debug_log, "%s: can't bind to UDP port", __func__);
			inst->fd = -1;
			return;
		}
		host_addr.sin_family = AF_INET;
		host_addr.sin_port = htons(in_port);
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
	else if((f = fopen(dev_addr, "r+")) != NULL)
	{
		inst->fd = fileno(f);
		if(flock(inst->fd, LOCK_EX | LOCK_NB) != 0)
		{
			close(inst->fd);
			inst->fd = -1;
			LOG_ERROR(debug_log, "%s: can't get exclusive lock", __func__);
			return;
		}

		struct termios term;
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
#endif

	usbcan_flush_device(inst);
}


#ifdef _WIN32

int win_comm_recv(usbcan_instance_t *inst)
{
	DWORD bytes_to_read = 0;
	DWORD err;
	COMSTAT stat;

	if(!inst->evt_waiting) 
	{
		inst->evt_mask = EV_RXCHAR;
		if(!WaitCommEvent(inst->commh, &inst->evt_mask, &inst->overlap_evt)) 
		{
			if(GetLastError() != ERROR_IO_PENDING) 
			{
				LOG_INFO(debug_log, "%s: WaitCommEvent failed", __func__);
				return 0;
			}
			else
			{
				inst->evt_waiting = TRUE;
			}
		}
		else
		{
			ClearCommError(inst->commh, &err, &stat);
			bytes_to_read = stat.cbInQue;
			inst->evt_waiting = FALSE;
		}
	}

	if(inst->evt_waiting) 
	{
		switch(WaitForSingleObject(inst->overlap_evt.hEvent, USB_CAN_POLL_GRANULARITY_MS))
		{
			case WAIT_OBJECT_0:
				{				
					GetOverlappedResult(inst->commh, &inst->overlap_evt, &inst->evt_mask_len, false);
					ResetEvent(inst->overlap_evt.hEvent);
					ClearCommError(inst->commh, &err, &stat);
					bytes_to_read = stat.cbInQue;
					inst->evt_waiting = FALSE;
				}
				break;

			case WAIT_TIMEOUT:
				return 0;

			default:
				LOG_INFO(debug_log, "%s: GetOverlappedResult failed", __func__);
				inst->evt_waiting = FALSE;
				return 0;		
		}
	}

	if(bytes_to_read)
	{
		if(!ReadFile(inst->commh, inst->rx_data.b, bytes_to_read, &inst->rx_data.l, &inst->overlap_read))
		{
			LOG_ERROR(debug_log, "%s: ReadFile failed to read buffered data", __func__);
		}
	}

	return 0;
}

#endif



/*
 * Thread task.
 * Handles recieved data from USB<->CAN ot Ethernet<->CAN.
 */
static void *usbcan_process(void *udata)
{
	usbcan_instance_t *inst = (usbcan_instance_t *)udata;
	
#ifdef _WIN32

	if(!inst->usbcan_udp)
	{
		struct timeval tprev, tnow;

		gettimeofday(&tnow, NULL);

		while(1)
		{
			win_comm_recv(inst);
			usbcan_rx(inst);

			tprev = tnow;
			gettimeofday(&tnow, NULL);
			//fprintf(stderr, "%ld\n", (long int)TIME_DELTA_MS(tnow, tprev));
			usbcan_poll(inst, TIME_DELTA_US(tnow, tprev));
		}
	}
	else
#endif
	{
		struct timeval tprev, tnow;
		int n = 0;
		struct pollfd pfds[1];

		do
		{
			pfds[0].fd = inst->fd;
			pfds[0].events = POLLIN | POLLERR;

			gettimeofday(&tnow, NULL);
			tprev = tnow;

			while(inst->running)
			{
				n = poll(pfds, 1, USB_CAN_POLL_GRANULARITY_MS);
				gettimeofday(&tnow, NULL);
				usbcan_poll(inst, TIME_DELTA_US(tnow, tprev));
				if(n > 0)
				{
					if(pfds[0].revents & POLLERR)
					{
						LOG_ERROR(debug_log, "%s: poll failed", __func__);
						break;
					}
					if(pfds[0].revents & POLLIN)
					{
						inst->rx_data.l = recv(inst->fd, (char*)inst->rx_data.b, USB_CAN_MAX_PAYLOAD, 0);
						if(inst->rx_data.l <= 0)
						{
							LOG_ERROR(debug_log, "%s: usbcan UDP read failed", __func__);
							break;
						}

						if(usbcan_rx(inst) < 0)
						{
							LOG_ERROR(debug_log, "%s: read failed", __func__);
							break;
						}
					}
				}
				tprev = tnow;
			}
#ifndef _WIN32
			if(!inst->usbcan_udp)
			{
				flock(inst->fd, LOCK_UN);
			}
#endif
			close(inst->fd);
			inst->fd = -1;
		}
		while(0);

		inst->running = false;
	}

	return 0;
}

/*
 * PUBLIC functions
 */

void usbcan_setup_hb_tx_cb(usbcan_instance_t *inst, usbcan_hb_tx_cb_t cb, int64_t to_us)
{
	inst->master_hb_ival = to_us;
	inst->usbcan_hb_tx_cb = (void*)cb;
}

void usbcan_setup_hb_rx_cb(usbcan_instance_t *inst, usbcan_hb_rx_cb_t cb)
{
	inst->usbcan_hb_rx_cb = (void*)cb;
}

void usbcan_setup_emcy_cb(usbcan_instance_t *inst, usbcan_emcy_cb_t cb)
{
	inst->usbcan_emcy_cb = (void*)cb;
}

void usbcan_setup_nmt_state_cb(usbcan_instance_t *inst, usbcan_nmt_state_cb_t cb)
{
	inst->usbcan_nmt_state_cb = (void*)cb;
}

void usbcan_setup_com_frame_cb(usbcan_instance_t *inst, usbcan_com_frame_cb_t cb)
{
	inst->usbcan_com_frame_cb = (void*)cb;
}

int64_t usbcan_get_hb_interval(usbcan_instance_t *inst, int id)
{
	if(INRANGE(id, 0, USB_CAN_MAX_DEV - 1))
	{
		return inst->dev_hb_ival[id];
	}
	return -1;
}

int64_t usbcan_get_min_hb_interval(usbcan_instance_t *inst, int id)
{
	if(INRANGE(id, 0, USB_CAN_MAX_DEV - 1))
	{
		return inst->dev_min_hb_ival[id];
	}
	return -1;
}

int64_t usbcan_get_max_hb_interval(usbcan_instance_t *inst, int id)
{
	if(INRANGE(id, 0, USB_CAN_MAX_DEV - 1))
	{
		return inst->dev_max_hb_ival[id];
	}
	return -1;
}

bool usbcan_clear_hb_stat(usbcan_instance_t *inst, int id)
{
	if(INRANGE(id, 0, USB_CAN_MAX_DEV - 1))
	{
		inst->dev_max_hb_ival[id] = -1;
		inst->dev_min_hb_ival[id] = -1;
		return true;
	}
	return false;
}

usbcan_nmt_state_t usbcan_get_device_state(usbcan_instance_t *inst, int id)
{
	if(INRANGE(id, 0, USB_CAN_MAX_DEV - 1))
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

	usbcan_instance_t *inst = (usbcan_instance_t *)malloc(sizeof(usbcan_instance_t));
	if(!inst)
	{
		LOG_WARN(debug_log, "%s: can't allocate interface instance", __func__);
		return NULL;
	}
	memset(inst, 0, sizeof(usbcan_instance_t));
	inst->master_hb_ival = USB_CAN_MASTER_HB_IVAL_MS * 1000;
	inst->master_hb_timer = inst->master_hb_ival;
	inst->hb_alive_threshold = USB_CAN_HB_ALIVE_THRESHOLD_MS;
	inst->device = dev_name;

	inst->traj_sync_ival = USB_CAN_TRAJ_SYNC_IVAL_MS * 1000;
	inst->send_traj_sync_enable = true;

	for(i = 0; i < USB_CAN_MAX_DEV; i++)
	{
		inst->dev_alive[i] = -1;
		inst->dev_boot_up[i] = false;
		inst->dev_hb_ival[i] = -1;
		usbcan_clear_hb_stat(inst, i);
		inst->dev_state[i] = CO_NMT_HB_TIMEOUT;
	}
	
	inst->rx_data.t = 0;
	inst->rx_data.h = 0;
	inst->rx_data.b = (uint8_t *)malloc(USB_CAN_MAX_PAYLOAD);
	inst->rx_data.rb = (uint8_t *)malloc(USB_CAN_MAX_PAYLOAD);

	usbcan_open_device(inst);
#ifndef _WIN32
	if(inst->fd <= 0)
	{
		return NULL;
	}
#else
	if(inst->usbcan_udp)
	{
		if(inst->fd <= 0)
		{
			return NULL;
		}
	}
	else
	{
		if(inst->commh <= 0)
		{
			return NULL;
		}
	}
#endif
	
	usbcan_setup_hb_tx_cb(inst, hb_tx_cb, USB_CAN_MASTER_HB_IVAL_MS * 1000);
	usbcan_setup_hb_rx_cb(inst, hb_rx_cb);
	usbcan_setup_emcy_cb(inst, emcy_cb);
	usbcan_setup_nmt_state_cb(inst, nmt_state_cb);
	
	pthread_mutex_init(&inst->mutex, NULL);
	pthread_mutex_init(&inst->mutex_write, NULL);
	pthread_cond_init(&inst->cond, NULL);

	inst->running = true;

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

		(*inst)->running = false;

		msleep(10 * USB_CAN_POLL_GRANULARITY_MS);

#ifndef _WIN32
		if((*inst)->fd != -1)
		{
			LOG_WARN(debug_log, "%s: can't stop thread normally, cancelling it", __func__);
			pthread_cancel((*inst)->usbcan_thread);
			if(!(*inst)->usbcan_udp)
			{
				flock((*inst)->fd, LOCK_UN);
			}
			close((*inst)->fd);
		}
#else
		pthread_cancel((*inst)->usbcan_thread);
		if((*inst)->usbcan_udp)
		{
			shutdown((*inst)->fd, 2);
			closesocket((*inst)->fd);
			WSACleanup();
		}
		else
		{
			CloseHandle((*inst)->commh);
		
		}
#endif
		free((*inst)->rx_data.b);
		free((*inst)->rx_data.rb);
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
	usbcan_device_t *dev = (usbcan_device_t *)malloc(sizeof(usbcan_device_t));
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
			inst->device_list = (*dev)->next;			
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

int wait_device(usbcan_instance_t *inst, int id, int timeout_ms)
{
	return wait_device_state(inst, id, CO_NMT_ANY, timeout_ms);
}

int wait_device_state(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state, int timeout_ms)
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

	pthread_mutex_lock(&inst->mutex);
	
	inst->op.code = OP_WAIT_DEV_STATE;
	inst->op.ttl = timeout_ms;
	inst->op.state = state;
	inst->op.id = id;
	inst->op.abt = -1;
	inst->dev_alive[inst->op.id] = -1;

	pthread_cond_wait(&inst->cond, &inst->mutex);
	pthread_mutex_unlock(&inst->mutex);

	if(inst->op.abt)
	{
		LOG_WARN(debug_log, "%s: device (%d) not entered into desired (%d), mode during timeout (%d) period", __func__, id, state, timeout_ms);
	}

	return !inst->op.abt;
}

void clear_device_boot_up_flag(usbcan_instance_t *inst, int id)
{
	inst->dev_boot_up[id] = false;
}

int wait_device_boot_up(usbcan_instance_t *inst, int id, int timeout_ms)
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

	pthread_mutex_lock(&inst->mutex);
	
	inst->op.code = OP_WAIT_DEV_BOOT_UP;
	inst->op.ttl = timeout_ms;
	inst->op.id = id;
	inst->op.abt = -1;

	pthread_cond_wait(&inst->cond, &inst->mutex);
	pthread_mutex_unlock(&inst->mutex);

	if(inst->op.abt)
	{
		LOG_WARN(debug_log, "%s: device (%d) sent no boot-up messages during timeout (%d) period", __func__, id, timeout_ms);
	}

	return !inst->op.abt;
}

int write_nmt(usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd)
{
	if(!is_valid_instance(inst))
	{
		return 0;
	}

	usbcan_send_nmt(inst, id, cmd);

	return 1;
}

int write_timestamp(usbcan_instance_t *inst, uint32_t ts)
{
	if(!is_valid_instance(inst))
	{
		return 0;
	}

	usbcan_send_timestamp(inst, ts);

	return 1;
}

int write_com_frame(usbcan_instance_t *inst, can_msg_t *msg)
{
	if(!is_valid_instance(inst))
	{
		return 0;
	}

	usbcan_send_com_frame(inst, msg);

	return 1;
}

uint32_t write_raw_sdo(usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int len, int retry, int timeout_ms)
{
	if(!is_valid_device(dev))
	{
		return -1;
	}

	usbcan_instance_t *inst = dev->inst;

	pthread_mutex_lock(&inst->mutex);

	inst->op.code = OP_SDO;
	inst->op.write = true;
	inst->op.id = dev->id;
	inst->op.idx = idx;
	inst->op.sidx = sidx;
	inst->op.tout = timeout_ms ? timeout_ms : dev->timeout;
	inst->op.re_txn = retry ? retry : dev->retry;
	inst->op.ttl = inst->op.tout * 2;
	inst->op.len = len;

	usbcan_send_sdo_req(inst, true, dev->id, idx, sidx, timeout_ms, retry, data, len);

	pthread_cond_wait(&inst->cond, &inst->mutex);
	pthread_mutex_unlock(&inst->mutex);

	if(inst->op.abt)
	{
		LOG_ERROR(debug_log, "%s: SDO failed id(%d) idx(0x%X) sidx(%d), len(%d), re_txn(%d), tout(%d) with abort-code(0x%.X):\n    %s", 
					__func__,
					inst->op.id,
					(unsigned int)idx, 
					(int)sidx, 
					len, 
					inst->op.re_txn, 
					inst->op.tout, 
					(unsigned int)inst->op.abt, 
					sdo_describe_error(inst->op.abt));
	}

	return inst->op.abt;
}

uint32_t read_raw_sdo(usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int *len, int retry, int timeout_ms)
{
	if(!is_valid_device(dev))
	{
		return -1;
	}

	usbcan_instance_t *inst = dev->inst;

	pthread_mutex_lock(&inst->mutex);

	inst->op.code = OP_SDO;
	inst->op.write = false;
	inst->op.id = dev->id;
	inst->op.idx = idx;
	inst->op.sidx = sidx;
	inst->op.tout = timeout_ms ? timeout_ms : dev->timeout;
	inst->op.re_txn = retry ? retry : dev->retry;
	inst->op.ttl = inst->op.tout * 2;
	inst->op.len = *len;

	usbcan_send_sdo_req(inst, false, dev->id, idx, sidx, timeout_ms, retry,	data, *len);

	pthread_cond_wait(&inst->cond, &inst->mutex);
	pthread_mutex_unlock(&inst->mutex);

	if(!inst->op.abt)
	{
		if(inst->op.len > *len)
		{
			LOG_WARN(debug_log, "%s: supplied buffer of %d bytes to small, %d bytes required", __func__, *len, inst->op.len);
		}
		else
		{
			*len = inst->op.len;
		}
		memcpy(data, inst->op.data, *len);
	}
	else
	{
		LOG_ERROR(debug_log, "%s: SDO failed id(%d) idx(0x%X) sidx(%d), len(%d), re_txn(%d), tout(%d) with abort-code(0x%.X):\n    %s", 
					__func__,
					inst->op.id,
					(unsigned int)idx, 
					(int)sidx, 
					*len, 
					inst->op.re_txn, 
					inst->op.tout, 
					(unsigned int)inst->op.abt, 
					sdo_describe_error(inst->op.abt));
	}

	return inst->op.abt;
}



