/**
 * @brief Rozum Robotics API Source File
 * 
 * @file api.c
 * @author Rozum
 * @date 2018-06-01
 */

/*! \mainpage Rozum Robotics User API
 * 
 * \section intro_section Categories
 * - \ref Utils
 * - \ref Common
 * - \ref System_control
 * - \ref Servo_control
 * - \ref Servo_config
 * - \ref Servo_info
 * 
 * \defgroup Utils Utility functions
 * \defgroup Common Common functions
 * \defgroup System_control System control functions
 * \defgroup Servo_control Servo control functions
 * \defgroup Servo_config Servo configuration functions
 * \defgroup Servo_info Servo info functions
 */
/* Includes ------------------------------------------------------------------*/
#include "api.h"
#include "usbcan_proto.h"
#include "usbcan_types.h"
#include "usbcan_util.h"
#include "logging.h"
#include <stdio.h>

//! @cond Doxygen_Suppress
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RR_API_WAIT_DEVICE_TIMEOUT_MS 2000

/* Private macro -------------------------------------------------------------*/
#define BIT_SET_UINT_ARRAY(array, bit) ((array)[(bit) / 8] |= (1 << ((bit) % 8)))

#define  IS_VALID_INTERFACE(v) if(!v) return RET_BAD_INSTANCE
#define  IS_VALID_SERVO(v) if(!v) return RET_BAD_INSTANCE

#define CHECK_NMT_STATE(x)
/*                                          \
    if(x->nmt_state == CO_NMT_STOPPED || x->nmt_state == CO_NMT_BOOT) \
    {                                                                 \
        return RET_STOPPED;                                           \
    }
    */
//! @w

/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static int ret_sdo(int code)
{
    switch(code)
    {
    case CO_SDO_AB_NONE:    return RET_OK;
    case CO_SDO_AB_TIMEOUT: return RET_TIMEOUT;
    default:                return RET_ERROR;
    }
}


void rr_nmt_state_master_cb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
	rr_can_interface_t *i = (rr_can_interface_t *)inst->udata;

	LOG_INFO(debug_log, "Device %d state changed to %d:\n    '%s'", id, state, rr_describe_nmt((rr_nmt_state_t)state));

	if(i->nmt_cb)
	{
		((rr_nmt_cb_t)(i->nmt_cb))(i, id, (rr_nmt_state_t)state);
	}
}

void rr_emcy_master_cb(usbcan_instance_t *inst, int id, uint16_t code, uint8_t reg, uint8_t bits, uint32_t info)
{
	rr_can_interface_t *i = (rr_can_interface_t *)inst->udata;

	LOG_WARN(debug_log, "Emergency frame received: id(%"PRId8") code(0x%"PRIX16") reg(0x%"PRIX8") bits(0x%"PRIX8") info(0x%"PRIX32"):\n    '%s, %s'",
			id, code, reg, bits, info, rr_describe_emcy_code(code), rr_describe_emcy_bit(bits));

	if(i->emcy_cb)
	{
		((rr_emcy_cb_t)(i->emcy_cb))(i, id, code, reg, bits, info);
	}
}

/// @endcond

/**
 * @brief 
 * 
 * @param ms - time to sleep in milleseconds
 * @return void
 * @ingroup Utils
 */
void rr_sleep_ms(int ms)
{
    msleep(ms);
}

/**
 * @brief 
 * 
 * @param interface 
 * @param f stdio stream to write communication log to, no logging if 'NULL'
 * @return void
 * @ingroup Utils
 */
void rr_set_comm_log_stream(const rr_can_interface_t *interface, FILE *f)
{
	usbcan_instance_t *inst = (usbcan_instance_t *)interface->iface;
    usbcan_set_comm_log_stream(inst, f);
}

/**
 * @brief 
 * 
 * @param f stdio stream to write debug log to, no logging if 'NULL'
 * @return void
 * @ingroup Utils
 */
void rr_set_debug_log_stream(FILE *f)
{
    usbcan_set_debug_log_stream(f);
}

/**
 * @brief Register user callback for network management (NMT) events
 * 
 * @param interface to register callback on
 * @param cb (:rr_nmt_cb_t) function to be called when NMT event occures. set to NULL to disable.
 * @return void
 * @ingroup Utils
 */
void rr_setup_nmt_callback(rr_can_interface_t *interface, rr_nmt_cb_t cb)
{
	if(interface)
	{
		interface->nmt_cb = cb;
	}
}

/**
 * @brief Register user callback for Emergency (EMCY) events
 * 
 * @param interface to register callback on
 * @param cb (:rr_emcy_cb_t) function to be called when NMT event occures. Set to NULL to disable.
 * @return void
 * @ingroup Utils
 */
