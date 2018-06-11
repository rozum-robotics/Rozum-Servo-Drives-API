#ifndef __USBCAN_PROTO_H__
#define __USBCAN_PROTO_H__

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

typedef enum
{
	COM_FRAME = 0, 
	COM_NMT,
	COM_HB,
	COM_TIMESTAMP,
	COM_SDO_TX_REQ,
	COM_SDO_TX_RESP,
	COM_SDO_RX_REQ,
	COM_SDO_RX_RESP,
	COM_SYNC,
	COM_EMCY,
	COM_PDO
} usbcan_frame_type_t;


typedef enum 
{
	_CO_NMT_INITIALIZING = 0,      /**< Device is initializing */
	_CO_NMT_PRE_OPERATIONAL = 127, /**< Device is in pre-operational state */
	_CO_NMT_OPERATIONAL = 5,       /**< Device is in operational state */
	_CO_NMT_STOPPED = 4,           /**< Device is stopped */
	_CO_NMT_HB_TIMEOUT = -1,        
} usbcan_nmt_state_t;

typedef enum
{
	_CO_NMT_CMD_GOTO_OP = 0x01,	//Go to 'operational'
	_CO_NMT_CMD_GOTO_STOPPED = 0x02,	//Go to 'stopped'
	_CO_NMT_CMD_GOTO_PREOP = 0x80,	//Go to 'pre-operational'
	_CO_NMT_CMD_RESET_NODE = 0x81,	//Go to 'reset node'
	_CO_NMT_CMD_RESET_COMM = 0x82,	//Go to 'reset communication'
} usbcan_nmt_cmd_t;


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
} usbcan_wait_for_t;

typedef struct
{
	char *device;
	int fd;

	pthread_t usbcan_thread;
	int to_master_pipe[2];
	int to_child_pipe[2];

	void *usbcan_hb_tx_cb;
	void *usbcan_hb_rx_cb;
	void *usbcan_emcy_cb;
	void *usbcan_nmt_state_cb;
	void *usbcan_com_frame_cb;

	int64_t master_hb_ival;// = USB_CAN_MASTER_HB_IVAL_MS;
	int64_t master_hb_timer;
	int64_t hb_alive_threshold;// = USB_CAN_HB_ALIVE_THRESHOLD_MS;

	int64_t dev_alive[USB_CAN_MAX_DEV];
	int64_t dev_hb_ival[USB_CAN_MAX_DEV];
	usbcan_nmt_state_t dev_state[USB_CAN_MAX_DEV];

	usbcan_wait_for_t wait_for;// = {0};
	bool inhibit_master_hb;// = false;

	bool usbcan_udp;// = false;
} usbcan_instance_t;

typedef struct
{
	usbcan_instance_t *inst;
	int id;
	uint32_t timeout;
	uint32_t retry;
} usbcan_device_t;

typedef void (*sdo_resp_cb_t)(usbcan_instance_t *inst, uint32_t abt, uint8_t *data, int len);
typedef void (*usbcan_hb_tx_cb_t)(usbcan_instance_t *inst);
typedef void (*usbcan_hb_rx_cb_t)(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state);
typedef void (*usbcan_nmt_state_cb_t)(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state);
typedef void (*usbcan_com_frame_cb_t)(usbcan_instance_t *inst, can_msg_t *m);
typedef void (*usbcan_emcy_cb_t)(usbcan_instance_t *inst, int id, uint16_t err_code, uint8_t err_reg, uint8_t err_bits, uint32_t err_info);



extern const char *CAN_OPEN_CMD[];

void usbcan_setup_hb_tx_cb(usbcan_instance_t *inst, usbcan_hb_tx_cb_t cb, int64_t to);
void usbcan_setup_hb_rx_cb(usbcan_instance_t *inst, usbcan_hb_rx_cb_t cb);
void usbcan_setup_emcy_cb(usbcan_instance_t *inst, usbcan_emcy_cb_t cb);
void usbcan_setup_nmt_state_cb(usbcan_instance_t *inst, usbcan_nmt_state_cb_t cb);
void usbcan_setup_com_frame_cb(usbcan_instance_t *inst, usbcan_com_frame_cb_t cb);

void usbcan_poll(usbcan_instance_t *inst, int64_t delta_ms);
int usbcan_send_sdo_req(usbcan_instance_t *inst, usbcan_sdo_t *sdo, void *data, uint16_t len, sdo_resp_cb_t cb);
void send_master_hb(usbcan_instance_t *inst);
int usbcan_send_com_frame(usbcan_instance_t *inst, can_msg_t *m);
int usbcan_send_nmt(usbcan_instance_t *inst, int id, usbcan_nmt_cmd_t cmd);
usbcan_nmt_state_t usbcan_get_device_state(usbcan_instance_t *inst, int id);
int64_t usbcan_get_hb_interval(usbcan_instance_t *inst, int id);
int usbcan_send_hb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state);
int usbcan_send_timestamp(usbcan_instance_t *inst, uint32_t ts);
void usbcan_init(usbcan_instance_t *inst);
void usbcan_inhibit_master_hb(usbcan_instance_t *inst, bool inh);

int usbcan_instance_init(usbcan_instance_t *inst, const char *dev_name);

#endif

