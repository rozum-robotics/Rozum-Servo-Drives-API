#ifndef __CO_COMMON_DOT_H__
#define __CO_COMMON_DOT_H__

#ifdef __cplusplus
extern "C"
{
#endif

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
    CO_NMT_BOOT = 2,              /**< Device is in bootloader */
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
} co_sdo_abort_code_t;


typedef enum
{
	CO_EM_NO_ERROR                  = 0x00U, /**< 0x00, Error Reset or No Error */
	CO_EM_CAN_BUS_WARNING           = 0x01U, /**< 0x01, communication, info, CAN bus warning limit reached */
	CO_EM_RXMSG_WRONG_LENGTH        = 0x02U, /**< 0x02, communication, info, Wrong data length of the received CAN message */
	CO_EM_RXMSG_OVERFLOW            = 0x03U, /**< 0x03, communication, info, Previous received CAN message wasn't processed yet */
	CO_EM_RPDO_WRONG_LENGTH         = 0x04U, /**< 0x04, communication, info, Wrong data length of received PDO */
	CO_EM_RPDO_OVERFLOW             = 0x05U, /**< 0x05, communication, info, Previous received PDO wasn't processed yet */
	CO_EM_CAN_RX_BUS_PASSIVE        = 0x06U, /**< 0x06, communication, info, CAN receive bus is passive */
	CO_EM_CAN_TX_BUS_PASSIVE        = 0x07U, /**< 0x07, communication, info, CAN transmit bus is passive */

	CO_EM_NMT_WRONG_COMMAND         = 0x08U, /**< 0x08, communication, info, Wrong NMT command received */
	CO_EM_09_unused                 = 0x09U, /**< 0x09, (unused) */
	CO_EM_0A_unused                 = 0x0AU, /**< 0x0A, (unused) */
	CO_EM_0B_unused                 = 0x0BU, /**< 0x0B, (unused) */
	CO_EM_0C_unused                 = 0x0CU, /**< 0x0C, (unused) */
	CO_EM_0D_unused                 = 0x0DU, /**< 0x0D, (unused) */
	CO_EM_0E_unused                 = 0x0EU, /**< 0x0E, (unused) */
	CO_EM_0F_unused                 = 0x0FU, /**< 0x0F, (unused) */

	CO_EM_10_unused                 = 0x10U, /**< 0x10, (unused) */
	CO_EM_11_unused                 = 0x11U, /**< 0x11, (unused) */
	CO_EM_CAN_TX_BUS_OFF            = 0x12U, /**< 0x12, communication, critical, CAN transmit bus is off */
	CO_EM_CAN_RXB_OVERFLOW          = 0x13U, /**< 0x13, communication, critical, CAN module receive buffer has overflowed */
	CO_EM_CAN_TX_OVERFLOW           = 0x14U, /**< 0x14, communication, critical, CAN transmit buffer has overflowed */
	CO_EM_TPDO_OUTSIDE_WINDOW       = 0x15U, /**< 0x15, communication, critical, TPDO is outside SYNC window */
	CO_EM_16_unused                 = 0x16U, /**< 0x16, (unused) */
	CO_EM_17_unused                 = 0x17U, /**< 0x17, (unused) */

	CO_EM_SYNC_TIME_OUT             = 0x18U, /**< 0x18, communication, critical, SYNC message timeout */
	CO_EM_SYNC_LENGTH               = 0x19U, /**< 0x19, communication, critical, Unexpected SYNC data length */
	CO_EM_PDO_WRONG_MAPPING         = 0x1AU, /**< 0x1A, communication, critical, Error with PDO mapping */
	CO_EM_1B_unused                 = 0x1BU, /**< 0x1B, (unused) */
	CO_EM_HB_CONSUMER_REMOTE_RESET  = 0x1CU, /**< 0x1C, communication, critical, Heartbeat consumer detected remote node reset */
	CO_EM_1D_unused                 = 0x1DU, /**< 0x1D, (unused) */
	CO_EM_1E_unused                 = 0x1EU, /**< 0x1E, (unused) */
	CO_EM_1F_unused                 = 0x1FU, /**< 0x1F, (unused) */

	CO_EM_EMERGENCY_BUFFER_FULL     = 0x20U, /**< 0x20, generic, info, Emergency buffer is full, Emergency message wasn't sent */
	CO_EM_MOTION_ERROR              = 0x21U, /**< 0x21, generic, critical, Motion Error */
	CO_EM_MICROCONTROLLER_RESET     = 0x22U, /**< 0x22, generic, info, Microcontroller has just started */
	CO_EM_UNAUTHORIZED_ACCESS       = 0x23U, /**< 0x23, generic, critical, Access is only available to service engineer */
	CO_EM_TEMPERATURE_ERROR         = 0x24U, /**< 0x24, generic, critical, Temperature is too high */
	CO_EM_TEMPERATURE_INTRNL_ERROR  = 0x25U, /**< 0x25, generic, critical, Internal temperature is too high */
	CO_EM_HARDWARE_ERROR            = 0x26U, /**< 0x26, generic, critical, Hardware error (driver error) */
	CO_EM_MOTION_INVALID            = 0x27U, /**< 0x27, generic, critical, Invalid motion command received */

	CO_EM_WRONG_ERROR_REPORT        = 0x28U, /**< 0x28, generic, critical, Wrong parameters to CO_EM_reportError() function */
	CO_EM_ISR_TIMER_OVERFLOW        = 0x29U, /**< 0x29, generic, critical, Timer task has overflowed */
	CO_EM_MEMORY_ALLOCATION_ERROR   = 0x2AU, /**< 0x2A, generic, critical, Unable to allocate memory for objects */
	CO_EM_GENERIC_ERROR             = 0x2BU, /**< 0x2B, generic, critical, Generic error, test usage */
	CO_EM_GENERIC_SOFTWARE_ERROR    = 0x2CU, /**< 0x2C, generic, critical, Software error */
	CO_EM_INCONSISTENT_OBJECT_DICT  = 0x2DU, /**< 0x2D, generic, critical, Object dictionary does not match the software */
	CO_EM_CALCULATION_OF_PARAMETERS = 0x2EU, /**< 0x2E, generic, critical, Error in calculation of device parameters */
	CO_EM_NON_VOLATILE_MEMORY       = 0x2FU, /**< 0x2F, generic, critical, Error with access to non volatile device memory */

	CO_EM_FLT_CONFIG_CONSTRAINT     = 0x30U, /**< 0x30, fault, critical, Constraint was applied to the settings */
	CO_EM_FLT_CONFIG_CRC            = 0x31U, /**< 0x31, fault, critical, CRC check of the setings failed */
	CO_EM_FLT_NTC                   = 0x32U, /**< 0x32, fault, critical, NTC Error */
	CO_EM_FLT_CS0                   = 0x33U, /**< 0x33, fault, critical, Current sensor 0 error */
	CO_EM_FLT_CS1                   = 0x34U, /**< 0x34, fault, critical, Current sensor 1 error */
	CO_EM_FLT_CS2                   = 0x35U, /**< 0x35, fault, critical, Current sensor 2 error */
	CO_EM_FLT_DRIVER                = 0x36U, /**< 0x36, fault, critical, Driver error */
	CO_EM_FLT_VS0                   = 0x37U, /**< 0x37, fault, critical, Voltage sensor error */

	CO_EM_FLT_ENC_M_OFF             = 0x38U, /**< 0x38, fault, critical, Encoder disconnected */
	CO_EM_FLT_ENC_G_OFF             = 0x39U, /**< 0x39, fault, critical, Encoder disconnected */
	CO_EM_FLT_ENC_M_STUP_CRC        = 0x3AU, /**< 0x3A, fault, critical, CRC_ERR/EPR_ERR in STATUS1 & STUP in STATUS0 */
	CO_EM_FLT_ENC_G_STUP_CRC        = 0x3BU, /**< 0x3B, fault, critical, CRC_ERR/EPR_ERR in STATUS1 & STUP in STATUS0 */
	CO_EM_FLT_ENC_M_LEVEL           = 0x3CU, /**< 0x3C, fault, critical, FRQ_ABZ/FRQ_CNV in STATUS1 & AN_MAX/AN_MIN/AM_MAX/AM_MIN in STATUS0 */
	CO_EM_FLT_ENC_G_LEVEL           = 0x3DU, /**< 0x3D, fault, critical, FRQ_ABZ/FRQ_CNV in STATUS1 & AN_MAX/AN_MIN/AM_MAX/AM_MIN in STATUS0 */
	CO_EM_FLT_ENC_M_SIG             = 0x3EU, /**< 0x3E, fault, critical, NON_CTR bit in STATUS1 - Only during motion test */
	CO_EM_FLT_ENC_G_SIG             = 0x3FU, /**< 0x3F, fault, critical, NON_CTR bit in STATUS1 - Only during motion test */

	CO_EM_HW_VOLT_LO				= 0x40U,
	CO_EM_HW_VOLT_HI				= 0x41U,
	CO_EM_HW_CUR_LIMIT				= 0x42U,
	CO_EM_POWER_ERROR               = 0x43U,
	CO_EM_FORCE_ERROR               = 0x44U,
	CO_EM_HEARTBEAT_CONSUMER        = 0x45U, /**< 0x45, communication, critical, Heartbeat consumer timeout */

} co_emcy_err_bits_t;