void rr_setup_emcy_callback(rr_can_interface_t *interface, rr_emcy_cb_t cb)
{
	if(interface)
	{
		interface->emcy_cb = cb;
	}
}

/**
 * @brief Returns descriptive string for specified NMT state code
 * 
 * @param state NMT code to descibe
 * @return void
 * @ingroup Utils
 */
const char *rr_describe_nmt(rr_nmt_state_t state)
{
	switch(state)
	{
		case RR_NMT_INITIALIZING:               return "Device is initializing";
		case RR_NMT_BOOT:                       return "Bootloader mode";
		case RR_NMT_PRE_OPERATIONAL:            return "Device is in pre-operational mode";
		case RR_NMT_OPERATIONAL:                return "Device is in operational mode";
		case RR_NMT_STOPPED:                    return "Device is in stopped mode";
		case RR_NMT_HB_TIMEOUT:                 return "Device disappeared";
		default:                                return "N/A";
	}
}

/**
 * @brief Returns descriptive string for specified EMCY codes
 * 
 * @param bit EMCY error bit field according to CanOpen standard
 * @return void
 * @ingroup Utils
 */
const char *rr_describe_emcy_bit(uint8_t bit)
{
	switch(bit)
	{
		case CO_EM_NO_ERROR:                    return "Error Reset or No Error";
		case CO_EM_CAN_BUS_WARNING:             return "CAN bus warning limit reached";
		case CO_EM_RXMSG_WRONG_LENGTH:          return "Wrong data length of the received CAN message";
		case CO_EM_RXMSG_OVERFLOW:              return "Previous received CAN message wasn't processed yet";
		case CO_EM_RPDO_WRONG_LENGTH:           return "Wrong data length of received PDO";
		case CO_EM_RPDO_OVERFLOW:               return "Previous received PDO wasn't processed yet";
		case CO_EM_CAN_RX_BUS_PASSIVE:          return "CAN Rx passive";
		case CO_EM_CAN_TX_BUS_PASSIVE:          return "CAN Tx passive";

		case CO_EM_NMT_WRONG_COMMAND:           return "Wrong NMT command received";
        //case 09-11 unused

		case CO_EM_CAN_TX_BUS_OFF:              return "CAN transmit bus is off";
		case CO_EM_CAN_RXB_OVERFLOW:            return "CAN module receive buffer has overflowed";
		case CO_EM_CAN_TX_OVERFLOW:             return "CAN transmit buffer has overflowed";
		case CO_EM_TPDO_OUTSIDE_WINDOW:         return "TPDO is outside SYNC window";

        //case 16-17 unused

		case CO_EM_SYNC_TIME_OUT:               return "SYNC message timeout";
		case CO_EM_SYNC_LENGTH:                 return "Unexpected SYNC data length";
		case CO_EM_PDO_WRONG_MAPPING:           return "Error with PDO mapping";
		case CO_EM_HB_CONSUMER_REMOTE_RESET:    return "Heartbeat consumer detected remote node reset";

        //case 1D-1F unused

		case CO_EM_EMERGENCY_BUFFER_FULL:       return "Emergency buffer is full, Emergency message wasn't sent";
		case CO_EM_MOTION_ERROR:                return "Motion Error";
		case CO_EM_MICROCONTROLLER_RESET:       return "Microcontroller has just started";
		case CO_EM_UNAUTHORIZED_ACCESS:         return "Access is only available to service engineer";
		case CO_EM_TEMPERATURE_ERROR:           return "Temperature Motor is too high";
		case CO_EM_TEMPERATURE_INTRNL_ERROR:    return "Temperature PCB is too high";
		case CO_EM_HARDWARE_ERROR:              return "Hardware error (driver error)";
		case CO_EM_MOTION_INVALID:              return "Invalid motion command received";

		case CO_EM_WRONG_ERROR_REPORT:          return "Wrong parameters to CO_EM_reportError() function";
		case CO_EM_ISR_TIMER_OVERFLOW:          return "Timer task has overflowed";
		case CO_EM_MEMORY_ALLOCATION_ERROR:     return "Unable to allocate memory for objects";
		case CO_EM_GENERIC_ERROR:               return "Generic error, test usage";
		case CO_EM_GENERIC_SOFTWARE_ERROR:      return "Software error";
		case CO_EM_INCONSISTENT_OBJECT_DICT:    return "Object dictionary does not match the software";
		case CO_EM_CALCULATION_OF_PARAMETERS:   return "Error in calculation of device parameters";
		case CO_EM_NON_VOLATILE_MEMORY:         return "Error with access to non volatile device memory";

		case CO_EM_FLT_CONFIG_CONSTRAINT:       return "Constraint was applied to the settings";
		case CO_EM_FLT_CONFIG_CRC:              return "CRC check of the setings failed";
		case CO_EM_FLT_NTC:                     return "NTC Error";
		case CO_EM_FLT_CS0:                     return "Current sensor 0 error";
		case CO_EM_FLT_CS1:                     return "Current sensor 1 error";
		case CO_EM_FLT_CS2:                     return "Current sensor 2 error";
		case CO_EM_FLT_DRIVER:                  return "Driver error";
		case CO_EM_FLT_VS0:                     return "Voltage sensor error";

		case CO_EM_FLT_ENC_M_OFF:               return "Motor Encoder disconnected";
		case CO_EM_FLT_ENC_G_OFF:               return "Gear Encoder disconnected";
		case CO_EM_FLT_ENC_M_STUP_CRC:          return "Motor Encoder CRC_ERR/EPR_ERR in STATUS1 & STUP in STATUS0";
		case CO_EM_FLT_ENC_G_STUP_CRC:          return "Gear Encoder CRC_ERR/EPR_ERR in STATUS1 & STUP in STATUS0";
		case CO_EM_FLT_ENC_M_LEVEL:             return "Motor Encoder FRQ_ABZ/FRQ_CNV in STATUS1 & AN_MAX/AN_MIN/AM_MAX/AM_MIN in STATUS0";
		case CO_EM_FLT_ENC_G_LEVEL:             return "Gear Encoder FRQ_ABZ/FRQ_CNV in STATUS1 & AN_MAX/AN_MIN/AM_MAX/AM_MIN in STATUS0";
		case CO_EM_FLT_ENC_M_SIG:               return "Motor Encoder NON_CTR bit in STATUS1";
		case CO_EM_FLT_ENC_G_SIG:               return "Gear Encoder NON_CTR bit in STATUS1";

		case CO_EM_HW_VOLT_LO:                  return "Under Voltage";
		case CO_EM_HW_VOLT_HI:                  return "Over Voltage";
		case CO_EM_HW_CUR_LIMIT:                return "Over Current";
		case CO_EM_POWER_ERROR:                 return "Over Power";
		case CO_EM_FORCE_ERROR:                 return "Over Force";
		case CO_EM_HEARTBEAT_CONSUMER:          return "Heartbeat consumer timeout";

		default:                                return "N/A";
	}
}

