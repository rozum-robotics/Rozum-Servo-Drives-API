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


typedef enum
{
	CO_SDO_AB_NONE                  = 0x00000000UL, /**< 0x00000000, No abort */
	CO_SDO_AB_TOGGLE_BIT            = 0x05030000UL, /**< 0x05030000, Toggle bit not altered */
	CO_SDO_AB_TIMEOUT               = 0x05040000UL, /**< 0x05040000, SDO protocol timed out */
	CO_SDO_AB_CMD                   = 0x05040001UL, /**< 0x05040001, Command specifier not valid or unknown */
	CO_SDO_AB_BLOCK_SIZE            = 0x05040002UL, /**< 0x05040002, Invalid block size in block mode */
	CO_SDO_AB_SEQ_NUM               = 0x05040003UL, /**< 0x05040003, Invalid sequence number in block mode */
	CO_SDO_AB_CRC                   = 0x05040004UL, /**< 0x05040004, CRC error (block mode only) */
	CO_SDO_AB_OUT_OF_MEM            = 0x05040005UL, /**< 0x05040005, Out of memory */
	CO_SDO_AB_UNSUPPORTED_ACCESS    = 0x06010000UL, /**< 0x06010000, Unsupported access to an object */
	CO_SDO_AB_WRITEONLY             = 0x06010001UL, /**< 0x06010001, Attempt to read a write only object */
	CO_SDO_AB_READONLY              = 0x06010002UL, /**< 0x06010002, Attempt to write a read only object */
	CO_SDO_AB_NOT_EXIST             = 0x06020000UL, /**< 0x06020000, Object does not exist */
	CO_SDO_AB_NO_MAP                = 0x06040041UL, /**< 0x06040041, Object cannot be mapped to the PDO */
	CO_SDO_AB_MAP_LEN               = 0x06040042UL, /**< 0x06040042, Number and length of object to be mapped exceeds PDO length */
	CO_SDO_AB_PRAM_INCOMPAT         = 0x06040043UL, /**< 0x06040043, General parameter incompatibility reasons */
	CO_SDO_AB_DEVICE_INCOMPAT       = 0x06040047UL, /**< 0x06040047, General internal incompatibility in device */
	CO_SDO_AB_HW                    = 0x06060000UL, /**< 0x06060000, Access failed due to hardware error */
	CO_SDO_AB_TYPE_MISMATCH         = 0x06070010UL, /**< 0x06070010, Data type does not match, length of service parameter does not match */
	CO_SDO_AB_DATA_LONG             = 0x06070012UL, /**< 0x06070012, Data type does not match, length of service parameter too high */
	CO_SDO_AB_DATA_SHORT            = 0x06070013UL, /**< 0x06070013, Data type does not match, length of service parameter too short */
	CO_SDO_AB_SUB_UNKNOWN           = 0x06090011UL, /**< 0x06090011, Sub index does not exist */
	CO_SDO_AB_INVALID_VALUE         = 0x06090030UL, /**< 0x06090030, Invalid value for parameter (download only). */
	CO_SDO_AB_VALUE_HIGH            = 0x06090031UL, /**< 0x06090031, Value range of parameter written too high */
	CO_SDO_AB_VALUE_LOW             = 0x06090032UL, /**< 0x06090032, Value range of parameter written too low */
	CO_SDO_AB_MAX_LESS_MIN          = 0x06090036UL, /**< 0x06090036, Maximum value is less than minimum value. */
	CO_SDO_AB_NO_RESOURCE           = 0x060A0023UL, /**< 0x060A0023, Resource not available: SDO connection */
	CO_SDO_AB_GENERAL               = 0x08000000UL, /**< 0x08000000, General error */
	CO_SDO_AB_DATA_TRANSF           = 0x08000020UL, /**< 0x08000020, Data cannot be transferred or stored to application */
	CO_SDO_AB_DATA_LOC_CTRL         = 0x08000021UL, /**< 0x08000021, Data cannot be transferred or stored to application because of local control */
	CO_SDO_AB_DATA_DEV_STATE        = 0x08000022UL, /**< 0x08000022, Data cannot be transferred or stored to application because of present device state */
	CO_SDO_AB_DATA_OD               = 0x08000023UL, /**< 0x08000023, Object dictionary not present or dynamic generation fails */
	CO_SDO_AB_NO_DATA               = 0x08000024UL  /**< 0x08000024, No data available */
}CO_SDO_abortCode_t;


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
	const char *device;
	int fd;

	pthread_t usbcan_thread;
	int to_master_pipe[2];
	int to_child_pipe[2];

	void *usbcan_hb_tx_cb;
	void *usbcan_hb_rx_cb;
	void *usbcan_emcy_cb;
	void *usbcan_nmt_state_cb;
	void *usbcan_com_frame_cb;

	int64_t master_hb_ival;;
	int64_t master_hb_timer;
	int64_t hb_alive_threshold;

	int64_t dev_alive[USB_CAN_MAX_DEV];
	int64_t dev_hb_ival[USB_CAN_MAX_DEV];
	usbcan_nmt_state_t dev_state[USB_CAN_MAX_DEV];

	usbcan_wait_for_t wait_for;
	bool inhibit_master_hb;
	bool usbcan_udp;
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
void usbcan_inhibit_master_hb(usbcan_instance_t *inst, bool inh);

int usbcan_instance_init(usbcan_instance_t **inst, const char *dev_name);
int usbcan_instance_deinit(usbcan_instance_t **inst);
int usbcan_device_init(usbcan_instance_t *inst, usbcan_device_t **dev, int id);
int usbcan_device_deinit(usbcan_device_t **dev);

#endif

