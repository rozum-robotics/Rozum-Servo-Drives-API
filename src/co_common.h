#ifndef __CO_COMMON_DOT_H__
#define __CO_COMMON_DOT_H__

#include <stdint.h>


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
	CO_NMT_INITIALIZING = 0,      /**< Device is initializing */
	CO_NMT_PRE_OPERATIONAL = 127, /**< Device is in pre-operational state */
	CO_NMT_OPERATIONAL = 5,       /**< Device is in operational state */
	CO_NMT_STOPPED = 4,           /**< Device is stopped */
	CO_NMT_HB_TIMEOUT = -1,        
} usbcan_nmt_state_t;

typedef enum
{
	CO_NMT_CMD_GOTO_OP = 0x01,	//Go to 'operational'
	CO_NMT_CMD_GOTO_STOPPED = 0x02,	//Go to 'stopped'
	CO_NMT_CMD_GOTO_PREOP = 0x80,	//Go to 'pre-operational'
	CO_NMT_CMD_RESET_NODE = 0x81,	//Go to 'reset node'
	CO_NMT_CMD_RESET_COMM = 0x82,	//Go to 'reset communication'
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


const char *sdo_describe_error(uint32_t err);

extern const char *CAN_OPEN_CMD[];


#endif