/**
 * @brief Returns descriptive string for specified EMCY codes
 * 
 * @param code EMCY error code according to CanOpen standard
 * @return void
 * @ingroup Utils
 */
const char *rr_describe_emcy_code(uint16_t code)
{
	switch(code)
	{
		case CO_EMC_NO_ERROR:                   return "Error Reset or No Error";
		case CO_EMC_GENERIC:                    return "Generic Error";
		case CO_EMC_CURRENT:                    return "Current";
		case CO_EMC_CURRENT_INPUT:              return "Current, device input side";
		case CO_EMC_CURRENT_INSIDE:             return "Current inside the device";
		case CO_EMC_CURRENT_OUTPUT:             return "Current, device output side";
		case CO_EMC_VOLTAGE:                    return "Voltage";
		case CO_EMC_VOLTAGE_MAINS:              return "Mains Voltage";
		case CO_EMC_VOLTAGE_INSIDE:             return "Voltage inside the device";
		case CO_EMC_VOLTAGE_OUTPUT:             return "Output Voltage";
		case CO_EMC_TEMPERATURE:                return "Temperature";
		case CO_EMC_TEMP_AMBIENT:               return "Ambient Temperature";
		case CO_EMC_TEMP_DEVICE:                return "Device Temperature";
		case CO_EMC_HARDWARE:                   return "Device Hardware";
		case CO_EMC_SOFTWARE_DEVICE:            return "Device Software";
		case CO_EMC_SOFTWARE_INTERNAL:          return "Internal Software";
		case CO_EMC_SOFTWARE_USER:              return "User Software";
		case CO_EMC_DATA_SET:                   return "Data Set";
		case CO_EMC_ADDITIONAL_MODUL:           return "Additional Modules";
		case CO_EMC_MONITORING:                 return "Monitoring";
		case CO_EMC_COMMUNICATION:              return "Communication";
		case CO_EMC_CAN_OVERRUN:                return "CAN Overrun (Objects lost)";
		case CO_EMC_CAN_PASSIVE:                return "CAN Passive Mode";
		case CO_EMC_HEARTBEAT:                  return "Life Guard Error or Heartbeat Error";
		case CO_EMC_BUS_OFF_RECOVERED:          return "recovered from bus off";
		case CO_EMC_CAN_ID_COLLISION:           return "CAN-ID collision";
		case CO_EMC_PROTOCOL_ERROR:             return "Protocol Error";
		case CO_EMC_PDO_LENGTH:                 return "PDO not processed due to length error";
		case CO_EMC_PDO_LENGTH_EXC:             return "PDO length exceeded";
		case CO_EMC_DAM_MPDO:                   return "DAM MPDO not processed, destination object not available";
		case CO_EMC_SYNC_DATA_LENGTH:           return "Unexpected SYNC data length";
		case CO_EMC_RPDO_TIMEOUT:               return "RPDO timeout";
		case CO_EMC_EXTERNAL_ERROR:             return "External Error";
		case CO_EMC_ADDITIONAL_FUNC:            return "Additional Functions";
		case CO_EMC_DEVICE_SPECIFIC:            return "Device specific";

		case CO_EMC401_OUT_CUR_HI:              return "DS401: Current at outputs too high (overload)";
		case CO_EMC401_OUT_SHORTED:             return "DS401: Short circuit at outputs";
		case CO_EMC401_OUT_LOAD_DUMP:           return "DS401: Load dump at outputs";
		case CO_EMC401_IN_VOLT_HI:              return "DS401: Input voltage too high";
		case CO_EMC401_IN_VOLT_LOW:             return "DS401: Input voltage too low";
		case CO_EMC401_INTERN_VOLT_HI:          return "DS401: Internal voltage too high";
		case CO_EMC401_INTERN_VOLT_LO:          return "DS401: Internal voltage too low";
		case CO_EMC401_OUT_VOLT_HIGH:           return "DS401: Output voltage too high";
		case CO_EMC401_OUT_VOLT_LOW:            return "DS401: Output voltage too low";
		case CO_EMC401_POWER_TEMP_OVER:         return "High temperature of the PCB";
		case CO_EMC401_MOTOR_TEMP_OVER:         return "High temperature of the motor";
		case CO_EMC401_SYS_ERROR:               return "System error";
		case CO_EMC401_POINT_ERROR:             return "System error: invalid motion point";
		case CO_EMC401_CURR_MEAS_OFFSET:        return "Control: Current measurement offset";
		case CO_EMC401_EE_FAULT:                return "EEPROM fault";
		case CO_EMC401_EE_CRC_ERROR:            return "EEPROM checksum error";
		case CO_EMC401_CONF_ERROR:              return "Configuration error";
		case CO_EMC401_ENC_CNT_ERROR:           return "Encoder counting error";
		case CO_EMC401_VEL_FLW_ERROR:           return "Velocity controller following error";
		case CO_EMC401_POS_LIMIT:               return "Position controller limits";
		case CO_EMC401_POS_FLW_ERROR:           return "Position controller following error";
		case CO_EMC401_POS_FLW_STATIC_ERROR:    return "Position controller static following error";
		case CO_EMC401_ACCESS_ERROR:            return "Unauthorized access";
		case CO_EMC401_PWRCTRL_ERROR:           return "Power Stage Controller Error";
		case CO_EMC401_BUSY:                    return "Busy";
		case CO_EMC401_PROCEDURE_ERROR:         return "Procedure error";
		case CO_EMC401_FORCE_OVER:              return "Over force";
		case CO_EMC401_POWER_OVER:              return "Over power";
		default:                                return "N/A";
	}
}

