#include <errno.h>
#include "usbcan_ipc.h"
#include "logging.h"


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
			exit(1);
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
			exit(1);
		}
	}
	return bytes;
}

void sdo_resp_cb(usbcan_instance_t *inst, uint32_t abt, uint8_t *data, int len)
{
    ipc_sdo_resp_t r;

    r.sdo = inst->wait_for.sdo;
    r.abt = abt;
    r.data_len = len;
    write_sig_safe(inst->to_master_pipe[1], &r, sizeof(r));
    if(data && len)
    {
        write_sig_safe(inst->to_master_pipe[1], data, len);
    }
}

void write_nmt(const usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd)
{
	ipc_opcode_t opcode = IPC_NMT;
	ipc_nmt_t nmt = {.id = id, .cmd = cmd};

    write_sig_safe(inst->to_child_pipe[1], &opcode, sizeof(opcode));
    write_sig_safe(inst->to_child_pipe[1], &nmt, sizeof(nmt));
}

void write_timestamp(const usbcan_instance_t *inst, uint32_t ts)
{
	ipc_opcode_t opcode = IPC_TIMESTAMP;

    write_sig_safe(inst->to_child_pipe[1], &opcode, sizeof(opcode));
    write_sig_safe(inst->to_child_pipe[1], &ts, sizeof(ts));
}

void write_com_frame(const usbcan_instance_t *inst, can_msg_t *msg)
{
	ipc_opcode_t opcode = IPC_COM_FRAME;

    write_sig_safe(inst->to_child_pipe[1], &opcode, sizeof(opcode));
    write_sig_safe(inst->to_child_pipe[1], msg, sizeof(can_msg_t));
}

uint32_t write_raw_sdo(const usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int len, int retry, int timeout)
{
    ipc_sdo_req_t req;
    ipc_sdo_resp_t resp;
	ipc_opcode_t opcode = IPC_SDO;

    req.sdo.write = true;
    req.sdo.id = dev->id;
    req.sdo.idx = idx;
    req.sdo.sidx = sidx;
    req.sdo.tout = timeout ? timeout : dev->timeout;
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
        LOG_ERROR("SDO write failed idx(0x%X) sidx(%d), len(%d), re_txn(%d), tout(%d) with abort-code(0x%.X)",
                  (unsigned int)idx, (int)sidx, len, req.sdo.re_txn, req.sdo.tout, (unsigned int)resp.abt);
    }

    return resp.abt;
}

uint32_t read_raw_sdo(const usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int *len, int retry, int timeout)
{
    ipc_sdo_req_t req;
    ipc_sdo_resp_t resp;
	ipc_opcode_t opcode = IPC_SDO;

    req.sdo.write = false;
    req.sdo.id = dev->id;
    req.sdo.idx = idx;
    req.sdo.sidx = sidx;
    req.sdo.tout = timeout ? timeout : dev->timeout;
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
            LOG_WARN("SDO read: supplied buffer of %d bytes to small, %d bytes required", *len, resp.data_len);
        }
        else
        {
            *len = resp.data_len;
        }
        read_sig_safe(dev->inst->to_master_pipe[0], data, *len);
    }
    else
    {
        LOG_ERROR("SDO read failed idx(0x%X) sidx(%d), len(%d), re_txn(%d), tout(%d) with abort-code(0x%.X)",
                  (unsigned int)idx, (int)sidx, len, req.sdo.re_txn, req.sdo.tout, (unsigned int)resp.abt);
    }

    return resp.abt;
}

void ipc_create_link(usbcan_instance_t *inst)
{
    if(pipe(inst->to_master_pipe))
    {
        LOG_ERROR("can't create master pipe");
        exit(2);
    }

    if(pipe(inst->to_child_pipe))
    {
        LOG_ERROR("can't create child pipe");
        exit(2);
    }
}

void ipc_process(usbcan_instance_t *inst)
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
			break;
		default:
			break;
	}
}

