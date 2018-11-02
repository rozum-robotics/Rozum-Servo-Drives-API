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
#ifdef _WIN32
#include "windows.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>
#endif
#include <pthread.h>
#include "usbcan_config.h"
#include "co_common.h"

#define TIME_DELTA_MS(x, y) ((x.tv_sec - y.tv_sec) * 1000LL + (x.tv_usec - y.tv_usec) / 1000LL)
#define TIME_DELTA_US(x, y) ((x.tv_sec - y.tv_sec) * 1000000LL + (x.tv_usec - y.tv_usec) )

typedef struct
{
	uint32_t id;
	uint8_t dlc;
	uint8_t data[8];
} can_msg_t;

typedef struct usbcan_instance_t usbcan_instance_t;
typedef struct usbcan_device_t usbcan_device_t;

typedef enum
{
	OP_NONE,
	OP_WAIT_DEV,
	OP_SDO,

} usbcan_op_code_t;

typedef struct
{
		usbcan_op_code_t code;
		int tout;
		bool write;
		int id;
		int idx;
		int sidx;
		int len;
		int ttl;
		int re_txn;
		uint8_t data[8192];
		uint32_t abt;
} usbcan_op_t;

typedef struct
{
	int h;
	int t;
	uint8_t *b;
	uint8_t *rb;
	#ifdef _WIN32
	DWORD l;
	#else
	int l;
	#endif
} usbcan_rx_data_t;

struct usbcan_instance_t
{
	void *udata;
	const char *device;
	#ifdef _WIN32
	HANDLE fd;
	OVERLAPPED fd_overlap_read, fd_overlap_write, fd_overlap_evt;
	#else
	int fd;
	#endif
	
	usbcan_rx_data_t rx_data;

	usbcan_device_t *device_list;

	pthread_t usbcan_thread;
	pthread_mutex_t mutex, mutex_write;
	pthread_cond_t cond;

	usbcan_op_t op;

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

typedef void (*usbcan_hb_tx_cb_t)(usbcan_instance_t *inst);
typedef void (*usbcan_hb_rx_cb_t)(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state);
typedef void (*usbcan_nmt_state_cb_t)(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state);
typedef void (*usbcan_com_frame_cb_t)(usbcan_instance_t *inst, can_msg_t *m);
typedef void (*usbcan_emcy_cb_t)(usbcan_instance_t *inst, int id, uint16_t err_code, uint8_t err_reg, uint8_t err_bits, uint32_t err_info);

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
int wait_device(usbcan_instance_t *inst, int id, int timeout_ms);
uint32_t write_raw_sdo(usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int len, int retry, int timeout_ms);
uint32_t read_raw_sdo(usbcan_device_t *dev, uint16_t idx, uint8_t sidx, uint8_t *data, int *len, int retry, int timeout_ms);
int write_com_frame(usbcan_instance_t *inst, can_msg_t *msg);
int write_timestamp(usbcan_instance_t *inst, uint32_t ts);
int write_nmt(usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd);

#ifdef __cplusplus
}
#endif

#endif