/**
 * @brief 
 * 
 * @param interface_name 
 * @return int Status code (::rr_can_interface_t)
 * @ingroup Common
 */
rr_can_interface_t *rr_init_interface(const char *interface_name)
{
	rr_can_interface_t *i = calloc(1, sizeof(rr_can_interface_t));

	if(!i)
	{
		return NULL;
	}

	usbcan_instance_t *usbcan = usbcan_instance_init(interface_name);
	usbcan->udata = i;
	i->iface = usbcan;


	usbcan_setup_nmt_state_cb(usbcan, rr_nmt_state_master_cb);
	usbcan_setup_emcy_cb(usbcan, rr_emcy_master_cb);

	if(!i->iface)
	{
		free(i);
		return NULL;
	}

	//rr_set_comm_log_stream(i, stderr);
	rr_set_debug_log_stream(stderr);
	return i;
}

/**
 * @brief 
 * 
 * @param interface 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Common
 */
int rr_deinit_interface(rr_can_interface_t **interface)
{
	IS_VALID_INTERFACE(*interface);

	if(usbcan_instance_deinit((usbcan_instance_t **)&((*interface)->iface)))
	{
		free(*interface);
		*interface = NULL;
		return RET_OK;
	}
	return RET_ERROR;
}

/**
 * @brief 
 * 
 * @param interface 
 * @param id 
 * @return int Status code (::rr_servo_t)
 * @ingroup Common
 */
