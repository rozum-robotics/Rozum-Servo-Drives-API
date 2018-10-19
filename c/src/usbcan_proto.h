#ifndef __USBCAN_PROTO_H__
#define __USBCAN_PROTO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>
#include <pthread.h>
#include "usbcan_config.h"
#include "co_common.h"

#define TIME_DELTA_MS(x, y) ((x.tv_sec - y.tv_sec) * 1000 + (x.tv_usec - y.tv_usec) / 1000)
#define TIME_DELTA_US(x, y) ((x.tv_sec - y.tv_sec) * 1000000 + (x.tv_usec - y.tv_usec) )

typedef struct
{
	uint32_t id;
	uint8_t dlc;
	uint8_t data[8];
} can_msg_t;

typedef struct
{
	bool write;
	uint8_t id;
	uint16_t idx;
	uint8_t sidx;
	uint16_t tout;
	uint8_t re_txn;
	int32_t ttl;
} usbcan_sdo_t;

typedef struct
{
	void *cb;
	usbcan_sdo_t sdo;
} usbcan_wait_sdo_t;

typedef struct
{
	int timer;
	int id;
} usbcan_wait_device_t;

typedef struct usbcan_instance_t usbcan_instance_t;
typedef struct usbcan_device_t usbcan_device_t;

struct usbcan_instance_t
{
	void *udata;
	const char *device;
	int fd;

	usbcan_device_t *device_list;

	pthread_t usbcan_thread;
	int to_master_pipe[2];
	int to_child_pipe[2];

	void *usbcan_hb_tx_cb;
	void *usbcan_hb_rx_cb;
	void *usbcan_emcy_cb;
	void *usbcan_nmt_state_cb;
	void *usbcan_com_frame_cb;

	int64_t master_hb_ival;
	int64_t master_hb_timer;
	int64_t hb_alive_threshold;

	int64_t dev_alive[USB_CAN_MAX_DEV];
	int64_t dev_hb_ival[USB_CAN_MAX_DEV];
	usbcan_nmt_state_t dev_state[USB_CAN_MAX_DEV];

	usbcan_wait_sdo_t wait_sdo;
	usbcan_wait_device_t wait_device;
	bool inhibit_master_hb;
	bool usbcan_udp;

	FILE *comm_log;
	bool running;
};

struct usbcan_device_t
{
	usbcan_instance_t *inst;
	int id;
	uint32_t timeout;
	uint32_t retry;
	usbcan_device_t *next;
};

typedef void (*sdo_resp_cb_t)(usbcan_instance_t *inst, uint32_t abt, uint8_t *data, int len);
typedef void (*usbcan_hb_tx_cb_t)(usbcan_instance_t *inst);
typedef void (*usbcan_hb_rx_cb_t)(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state);
typedef void (*usbcan_nmt_state_cb_t)(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state);
typedef void (*usbcan_com_frame_cb_t)(usbcan_instance_t *inst, can_msg_t *m);
typedef void (*usbcan_emcy_cb_t)(usbcan_instance_t *inst, int id, uint16_t err_code, uint8_t err_reg, uint8_t err_bits, uint32_t err_info);

typedef enum
{
	IPC_COM_FRAME,
	IPC_TIMESTAMP,
	IPC_SDO,
	IPC_EMCY,
	IPC_NMT,
	IPC_WAIT_DEVICE,
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
	int id;
	int timeout_ms;
} ipc_wait_device_t;

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


extern const char *CAN_OPEN_CMD[];
extern FILE *debug_log;

void usbcan_setup_hb_tx_cb(usbcan_instance_t *inst, usbcan_hb_tx_cb_t cb, int64_t to);
void usbcan_setup_hb_rx_cb(usbcan_instance_t *inst, usbcan_hb_rx_cb_t cb);
void usbcan_setup_emcy_cb(usbcan_instance_t *inst, usbcan_emcy_cb_t cb);
void usbcan_setup_nmt_state_cb(usbcan_instance_t *inst, usbcan_nmt_state_cb_t cb);
void usbcan_setup_com_frame_cb(usbcan_instance_t *inst, usbcan_com_frame_cb_t cb);

usbcan_nmt_state_t usbcan_get_device_state(usbcan_instance_t *inst, int id);
int64_t usbcan_get_hb_interval(usbcan_instance_t *inst, int id);
void usbcan_inhibit_master_hb(usbcan_instance_t *inst, bool inh);

void usbcan_set_comm_log_stream(usbcan_instance_t *inst, FILE *f);
void usbcan_set_debug_log_stream(FILE *f);

usbcan_instance_t *usbcan_instance_init(const char *dev_name);
int usbcan_instance_deinit(usbcan_instance_t **inst);
usbcan_device_t *usbcan_device_init(usbcan_instance_t *inst, int id);
int usbcan_device_deinit(usbcan_device_t **dev);

/*
 * User thread functions
 */
int wait_device(const usbcan_instance_t *inst, int id, int timeout_ms);
uint32_t write_raw_sdo(const usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int len, int retry, int timeout_ms);
uint32_t read_raw_sdo(const usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int *len, int retry, int timeout_ms);
int write_com_frame(const usbcan_instance_t *inst, can_msg_t *msg);
int write_timestamp(const usbcan_instance_t *inst, uint32_t ts);
int write_nmt(const usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd);

#ifdef __cplusplus
}
#endif

#endif

