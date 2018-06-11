#ifndef __USBCAN_IPC_H__
#define __USBCAN_IPC_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "usbcan_proto.h"
#include "usbcan_types.h"
#include "usbcan_util.h"
#include "usbcan_config.h"

typedef enum
{
	IPC_COM_FRAME,
	IPC_TIMESTAMP,
	IPC_SDO,
	IPC_EMCY,
	IPC_NMT,
} ipc_opcode_t;

typedef struct
{
	int id;
	uint16_t code;
	uint8_t reg;
	uint8_t bits;
	uint32_t info;
} ipc_emcy_t;

typedef struct
{
	int id;
	usbcan_nmt_cmd_t cmd;
} ipc_nmt_t;

typedef struct
{
    usbcan_sdo_t sdo;
    int data_len;
} ipc_sdo_req_t;

typedef struct
{
    usbcan_sdo_t sdo;
    uint32_t abt;
    int data_len;
} ipc_sdo_resp_t;

uint32_t write_raw_sdo(usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int len);
uint32_t read_raw_sdo(usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int *len);
void write_com_frame(usbcan_instance_t *inst, can_msg_t *msg);
void write_timestamp(usbcan_instance_t *inst, uint32_t ts);
void write_nmt(usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd);
void ipc_process(usbcan_instance_t *inst);
void ipc_create_link(usbcan_instance_t *inst);

#endif