rr_servo_t *rr_init_servo(rr_can_interface_t *interface, const uint8_t id)
{
	rr_servo_t *s = calloc(1, sizeof(rr_servo_t));

	if(!s)
	{
		return NULL;
	}

	s->dev = usbcan_device_init((usbcan_instance_t *)interface->iface, id);

	if(!s->dev)
	{
		free(s);
		return NULL;
	}

	wait_device((usbcan_instance_t *)interface->iface, id, RR_API_WAIT_DEVICE_TIMEOUT_MS);

	return s;
}

/**
 * @brief 
 * 
 * @param servo Device instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Common
 */
int rr_deinit_servo(rr_servo_t **servo)
{
	IS_VALID_SERVO(*servo);

	if(usbcan_device_deinit((usbcan_device_t **)&((*servo)->dev)))
	{
		free(*servo);
		*servo = NULL;
		return RET_OK;
	}
	return RET_ERROR;
    //return usbcan_servo_deinit((rr_servo_t **)servo) ? RET_OK : RET_ERROR;
}

/**
 * @brief Reboots servo
 * 
 * @param servo Device instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_servo_reboot(const rr_servo_t *servo)
{
	IS_VALID_SERVO(servo);
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_RESET_NODE) ? RET_OK : RET_ERROR;;
}

/**
 * @brief Resets servo communication
 * 
 * @param servo Device instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_servo_reset_communication(const rr_servo_t *servo)
{
	IS_VALID_SERVO(servo);
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_RESET_COMM) ? RET_OK : RET_ERROR;;
}

/**
 * @brief Puts servo to the operational state  
 * 
 * @param servo Device instance 
 *  If servo == 0 > all servos on the bus will be set to the operational state
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_servo_set_state_operational(const rr_servo_t *servo)
{
	IS_VALID_SERVO(servo);
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_GOTO_OP) ? RET_OK : RET_ERROR;;
}

/**
 * @brief Puts servo to the pre-operational state  
 * 
 * @param servo Device instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_servo_set_state_pre_operational(const rr_servo_t *servo)
{
	IS_VALID_SERVO(servo);
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_GOTO_PREOP) ? RET_OK : RET_ERROR;;
}

/**
 * @brief Puts servo to the stopped state  
 * 
 * @param servo Device instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_servo_set_state_stopped(const rr_servo_t *servo)
{
	IS_VALID_SERVO(servo);
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_GOTO_STOPPED) ? RET_OK : RET_ERROR;;
}

/**
 * @brief Reboots entire network
 * 
 * @param interface Interface instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_net_reboot(const rr_can_interface_t *interface)
{
	IS_VALID_INTERFACE(interface);
	usbcan_instance_t *inst = (usbcan_instance_t *)interface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_RESET_NODE) ? RET_OK : RET_ERROR;;
}

/**
 * @brief Resets entire network communication
 * 
 * @param interface Interface instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_net_reset_communication(const rr_can_interface_t *interface)
{
	IS_VALID_INTERFACE(interface);
	usbcan_instance_t *inst = (usbcan_instance_t *)interface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_RESET_COMM) ? RET_OK : RET_ERROR;;
}

/**
 * @brief Putss entire network to the operational state  
 * 
 * @param interface Interface instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_net_set_state_operational(const rr_can_interface_t *interface)
{
	IS_VALID_INTERFACE(interface);
	usbcan_instance_t *inst = (usbcan_instance_t *)interface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_GOTO_OP) ? RET_OK : RET_ERROR;;
}

/**
 * @brief Putss entire network to the pre-operational state  
 * 
 * @param interface Interface instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_net_set_state_pre_operational(const rr_can_interface_t *interface)
{
	IS_VALID_INTERFACE(interface);
	usbcan_instance_t *inst = (usbcan_instance_t *)interface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_GOTO_PREOP) ? RET_OK : RET_ERROR;;
}

/**
 * @brief Puts entire network to the stopped state  
 * 
 * @param interface Interface instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup System_control
 */
int rr_net_set_state_stopped(const rr_can_interface_t *interface)
{
	IS_VALID_INTERFACE(interface);
	usbcan_instance_t *inst = (usbcan_instance_t *)interface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_GOTO_STOPPED) ? RET_OK : RET_ERROR;
}