typedef enum
{
	CO_EMC_NO_ERROR                 = 0x0000U, /**< 0x00xx, error Reset or No Error */
	CO_EMC_GENERIC                  = 0x1000U, /**< 0x10xx, Generic Error */
	CO_EMC_CURRENT                  = 0x2000U, /**< 0x20xx, Current */
	CO_EMC_CURRENT_INPUT            = 0x2100U, /**< 0x21xx, Current, device input side */
	CO_EMC_CURRENT_INSIDE           = 0x2200U, /**< 0x22xx, Current inside the device */
	CO_EMC_CURRENT_OUTPUT           = 0x2300U, /**< 0x23xx, Current, device output side */
	CO_EMC_VOLTAGE                  = 0x3000U, /**< 0x30xx, Voltage */
	CO_EMC_VOLTAGE_MAINS            = 0x3100U, /**< 0x31xx, Mains Voltage */
	CO_EMC_VOLTAGE_INSIDE           = 0x3200U, /**< 0x32xx, Voltage inside the device */
	CO_EMC_VOLTAGE_OUTPUT           = 0x3300U, /**< 0x33xx, Output Voltage */
	CO_EMC_TEMPERATURE              = 0x4000U, /**< 0x40xx, Temperature */
	CO_EMC_TEMP_AMBIENT             = 0x4100U, /**< 0x41xx, Ambient Temperature */
	CO_EMC_TEMP_DEVICE              = 0x4200U, /**< 0x42xx, Device Temperature */
	CO_EMC_HARDWARE                 = 0x5000U, /**< 0x50xx, Device Hardware */
	CO_EMC_SOFTWARE_DEVICE          = 0x6000U, /**< 0x60xx, Device Software */
	CO_EMC_SOFTWARE_INTERNAL        = 0x6100U, /**< 0x61xx, Internal Software */
	CO_EMC_SOFTWARE_USER            = 0x6200U, /**< 0x62xx, User Software */
	CO_EMC_DATA_SET                 = 0x6300U, /**< 0x63xx, Data Set */
	CO_EMC_ADDITIONAL_MODUL         = 0x7000U, /**< 0x70xx, Additional Modules */
	CO_EMC_MONITORING               = 0x8000U, /**< 0x80xx, Monitoring */
	CO_EMC_COMMUNICATION            = 0x8100U, /**< 0x81xx, Communication */
	CO_EMC_CAN_OVERRUN              = 0x8110U, /**< 0x8110, CAN Overrun (Objects lost) */
	CO_EMC_CAN_PASSIVE              = 0x8120U, /**< 0x8120, CAN in Error Passive Mode */
	CO_EMC_HEARTBEAT                = 0x8130U, /**< 0x8130, Life Guard Error or Heartbeat Error */
	CO_EMC_BUS_OFF_RECOVERED        = 0x8140U, /**< 0x8140, recovered from bus off */
	CO_EMC_CAN_ID_COLLISION         = 0x8150U, /**< 0x8150, CAN-ID collision */
	CO_EMC_PROTOCOL_ERROR           = 0x8200U, /**< 0x82xx, Protocol Error */
	CO_EMC_PDO_LENGTH               = 0x8210U, /**< 0x8210, PDO not processed due to length error */
	CO_EMC_PDO_LENGTH_EXC           = 0x8220U, /**< 0x8220, PDO length exceeded */
	CO_EMC_DAM_MPDO                 = 0x8230U, /**< 0x8230, DAM MPDO not processed, destination object not available */
	CO_EMC_SYNC_DATA_LENGTH         = 0x8240U, /**< 0x8240, Unexpected SYNC data length */
	CO_EMC_RPDO_TIMEOUT             = 0x8250U, /**< 0x8250, RPDO timeout */
	CO_EMC_EXTERNAL_ERROR           = 0x9000U, /**< 0x90xx, External Error */
	CO_EMC_ADDITIONAL_FUNC          = 0xF000U, /**< 0xF0xx, Additional Functions */
	CO_EMC_DEVICE_SPECIFIC          = 0xFF00U, /**< 0xFFxx, Device specific */

	CO_EMC401_OUT_CUR_HI            = 0x2310U, /**< 0x2310, DS401, Current at outputs too high (overload) */
	CO_EMC401_OUT_SHORTED           = 0x2320U, /**< 0x2320, DS401, Short circuit at outputs */
	CO_EMC401_OUT_LOAD_DUMP         = 0x2330U, /**< 0x2330, DS401, Load dump at outputs */
	CO_EMC401_IN_VOLT_HI            = 0x3110U, /**< 0x3110, DS401, Input voltage too high */
	CO_EMC401_IN_VOLT_LOW           = 0x3120U, /**< 0x3120, DS401, Input voltage too low */
	CO_EMC401_INTERN_VOLT_HI        = 0x3210U, /**< 0x3210, DS401, Internal voltage too high */
	CO_EMC401_INTERN_VOLT_LO        = 0x3220U, /**< 0x3220, DS401, Internal voltage too low */
	CO_EMC401_OUT_VOLT_HIGH         = 0x3310U, /**< 0x3310, DS401, Output voltage too high */
	CO_EMC401_OUT_VOLT_LOW          = 0x3320U, /**< 0x3320, DS401, Output voltage too low */
	CO_EMC401_POWER_TEMP_OVER       = 0x4210U, /**< 0x4210, Excess temperature of the inverter */
	CO_EMC401_MOTOR_TEMP_OVER       = 0x4290U, /**< 0x4290, Excess temperature of the motor */
	CO_EMC401_SYS_ERROR             = 0x50A0U, /**< 0x50A0, System error */
	CO_EMC401_POINT_ERROR           = 0x50B0U, /**< 0x50B0, System error: invalid motion point */
	CO_EMC401_CURR_MEAS_OFFSET      = 0x5210U, /**< 0x5210, Control: Measurement circuit, Current measurement offset */
	CO_EMC401_EE_FAULT				= 0x5430U, /**< 0x5430, EEPROM Fault */
	CO_EMC401_EE_CRC_ERROR			= 0x5530U, /**< 0x5530, EEPROM checksum error */
	CO_EMC401_CONF_ERROR            = 0x6320U, /**< 0x6320, Configuration error */
	CO_EMC401_ENC_CNT_ERROR         = 0x7305U, /**< 0x7305, Encoder counting error */
	CO_EMC401_VEL_FLW_ERROR         = 0x8400U, /**< 0x8400, Velocity controller following error */
	CO_EMC401_POS_LIMIT         	= 0x8610U, /**< 0x8610, Position controller limits */
	CO_EMC401_POS_FLW_ERROR         = 0x8611U, /**< 0x8611, Position controller following error */
	CO_EMC401_POS_FLW_STATIC_ERROR  = 0x8612U, /**< 0x8611, Position controller following (static) error */
	CO_EMC401_ACCESS_ERROR			= 0xFF10U, /**< 0xFF10, Manufacturer specific, Unauthorized access */
	CO_EMC401_PWRCTRL_ERROR			= 0xFF80U, /**< 0xFF80, Manufacturer specific, Power Stage Controller Error */
	CO_EMC401_BUSY					= 0xFFA0U, /**< 0xFFA0, Busy */
	CO_EMC401_PROCEDURE_ERROR       = 0xFFA2U, /**< 0xFFA2, Procedure error */
	CO_EMC401_FORCE_OVER	        = 0xFFA3U, /**< 0xFFA3, Over force */
	CO_EMC401_POWER_OVER            = 0xFFA4U, /**< 0xFFA4, Over power */
} co_emcy_code_t;

const char *sdo_describe_error(uint32_t err);

extern const char *CAN_OPEN_CMD[];

#ifdef __cplusplus
}
#endif

#endif