/**
 * @brief Stops the servo and releases it
 * 
 * @param servo Device instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_stop_and_release(const rr_servo_t *servo)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data = 0;
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = write_raw_sdo(dev, 0x2010, 0x01, &data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief Stops the servo and make it hold current position
 * 
 * @param servo Device instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_stop_and_freeze(const rr_servo_t *servo)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data = 0;
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = write_raw_sdo(dev, 0x2010, 0x02, &data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief Sets servo contol current
 * 
 * @param servo Device instance 
 * @param current_a Phase current in Amperes
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_set_current(const rr_servo_t *servo, const float current_a)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &current_a, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2012, 0x01, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief Sets servo contol velocity
 * 
 * @param servo Device instance 
 * @param velocity_deg_per_sec Flange velocity in degrees/sec
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_set_velocity(const rr_servo_t *servo, const float velocity_deg_per_sec)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &velocity_deg_per_sec, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2012, 0x03, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief Sets servo contol position with maximum current
 * 
 * @param servo Device instance 
 * @param position_deg Flange position in degrees
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_set_position(const rr_servo_t *servo, const float position_deg)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &position_deg, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2012, 0x04, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief Sets servo contol velocity with limited control phase current
 * 
 * @param servo Device instance 
 * @param velocity_deg_per_sec Flange velocity in degrees/sec
 * @param current_a Phase current limit in Amperes
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_set_velocity_with_limits(const rr_servo_t *servo, const float velocity_deg_per_sec, const float current_a)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[8];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &velocity_deg_per_sec, 1);
    usb_can_put_float(data, 0, &current_a, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2012, 0x05, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief Sets servo contol position with 
 * limited control flange velocity and
 * limited control phase current
 * 
 * @param servo Device instance 
 * @param position_deg Flange position in degrees
 * @param velocity_deg_per_sec Flange velocity in degrees/sec
 * @param current_a Phase current limit in Amperes
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_set_position_with_limits(const rr_servo_t *servo, const float position_deg, const float velocity_deg_per_sec, const float current_a)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[12];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &position_deg, 1);
    usb_can_put_float(data, 0, &velocity_deg_per_sec, 1);
    usb_can_put_float(data, 0, &current_a, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2012, 0x06, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief Sets servo control duty.
 * Duty is the part of the input voltage that is passed to the motor to spin it
 * 
 * @param servo Device instance 
 * @param duty_percent 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_set_duty(const rr_servo_t *servo, float duty_percent)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &duty_percent, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2012, 0x07, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief Adds motion PVT point to the servo queue
 * 
 * @param servo Device instance 
 * @param position_deg Flange position in degrees
 * @param velocity_deg Flange velocity in degrees/sec
 * @param time_ms Relative point time in milliseconds
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_add_motion_point(const rr_servo_t *servo, const float position_deg, const float velocity_deg, const uint32_t time_ms)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[12];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &position_deg, 1);
    usb_can_put_float(data + 4, 0, &velocity_deg, 1);
    usb_can_put_uint32_t(data + 8, 0, &time_ms, 1);

    uint32_t sts = write_raw_sdo(dev, 0x2200, 2, data, sizeof(data), 1, 200);
    if(sts == CO_SDO_AB_PRAM_INCOMPAT)
    {
        return RET_WRONG_TRAJ;
    }
    else
    {
        return ret_sdo(sts);
    }
}

/**
 * @brief Starts the servo movement by the spline points.
 * This is a broadcast command.
 * Note: if any servo is not completed the movement (by spline points) it will send 
 * broadcast "Goto Stopped State" command to the all servos on the bus
 * 
 * @param interface interface to start motion on. 
 * @param timestamp_ms Startup delay in milliseconds. 
 * Default: 0
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_start_motion(rr_can_interface_t *interface, uint32_t timestamp_ms)
{
	IS_VALID_INTERFACE(interface);

	usbcan_instance_t *inst = (usbcan_instance_t *)interface->iface;
    write_timestamp(inst, timestamp_ms);
    return RET_OK;
}
 
/**
 * @brief Reads error flag array
 * 
 * @param servo Device instance 
 * @param error_count Pointer to the error count variable
 * @param error_array Pointer to the error array. Array contains the codes of the errors that can be described with ::rr_describe_emcy_bit function. ::error_count is a size of the array.
 * If it is not used - pass 0 argument
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_read_error_status(const rr_servo_t *servo, uint32_t * const error_count, uint8_t * const error_array)
{
	IS_VALID_SERVO(servo);

    CHECK_NMT_STATE(servo);

    uint8_t array[32];
    uint8_t *ptr = error_array == 0 ? array : error_array;
    *error_count = 0;

    int size = sizeof(array); //? think about
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = read_raw_sdo(dev, 0x2000, 0, ptr, &size, 1, 200);

    if(sts == CO_SDO_AB_NONE)
    {
        for(int i = 0; i < size*8; i++)
        {
            if(ptr[i / 8] & (1 << (i % 8)))
            {            
                if(error_array)
                {
                    error_array[*error_count] = i;
                }
                *error_count += 1;
            }
        }
    }

    return ret_sdo(sts);
}

/**
 * @brief Updates enabled cache entries
 * 
 * @param servo Device instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_param_cache_update(rr_servo_t *servo)
{
	IS_VALID_SERVO(servo);
	CHECK_NMT_STATE(servo);

	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint8_t data[APP_PARAM_SIZE * sizeof(float)];
	int len = sizeof(data);

    int sts = read_raw_sdo(dev, 0x2014, 0x01, data, &len, 1, 100);
	int i, src;

    if(sts == CO_SDO_AB_NONE)
    {
        for(i = 0, src = 0; i < APP_PARAM_SIZE; i++)
        {
            if(servo->pcache[i].activated)
            {
                usb_can_get_float(data + src, 0, (float *)&servo->pcache[i].value, 1);
				src += sizeof(float);
            }
        }
		if(src != len)
		{
			return RET_SIZE_MISMATCH;
		}
        return RET_OK;
    }

    return ret_sdo(sts);
}

/**
 * @brief Enables or disables reading specified parameter during cache update
 * 
 * @param servo Device instance 
 * @param param Parameter ID
 * @param enabled Enable/disable reading of this parameter
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_param_cache_setup_entry(rr_servo_t *servo, const rr_servo_param_t param, bool enabled)
{
	IS_VALID_SERVO(servo);
	CHECK_NMT_STATE(servo);

    uint8_t array[10] = {0};
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;

	servo->pcache[param].activated = enabled;

    for(int i = 0; i < APP_PARAM_SIZE; i++)
    {
		if(servo->pcache[i].activated)
		{
        	BIT_SET_UINT_ARRAY(array, i);
		}
    }

    uint32_t sts = write_raw_sdo(dev, 0x2015, 1, array, sizeof(array), 1, 200);

	return ret_sdo(sts);
}

/**
 * @brief Reads single information parameter. Cache also update during this operation irrespective of enabled status.
 * 
 * @param servo Device instance 
 * @param param Parameter index to read (::rr_servo_param_t)
 * @param value Pointer to the readed variable
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_read_parameter( rr_servo_t *servo, const rr_servo_param_t param, float *value)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    int size = sizeof(data);

    uint32_t sts = read_raw_sdo(dev, 0x2013, param, data, &size, 2, 100);
    if(sts == CO_SDO_AB_NONE && size == 4)
    {        
        usb_can_get_float(data, 0, (float *)&servo->pcache[param].value, 1);
        *value = servo->pcache[param].value;
        return RET_OK;
    }

    return ret_sdo(sts);
}

/**
 * @brief Reads single information parameter from cache.
 * 
 * @param servo Device instance 
 * @param param Parameter index to read (::rr_servo_param_t)
 * @param value Pointer to the readed variable
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_read_cached_parameter( rr_servo_t *servo, const rr_servo_param_t param, float *value)
{
	IS_VALID_SERVO(servo);
	*value = servo->pcache[param].value;
    return RET_OK;
}

/**
 * @brief Erases the whole servo motion queue
 * 
 * @param servo Device instance 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_clear_points_all(const rr_servo_t *servo)
{
    return rr_clear_points(servo, 0);
}

/**
 * @brief Erases number of cells from the tail of the servo motion queue
 * 
 * @param servo Device instance 
 * @param num_to_clear 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_control
 */
int rr_clear_points(const rr_servo_t *servo, const uint32_t num_to_clear)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = write_raw_sdo(dev, 0x2202, 0x01, (uint8_t *)&num_to_clear, sizeof(num_to_clear), 1, 100); 
    return ret_sdo(sts);
}

/**
 * @brief Gets servo motion queue occupied cells (queue size)
 * 
 * @param servo Device instance 
 * @param num 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_get_points_size(const rr_servo_t *servo, uint32_t *num)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    int len = sizeof(data);
    uint32_t sts = read_raw_sdo(dev, 0x2202, 0x02, data, &len, 1, 100);

    if(sts == CO_SDO_AB_NONE && len == 4)
    {
        usb_can_get_uint32_t(data, 0, num, 1);
        return RET_OK;
    }

    return ret_sdo(sts);
}

/**
 * @brief Gets servo remaining motion queue free cell count
 * 
 * @param servo Device instance 
 * @param num 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_get_points_free_space(const rr_servo_t *servo, uint32_t *num)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
    int len = sizeof(data);
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = read_raw_sdo(dev, 0x2202, 0x03, data, &len, 1, 100);

    if(sts == CO_SDO_AB_NONE && len == 4)
    {
        usb_can_get_uint32_t(data, 0, num, 1);
        return RET_OK;
    }

    return ret_sdo(sts);
}

/**
 * @brief Calculates motion point parameters
 * 
 * @param servo Device instance 
 * @param start_position_deg 
 * @param start_velocity_deg 
 * @param start_acceleration_deg_per_sec2 
 * @param start_time_ms 
 * @param end_position_deg 
 * @param end_velocity_deg 
 * @param end_acceleration_deg_per_sec2 
 * @param end_time_ms 
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_invoke_time_calculation(const rr_servo_t *servo,
                              const float start_position_deg, const float start_velocity_deg, const float start_acceleration_deg_per_sec2, const uint32_t start_time_ms,
                              const float end_position_deg, const float end_velocity_deg, const float end_acceleration_deg_per_sec2, const uint32_t end_time_ms)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[8 * 4];
    int p = 0;
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;

    p = usb_can_put_float(data, p, &start_position_deg, 1);
    p = usb_can_put_float(data, p, &start_velocity_deg, 1);
    p = usb_can_put_float(data, p, &start_acceleration_deg_per_sec2, 1);
    p = usb_can_put_uint32_t(data, p, &start_time_ms, 1);

    p = usb_can_put_float(data, p, &end_position_deg, 1);
    p = usb_can_put_float(data, p, &end_velocity_deg, 1);
    p = usb_can_put_float(data, p, &end_acceleration_deg_per_sec2, 1);
    p = usb_can_put_uint32_t(data, p, &end_time_ms, 1);

    uint32_t sts = write_raw_sdo(dev, 0x2203, 0x01, data, sizeof(data), 1, 200);

    if(sts == CO_SDO_AB_GENERAL)
    {
        return RET_WRONG_TRAJ;
    }
    else
    {
        return ret_sdo(sts);
    }
}

/**
 * @brief Gets ::rr_invoke_time_calculation calculated time in milliseconds
 * 
 * @param servo Device instance 
 * @param time_ms Pointer to the calculated time in milliseconds
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_get_time_calculation_result(const rr_servo_t *servo, uint32_t *time_ms)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    int len = sizeof(data);
    uint32_t sts = read_raw_sdo(dev, 0x2203, 0x02, data, &len, 1, 100);
    uint32_t num;

    if(sts == CO_SDO_AB_NONE && len == 4)
    {
        usb_can_get_uint32_t(data, 0, &num, 1);
        *time_ms = num;
        return RET_OK;
    }

    return ret_sdo(sts);
}

/**
 * @brief Sets servo zero position
 * 
 * @param servo Device instance 
 * @param position_deg Target position in degrees
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_config
 */
int rr_set_zero_position(const rr_servo_t *servo, const float position_deg)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &position_deg, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2208, 0x01, data, sizeof(data), 0, 200);

    return ret_sdo(sts);
}

/**
 * @brief Sets servo zero position and saves it to the servo FLASH memory
 * 
 * @param servo Device instance 
 * @param position_deg Target position in degrees
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_config
 */
int rr_set_zero_position_and_save(const rr_servo_t *servo, const float position_deg)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &position_deg, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2208, 0x02, data, sizeof(data), 0, 200);

    return ret_sdo(sts);
}

/**
 * @brief Gets the maximum servo velocity
 * 
 * @param servo Device instance 
 * @param velocity_deg_per_sec Velocity in degrees/sec
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_get_max_velocity(const rr_servo_t *servo, float *velocity_deg_per_sec)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
    int len = sizeof(data);

	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = read_raw_sdo(dev, 0x2207, 0x02, data, &len, 1, 100);
    if(sts == CO_SDO_AB_NONE)
    {
        usb_can_get_float(data, 0, velocity_deg_per_sec, 1);
    }

    return ret_sdo(sts);
}

/**
 * @brief Sets the global limit for the servo velocity.
 * Note: this is power volatile command
 * 
 * @param servo Device instance 
 * @param max_velocity_deg_per_sec Flange velocity in degrees/sec
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_config
 */
int rr_set_max_velocity(const rr_servo_t *servo, const float max_velocity_deg_per_sec)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &max_velocity_deg_per_sec, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2300, 0x03, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}
