/**
 * @brief Rozum Robotics API Source File
 * 
 * @file api.c
 * @author Rozum
 * @date 2018-06-01
 */

/*! \mainpage Rozum Robotics User API & Servo Box
 * \section sbox Servo box
 * - \subpage servo_box
 * 
 * \section intro_section API Categories
 * - \ref Aux
 * - \ref Init
 * - \ref State
 * - \ref Motion
 * - \ref Trajectory
 * - \ref Config
 * - \ref Realtime
 * - \ref Err
 * - \ref Dbg
 * 
 * \section tutor API Tutorials
 * - C
 *    -# \ref tutor_c_servomove1
 *    -# \ref tutor_c_servomove2
 *    -# \ref tutor_c_servomove3
 *    -# \ref tutor_c_param
 *    -# \ref tutor_c_param_cache
 *    -# \ref tutor_c_error_read
 *    -# \ref tutor_c_calculate_point
 *    -# \ref tutor_c_read_motion_queue
 *    -# \ref tutor_c_get_max_velocity
 *    -# \ref tutor_c_changeID1
 * - Java
 * - Python
 * - Ruby
 * 
 * 
 * \defgroup Init Initialization and deinitialization
 * \defgroup State  Switching servo working states
 * \defgroup Motion Simple motion control (duty, current, velocity, position)
 * \defgroup Trajectory Trajectory motion control (PVT)
 * \defgroup Config Reading and writing servo configuration
 * \defgroup Realtime Reading realtime parameters
 * \defgroup Err Error handling
 * \defgroup Dbg Debugging
 * \defgroup Aux Auxiliary functions
 * 
 * \defgroup hw_manual Servo box specs & manual
 * 
 * \defgroup tutor_c_servomove1 PVT trajectory for one servo
 * \defgroup tutor_c_servomove2 PVT trajectory for two servos
 * \defgroup tutor_c_servomove3 PVT trajectory for three servos
 * \defgroup tutor_c_param_cache Setting up parameter cache and reading cached parameters
 * \defgroup tutor_c_param Reading device parameters
 * \defgroup tutor_c_calculate_point PVT point calculation
 * \defgroup tutor_c_get_max_velocity Reading maximum servo velocity
 * \defgroup tutor_c_read_motion_queue Reading motion queue parameters
 * \defgroup tutor_c_changeID1 Changing CAN ID of a single servo
 */
/* Includes ------------------------------------------------------------------*/
#include "api.h"
#include "logging.h"
#include "usbcan_proto.h"
#include "usbcan_types.h"
#include "usbcan_util.h"
#include <stdio.h>

//! @cond Doxygen_Suppress
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RR_API_WAIT_DEVICE_TIMEOUT_MS 2000

/* Private macro -------------------------------------------------------------*/
#define BIT_SET_UINT_ARRAY(array, bit) ((array)[(bit) / 8] |= (1 << ((bit) % 8)))

#define IS_VALID_INTERFACE(v) \
    if(!v) return RET_BAD_INSTANCE
#define IS_VALID_SERVO(v) \
    if(!v) return RET_BAD_INSTANCE

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
static rr_ret_status_t ret_sdo(int code)
{
    switch(code)
    {
    case CO_SDO_AB_NONE:
        return RET_OK;
    case CO_SDO_AB_TIMEOUT:
        return RET_TIMEOUT;
    default:
        return RET_ERROR;
    }
}

void rr_nmt_state_master_cb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
    rr_can_interface_t *i = (rr_can_interface_t *)inst->udata;

    LOG_INFO(debug_log, "ID: %d %s", id, rr_describe_nmt((rr_nmt_state_t)state));

    if(i->nmt_cb)
    {
        ((rr_nmt_cb_t)(i->nmt_cb))(i, id, (rr_nmt_state_t)state);
    }
}

void rr_emcy_master_cb(usbcan_instance_t *inst, int id, uint16_t code, uint8_t reg, uint8_t bits, uint32_t info)
{
    rr_can_interface_t *i = (rr_can_interface_t *)inst->udata;

    LOG_WARN(debug_log, "Emergency frame received: id(%" PRId8 ") code(0x%" PRIX16 ") reg(0x%" PRIX8 ") bits(0x%" PRIX8 ") info(0x%" PRIX32 "):\n    '%s, %s'",
             id, code, reg, bits, info, rr_describe_emcy_code(code), rr_describe_emcy_bit(bits));

    if(i->emcy_cb)
    {
        ((rr_emcy_cb_t)(i->emcy_cb))(i, id, code, reg, bits, info);
    }
}

/// @endcond

/**
 * @brief The function sets an idle period for the user program (e.g., to wait till a servo executes a motion trajectory).
 * Until the period expires, the user program will not execute any further operations.
 * However, the network management, CAN communication, emergency, and Heartbeat functions remain available.
 * <p><b>Note:</b>The user can also call system-specific sleep functions directly. However, using this sleep function is preferable to ensure
 * compatibility with subsequent API library versions.<p> 
 * @param ms Idle period (in milleseconds)
 * @return void
 * @ingroup Aux
 */
void rr_sleep_ms(int ms)
{
    msleep(ms);
}

//! @cond Doxygen_Suppress
/**
 * @brief The function sends an arbitrary SDO write request to the specified servo.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param idx Index of the SDO object to which the request refers
 * @param sidx Subindex
 * @param data Data to write to
 * @param sz Size of the `data` in bytes
 * @param retry Number of retries (if a communication error occured during the request)
 * @param tout Request timeout in milliseconds
 * @return Status code (::rr_ret_status_t)
 * @ingroup Aux
 */
rr_ret_status_t rr_write_raw_sdo(const rr_servo_t *servo, uint16_t idx, uint8_t sidx, uint8_t *data, int sz, int retry, int tout)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint32_t sts = write_raw_sdo((usbcan_device_t *)servo->dev, idx, sidx, data, sz, retry, tout);

    return ret_sdo(sts);
}

/**
 * @brief The function sends an arbitrary SDO read request to the specified servo.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param idx Index of the SDO object to which the request refers
 * @param sidx Subindex
 * @param data Data to read to
 * @param sz Size of the `data` in bytes, is writed with the number of readed bytes
 * @param retry Number of retries (if a communication error occured during the request)
 * @param tout Request timeout in milliseconds
 * @return Status code (::rr_ret_status_t)
 * @ingroup Aux
 */
rr_ret_status_t rr_read_raw_sdo(const rr_servo_t *servo, uint16_t idx, uint8_t sidx, uint8_t *data, int *sz, int retry, int tout)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint32_t sts = read_raw_sdo((usbcan_device_t *)servo->dev, idx, sidx, data, sz, retry, tout);

    return ret_sdo(sts);
}
/// @endcond

/**
 * @brief The function sets a stream for saving CAN communication dump from the specified interface.
 * Subsequently, the user can look through the logs saved to the stream to identify causes of CAN communication failures.
 * @param iface Descriptor of the interface where the logged CAN communication occurs (returned by the ::rr_init_interface function) 
 * @param f stdio stream for saving the communication log. When the parameter is set to "NULL," logging of CAN communication events in the interface is disabled.
 * @return void
 * @ingroup Dbg
 */
void rr_set_comm_log_stream(const rr_can_interface_t *iface, FILE *f)
{
    usbcan_instance_t *inst = (usbcan_instance_t *)iface->iface;
    usbcan_set_comm_log_stream(inst, f);
}

/**
 * @brief The function sets a stream for saving the debugging messages generated by the API library.
 * Subsequently, the user can look through the logs to identify and locate the events associated with certain problems.
 * @param f stdio stream for saving the debugging log. When the parameter is set to "NULL," logging of debugging events is disabled.
 * @return void
 * @ingroup Dbg
 */
void rr_set_debug_log_stream(FILE *f)
{
    usbcan_set_debug_log_stream(f);
}

/**
 * @brief The function sets a user callback to be intiated in connection with  with changes of network management (NMT) states
 * (e.g., a servo connected to/ disconnected from the CAN bus, the interface/ a servo going to the operational state, etc.).
 * User callbacks are functions to execute specific user-defined operations, e.g., to  display a warning about an NMT state change or stop the program.
 * @param iface Descriptor of the interface (as returned by the ::rr_init_interface function)
 * @param cb (::rr_nmt_cb_t) Type of the callback to be initiated when an NMT event occurs. When the parameter is set to "NULL," the function is disabled.
 * @return void
 * @ingroup State
 */
void rr_setup_nmt_callback(rr_can_interface_t *iface, rr_nmt_cb_t cb)
{
    if(iface)
    {
        iface->nmt_cb = (void*)cb;
    }
}

/**
 * @brief The function sets a user callback to be intiated in connection with emergency (EMCY) events (e.g., overcurrent, power outage, etc.).
 * User callbacks are functions to execute specific user-defined operations, e.g., to  display a warning about an EMCY event or stop the program.
 * @param iface Descriptor of the interface (as returned by the ::rr_init_interface function)
 * @param cb (::rr_emcy_cb_t) Type of the callback to be initiated when an NMT event occurs. When the parameter is set to "NULL," the function is disabled.
 * @return void
 * @ingroup Err
 */
void rr_setup_emcy_callback(rr_can_interface_t *iface, rr_emcy_cb_t cb)
{
    if(iface)
    {
        iface->emcy_cb = (void*)cb;
    }
}

/**
 * @brief The function returns a string describing the NMT state code specified in the 'state' parameter. 
 * You can also use the function with ::rr_setup_nmt_callback, setting the callback to display a detailed message
 * describing an NMT event.
 * @param state NMT state code to descibe
 * @return Pointer to the description string
 * @ingroup State
 */
const char *rr_describe_nmt(rr_nmt_state_t state)
{
    switch(state)
    {
    case RR_NMT_INITIALIZING:
        return "Device is initializing";
    case RR_NMT_BOOT:
        return "Bootloader mode";
    case RR_NMT_PRE_OPERATIONAL:
        return "Device is in pre-operational mode";
    case RR_NMT_OPERATIONAL:
        return "Device is in operational mode";
    case RR_NMT_STOPPED:
        return "Device is in stopped mode";
    case RR_NMT_HB_TIMEOUT:
        return "Device disappeared";
    default:
        return "N/A";
    }
}

/**
 * @brief The function returns a string describing in detail a specific EMCY event based on the code in the 'bit' parameter (e.g., "CAN bus warning limit reached").
 * The function can be used in combination with ::rr_describe_emcy_code. The latter provides a more generic description of an EMCY event.
 * @param bit Error bit field of the corresponding EMCY message (according to the CanOpen standard)
 * @return Pointer to the description string
 * @ingroup Err
 */
const char *rr_describe_emcy_bit(uint8_t bit)
{
    switch(bit)
    {
    case CO_EM_NO_ERROR:
        return "Error Reset or No Error";
    case CO_EM_CAN_BUS_WARNING:
        return "CAN bus warning limit reached";
    case CO_EM_RXMSG_WRONG_LENGTH:
        return "Wrong data length of the received CAN message";
    case CO_EM_RXMSG_OVERFLOW:
        return "Previous received CAN message wasn't processed yet";
    case CO_EM_RPDO_WRONG_LENGTH:
        return "Wrong data length of received PDO";
    case CO_EM_RPDO_OVERFLOW:
        return "Previous received PDO wasn't processed yet";
    case CO_EM_CAN_RX_BUS_PASSIVE:
        return "CAN Rx passive";
    case CO_EM_CAN_TX_BUS_PASSIVE:
        return "CAN Tx passive";

    case CO_EM_NMT_WRONG_COMMAND:
        return "Wrong NMT command received";
        //case 09-11 unused

    case CO_EM_CAN_TX_BUS_OFF:
        return "CAN transmit bus is off";
    case CO_EM_CAN_RXB_OVERFLOW:
        return "CAN module receive buffer has overflowed";
    case CO_EM_CAN_TX_OVERFLOW:
        return "CAN transmit buffer has overflowed";
    case CO_EM_TPDO_OUTSIDE_WINDOW:
        return "TPDO is outside SYNC window";

        //case 16-17 unused

    case CO_EM_SYNC_TIME_OUT:
        return "SYNC message timeout";
    case CO_EM_SYNC_LENGTH:
        return "Unexpected SYNC data length";
    case CO_EM_PDO_WRONG_MAPPING:
        return "Error with PDO mapping";
    case CO_EM_HB_CONSUMER_REMOTE_RESET:
        return "Heartbeat consumer detected remote node reset";

        //case 1D-1F unused

    case CO_EM_EMERGENCY_BUFFER_FULL:
        return "Emergency buffer is full, Emergency message wasn't sent";
    case CO_EM_MOTION_ERROR:
        return "Motion Error";
    case CO_EM_MICROCONTROLLER_RESET:
        return "Microcontroller has just started";
    case CO_EM_UNAUTHORIZED_ACCESS:
        return "Access is only available to service engineer";
    case CO_EM_TEMPERATURE_ERROR:
        return "Temperature Motor is too high";
    case CO_EM_TEMPERATURE_INTRNL_ERROR:
        return "Temperature PCB is too high";
    case CO_EM_HARDWARE_ERROR:
        return "Hardware error (driver error)";
    case CO_EM_MOTION_INVALID:
        return "Invalid motion command received";

    case CO_EM_WRONG_ERROR_REPORT:
        return "Wrong parameters to CO_EM_reportError() function";
    case CO_EM_ISR_TIMER_OVERFLOW:
        return "Timer task has overflowed";
    case CO_EM_MEMORY_ALLOCATION_ERROR:
        return "Unable to allocate memory for objects";
    case CO_EM_GENERIC_ERROR:
        return "Generic error, test usage";
    case CO_EM_GENERIC_SOFTWARE_ERROR:
        return "Software error";
    case CO_EM_INCONSISTENT_OBJECT_DICT:
        return "Object dictionary does not match the software";
    case CO_EM_CALCULATION_OF_PARAMETERS:
        return "Error in calculation of device parameters";
    case CO_EM_NON_VOLATILE_MEMORY:
        return "Error with access to non volatile device memory";

    case CO_EM_FLT_CONFIG_CONSTRAINT:
        return "Constraint was applied to the settings";
    case CO_EM_FLT_CONFIG_CRC:
        return "CRC check of the setings failed";
    case CO_EM_FLT_NTC:
        return "NTC Error";
    case CO_EM_FLT_CS0:
        return "Current sensor 0 error";
    case CO_EM_FLT_CS1:
        return "Current sensor 1 error";
    case CO_EM_FLT_CS2:
        return "Current sensor 2 error";
    case CO_EM_FLT_DRIVER:
        return "Driver error";
    case CO_EM_FLT_VS0:
        return "Voltage sensor error";

    case CO_EM_FLT_ENC_M_OFF:
        return "Motor Encoder disconnected";
    case CO_EM_FLT_ENC_G_OFF:
        return "Gear Encoder disconnected";
    case CO_EM_FLT_ENC_M_STUP_CRC:
        return "Motor Encoder CRC_ERR/EPR_ERR in STATUS1 & STUP in STATUS0";
    case CO_EM_FLT_ENC_G_STUP_CRC:
        return "Gear Encoder CRC_ERR/EPR_ERR in STATUS1 & STUP in STATUS0";
    case CO_EM_FLT_ENC_M_LEVEL:
        return "Motor Encoder FRQ_ABZ/FRQ_CNV in STATUS1 & AN_MAX/AN_MIN/AM_MAX/AM_MIN in STATUS0";
    case CO_EM_FLT_ENC_G_LEVEL:
        return "Gear Encoder FRQ_ABZ/FRQ_CNV in STATUS1 & AN_MAX/AN_MIN/AM_MAX/AM_MIN in STATUS0";
    case CO_EM_FLT_ENC_M_SIG:
        return "Motor Encoder NON_CTR bit in STATUS1";
    case CO_EM_FLT_ENC_G_SIG:
        return "Gear Encoder NON_CTR bit in STATUS1";

    case CO_EM_HW_VOLT_LO:
        return "Under Voltage";
    case CO_EM_HW_VOLT_HI:
        return "Over Voltage";
    case CO_EM_HW_CUR_LIMIT:
        return "Over Current";
    case CO_EM_POWER_ERROR:
        return "Over Power";
    case CO_EM_FORCE_ERROR:
        return "Over Force";
    case CO_EM_HEARTBEAT_CONSUMER:
        return "Heartbeat consumer timeout";

    default:
        return "N/A";
    }
}

/**
 * @brief The function returns a string descibing a specific EMCY event based on the error code in the 'code' parameter. 
 * The description in the string is a generic type of the occured emergency event (e.g., "Temperature").
 * For a more detailed description, use the function together with the ::rr_describe_emcy_bit one.
 * @param code Error code from the corresponding EMCY message (according to the CanOpen standard)
 * @return Pointer to the description string
 * @ingroup Err
 */
const char *rr_describe_emcy_code(uint16_t code)
{
    switch(code)
    {
    case CO_EMC_NO_ERROR:
        return "Error Reset or No Error";
    case CO_EMC_GENERIC:
        return "Generic Error";
    case CO_EMC_CURRENT:
        return "Current";
    case CO_EMC_CURRENT_INPUT:
        return "Current, device input side";
    case CO_EMC_CURRENT_INSIDE:
        return "Current inside the device";
    case CO_EMC_CURRENT_OUTPUT:
        return "Current, device output side";
    case CO_EMC_VOLTAGE:
        return "Voltage";
    case CO_EMC_VOLTAGE_MAINS:
        return "Mains Voltage";
    case CO_EMC_VOLTAGE_INSIDE:
        return "Voltage inside the device";
    case CO_EMC_VOLTAGE_OUTPUT:
        return "Output Voltage";
    case CO_EMC_TEMPERATURE:
        return "Temperature";
    case CO_EMC_TEMP_AMBIENT:
        return "Ambient Temperature";
    case CO_EMC_TEMP_DEVICE:
        return "Device Temperature";
    case CO_EMC_HARDWARE:
        return "Device Hardware";
    case CO_EMC_SOFTWARE_DEVICE:
        return "Device Software";
    case CO_EMC_SOFTWARE_INTERNAL:
        return "Internal Software";
    case CO_EMC_SOFTWARE_USER:
        return "User Software";
    case CO_EMC_DATA_SET:
        return "Data Set";
    case CO_EMC_ADDITIONAL_MODUL:
        return "Additional Modules";
    case CO_EMC_MONITORING:
        return "Monitoring";
    case CO_EMC_COMMUNICATION:
        return "Communication";
    case CO_EMC_CAN_OVERRUN:
        return "CAN Overrun (Objects lost)";
    case CO_EMC_CAN_PASSIVE:
        return "CAN Passive Mode";
    case CO_EMC_HEARTBEAT:
        return "Life Guard Error or Heartbeat Error";
    case CO_EMC_BUS_OFF_RECOVERED:
        return "recovered from bus off";
    case CO_EMC_CAN_ID_COLLISION:
        return "CAN-ID collision";
    case CO_EMC_PROTOCOL_ERROR:
        return "Protocol Error";
    case CO_EMC_PDO_LENGTH:
        return "PDO not processed due to length error";
    case CO_EMC_PDO_LENGTH_EXC:
        return "PDO length exceeded";
    case CO_EMC_DAM_MPDO:
        return "DAM MPDO not processed, destination object not available";
    case CO_EMC_SYNC_DATA_LENGTH:
        return "Unexpected SYNC data length";
    case CO_EMC_RPDO_TIMEOUT:
        return "RPDO timeout";
    case CO_EMC_EXTERNAL_ERROR:
        return "External Error";
    case CO_EMC_ADDITIONAL_FUNC:
        return "Additional Functions";
    case CO_EMC_DEVICE_SPECIFIC:
        return "Device specific";

    case CO_EMC401_OUT_CUR_HI:
        return "DS401: Current at outputs too high (overload)";
    case CO_EMC401_OUT_SHORTED:
        return "DS401: Short circuit at outputs";
    case CO_EMC401_OUT_LOAD_DUMP:
        return "DS401: Load dump at outputs";
    case CO_EMC401_IN_VOLT_HI:
        return "DS401: Input voltage too high";
    case CO_EMC401_IN_VOLT_LOW:
        return "DS401: Input voltage too low";
    case CO_EMC401_INTERN_VOLT_HI:
        return "DS401: Internal voltage too high";
    case CO_EMC401_INTERN_VOLT_LO:
        return "DS401: Internal voltage too low";
    case CO_EMC401_OUT_VOLT_HIGH:
        return "DS401: Output voltage too high";
    case CO_EMC401_OUT_VOLT_LOW:
        return "DS401: Output voltage too low";
    case CO_EMC401_POWER_TEMP_OVER:
        return "High temperature of the PCB";
    case CO_EMC401_MOTOR_TEMP_OVER:
        return "High temperature of the motor";
    case CO_EMC401_SYS_ERROR:
        return "System error";
    case CO_EMC401_POINT_ERROR:
        return "System error: invalid motion point";
    case CO_EMC401_CURR_MEAS_OFFSET:
        return "Control: Current measurement offset";
    case CO_EMC401_EE_FAULT:
        return "EEPROM fault";
    case CO_EMC401_EE_CRC_ERROR:
        return "EEPROM checksum error";
    case CO_EMC401_CONF_ERROR:
        return "Configuration error";
    case CO_EMC401_ENC_CNT_ERROR:
        return "Encoder counting error";
    case CO_EMC401_VEL_FLW_ERROR:
        return "Velocity controller following error";
    case CO_EMC401_POS_LIMIT:
        return "Position controller limits";
    case CO_EMC401_POS_FLW_ERROR:
        return "Position controller following error";
    case CO_EMC401_POS_FLW_STATIC_ERROR:
        return "Position controller static following error";
    case CO_EMC401_ACCESS_ERROR:
        return "Unauthorized access";
    case CO_EMC401_PWRCTRL_ERROR:
        return "Power Stage Controller Error";
    case CO_EMC401_BUSY:
        return "Busy";
    case CO_EMC401_PROCEDURE_ERROR:
        return "Procedure error";
    case CO_EMC401_FORCE_OVER:
        return "Over force";
    case CO_EMC401_POWER_OVER:
        return "Over power";
    default:
        return "N/A";
    }
}

/**
 * @brief The function is the first to call to be able to work with the user API. It opens the COM port where the corresponding CAN-USB dongle
 * is connected, enabling communication between the user program and the servo motors on the respective CAN bus.
 <p><b>Example:</b></p>
 * <p>rr_can_interface_t *iface =  rr_init_interface	("/dev/ttyACM0");</p>
 * <p><code>if(!iface)<br>
 *{<br>
 *	... handle errors ...<br>
 *}</code></p>
 * @param interface_name Full path to the COM port to open. The path can vary, depending on the operating system.
 <p><b>Examples:</b></p>
 <p>OS Linux: "/dev/ttyACM0"</p>
 <p>mac OS: "/dev/cu.modem301"</p>
 * @return Interface descriptor (::rr_can_interface_t)<br> or NULL when an error occurs
 * @ingroup Init
 */
rr_can_interface_t *rr_init_interface(const char *interface_name)
{
    rr_can_interface_t *i = (rr_can_interface_t *)calloc(1, sizeof(rr_can_interface_t));

    if(!i)
    {
        return NULL;
    }

    rr_set_debug_log_stream(stderr);

    usbcan_instance_t *usbcan = usbcan_instance_init(interface_name);
	if(!usbcan)
	{
		free(i);
		return NULL;
	}
    usbcan->udata = i;
    i->iface = usbcan;

    usbcan_setup_nmt_state_cb(usbcan, rr_nmt_state_master_cb);
    usbcan_setup_emcy_cb(usbcan, rr_emcy_master_cb);

    if(!i->iface)
    {
        free(i);
        return NULL;
    }

    return i;
}

/**
 * @brief The function closes the COM port where the corresponding CAN-USB dongle is connected,
 * clearing all data associated with the interface descriptor.
 * It is advisable to call the function every time before quitting the user program.
 * @param iface Interface descriptor (see ::rr_init_interface).
 * @return Status code (::rr_ret_status_t)
 * @ingroup Init
 */
rr_ret_status_t rr_deinit_interface(rr_can_interface_t **iface)
{
    IS_VALID_INTERFACE(*iface);

    if(usbcan_instance_deinit((usbcan_instance_t **)&((*iface)->iface)))
    {
        free(*iface);
        *iface = NULL;
        return RET_OK;
    }
    return RET_ERROR;
}

/**
 * @brief The function determines whether the servo motor with the specified ID is connected to the specified interface. It waits for 2 seconds to receive a Heartbeat message from the servo.
 * When the message arrives within the interval, the servo is identified as successfully connected.<br>
 * <p>The function returns the servo descriptor that you will need for subsequent API calls to the servo.</p> 
 * @param iface Descriptor of the interface (returned by the ::rr_init_interface function) where the servo is connected
 * @param id Unique identifier of the servo in the specified interface. The available value range is from 0 to 127.
 * @return Servo descriptor (::rr_servo_t) <br> or NULL when no Heartbeat message is received within the specified interval
 * @ingroup Init
 */
rr_servo_t *rr_init_servo(rr_can_interface_t *iface, const uint8_t id)
{
	if(!iface)
	{
		return NULL;
	}
    rr_servo_t *s = (rr_servo_t *)calloc(1, sizeof(rr_servo_t));

    if(!s)
    {
        return NULL;
    }

    s->dev = usbcan_device_init((usbcan_instance_t *)iface->iface, id);

    if(!s->dev)
    {
        free(s);
        return NULL;
    }

    wait_device((usbcan_instance_t *)iface->iface, id, RR_API_WAIT_DEVICE_TIMEOUT_MS);

    return s;
}

/**
 * @brief The function deinitializes the servo, clearing all data associated with the servo descriptor.
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @return Status code (::rr_ret_status_t)
 * @ingroup Init
 */
rr_ret_status_t rr_deinit_servo(rr_servo_t **servo)
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
 * @brief The function reboots the servo specified in the 'servo' parameter of the function, resetting it to the power-on state.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_servo_reboot(const rr_servo_t *servo)
{
    IS_VALID_SERVO(servo);
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_RESET_NODE) ? RET_OK : RET_ERROR;
    ;
}

/**
 * @brief The function resets communication with the servo specified in the 'servo' parameter without resetting the entire interface.
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_servo_reset_communication(const rr_servo_t *servo)
{
    IS_VALID_SERVO(servo);
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_RESET_COMM) ? RET_OK : RET_ERROR;
    ;
}

/**
 * @brief The function sets the servo specified in the 'servo' parameter to the operational state. In the state, the servo is both available for communication and can execute commands.
 * <p>For instance, you may need to call the function to switch the servo from the pre-operational state to the operational one after an error (e.g., due to overcurrent).</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 *  If the parameter is set to 0, all servos connected to the interface will be set to the operational state.
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_servo_set_state_operational(const rr_servo_t *servo)
{
    IS_VALID_SERVO(servo);
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_GOTO_OP) ? RET_OK : RET_ERROR;
    ;
}

/**
 * @brief The function sets the servo specified in the 'servo' parameter to the pre-operational state. In the state, the servo is available for communication, but cannot execute any commands.  
 * <p>For instance, you may need to call the function, if you want to force the servo to stop executing commands, e.g., in an emergency.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_servo_set_state_pre_operational(const rr_servo_t *servo)
{
    IS_VALID_SERVO(servo);
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_GOTO_PREOP) ? RET_OK : RET_ERROR;
    ;
}

/**
 * @brief The function sets the servo specified in the 'servo' parameter to the stopped state. In the state, only Heartbeats are available. You can neither communicate with the servo nor make it execute any commands.  
 * <p>For instance, you may need to call the fuction to reduce the workload of a CAN bus by disabling individual servos connected to it without deninitializing them.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function  
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_servo_set_state_stopped(const rr_servo_t *servo)
{
    IS_VALID_SERVO(servo);
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    return write_nmt(dev->inst, dev->id, CO_NMT_CMD_GOTO_STOPPED) ? RET_OK : RET_ERROR;
    ;
}

/**
 * @brief The function reboots all servos connected to the interface specified in the 'interface' parameter,
 * resetting them back to the power-on state.
 * @param iface Interface descriptor returned by the ::rr_init_interface function
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_net_reboot(const rr_can_interface_t *iface)
{
    IS_VALID_INTERFACE(iface);
    usbcan_instance_t *inst = (usbcan_instance_t *)iface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_RESET_NODE) ? RET_OK : RET_ERROR;
    ;
}

/**
 * @brief The function resets communication via the interface specified in the 'interface' parameter.
 * For instance, you may need to use the function when changing settings that require a reset after modification.
 * @param iface Interface descriptor returned by the ::rr_init_interface function 
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_net_reset_communication(const rr_can_interface_t *iface)
{
    IS_VALID_INTERFACE(iface);
    usbcan_instance_t *inst = (usbcan_instance_t *)iface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_RESET_COMM) ? RET_OK : RET_ERROR;
    ;
}

/**
 * @brief The function sets all servos connected to the interface (CAN bus) specified in the 'interface' parameter to the operational state.
 *  In the state, the servos can both communicate with the user program and execute commands.
 *  <p>For instance, you may need to call the function to switch all servos on a specific bus from the
 *  pre-operational state to the operational one after an error (e.g., due to overcurrent).</p>
 * @param iface Interface descriptor returned by the ::rr_init_interface function  
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_net_set_state_operational(const rr_can_interface_t *iface)
{
    IS_VALID_INTERFACE(iface);
    usbcan_instance_t *inst = (usbcan_instance_t *)iface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_GOTO_OP) ? RET_OK : RET_ERROR;
    ;
}

/**
 * @brief The function sets all servos connected to the interface specified in the 'interface' parameter to the pre-operational state.  
 * In the state, the servos are available for communication, but cannot execute commands.
 * <p>For instance, you may need to call the function, if you want to force all servos on a specific bus to stop executing commands, e.g., in an emergency.</p>
 * @param iface Interface descriptor returned by the ::rr_init_interface function 
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_net_set_state_pre_operational(const rr_can_interface_t *iface)
{
    IS_VALID_INTERFACE(iface);
    usbcan_instance_t *inst = (usbcan_instance_t *)iface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_GOTO_PREOP) ? RET_OK : RET_ERROR;
    ;
}

/**
 * @brief The function sets all servos connected to the interface specified in the 'interface' parameter to the stopped state.  
 * In the state, the servos are neither available for communication nor can execute commands.
 * <p>For instance, you may need to call the fuction to stop all servos on a specific bus without deinitializing them.</p>
 * @param iface Interface descriptor returned by the ::rr_init_interface function. 
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_net_set_state_stopped(const rr_can_interface_t *iface)
{
    IS_VALID_INTERFACE(iface);
    usbcan_instance_t *inst = (usbcan_instance_t *)iface->iface;
    return write_nmt(inst, 0, CO_NMT_CMD_GOTO_STOPPED) ? RET_OK : RET_ERROR;
}

/**
 * @brief The function retrieves the actual NMT state of any device (a servo motor or any other) connected to the specified CAN network.
 * The state is described as a status code (:: rr_nmt_state_t). 
 * <p></p>
 * @param iface Interface descriptor returned by the ::rr_init_interface function 
 * @param id Identificator of the addressed device 
 * @param state Pointer to the variable where the state of the device is returned 
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_net_get_state(const rr_can_interface_t *iface, int id, rr_nmt_state_t *state)
{
    IS_VALID_INTERFACE(iface);
    usbcan_instance_t *inst = (usbcan_instance_t *)iface->iface;

	*state = (rr_nmt_state_t)usbcan_get_device_state(inst, id);

    return RET_OK;
}

/**
 * @brief The function retrieves the actual NMT state of a specified servo. The state is described as a status code (:: rr_nmt_state_t).  
 * <p></p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param state Pointer to the variable where the state of the servo is returned 
 * @return Status code (::rr_ret_status_t)
 * @ingroup State
 */
rr_ret_status_t rr_servo_get_state(const rr_servo_t *servo, rr_nmt_state_t *state)
{
    IS_VALID_SERVO(servo);
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;

	*state = (rr_nmt_state_t)usbcan_get_device_state(dev->inst, dev->id);

    return RET_OK;
}

/**
 * @brief The function sets the specified servo to the released state. The servo is de-energized 
 * and continues rotating for as long as it is affected by external forces (e.g., inertia, gravity).
 * <p><b>Note:</b> Affected by external force, the servo may also begin rotating in the opposite direction.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_release(const rr_servo_t *servo)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data = 0;
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = write_raw_sdo(dev, 0x2010, 0x01, &data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief The function sets the specified servo to the freeze state. The servo stops, retaining its last position.
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_freeze(const rr_servo_t *servo)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data = 0;
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = write_raw_sdo(dev, 0x2010, 0x02, &data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief The function sets the current supplied to the stator of the servo specified in the 'servo' parameter.
 * Changing the 'current_a parameter' value, it is possible to adjust the servo's torque (Torque = stator current*Kt).
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param current_a Phase current of the stator in Amperes
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_set_current(const rr_servo_t *servo, const float current_a)
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
 * @brief The function applies or releases the servo's built-in brake.
 * If a servo is supplied without a brake, the function will not work.
 * In this case, to stop a servo, use either the ::rr_freeze() or ::rr_release() function.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param en Desired action: true - engage brake, false - disengage
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_brake_engage(const rr_servo_t *servo, const bool en)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data = en ? 1 : 0;
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;

    uint32_t sts = write_raw_sdo(dev, 0x2010, 0x03, &data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief The function sets the output shaft velocity with which the specified servo should move at its maximum current.
 * The maximum current is in accordance with the servo motor specification. When you need to set a lower current limit, use the ::rr_set_velocity_with_limits function.
 * <p>The rr_set_velocity() function works with geared servos only. When a servo includes no gearhead, use the ::rr_set_velocity_motor() for setting servo velocity.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @param velocity_deg_per_sec Velocity (in degrees/sec) at the servo flange 
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_set_velocity(const rr_servo_t *servo, const float velocity_deg_per_sec)
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
 * @brief The function sets the velocity with which the motor of the specified servo should move at its maximum current.
 * The maximum current is in accordance with the servo motor specification.
 * <p>You can use the function for both geared servos and servos without a gearhead.
 * When a servo is geared, the velocity at the output flange will depend on the applied gear ratio (refer to the servo motor specification).</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @param velocity_rpm Velocity of the motor (in revolutions per minute)
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_set_velocity_motor(const rr_servo_t *servo, const float velocity_rpm)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[3];
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float24(data, 0, &velocity_rpm, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2012, 0x03, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief The function sets the position that the specified servo should reach as a result of executing the command.
 * The velocity and current are maximum values in accordance with the servo motor specifications.
 * For setting lower velocity and current limits, use the ::rr_set_position_with_limits function.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param position_deg Position of the servo (in degrees) to be reached. The parameter is a multi-turn value (e.g., when set to 720, the servo will make two turns, 360 degrees each).
  When the parameter is set to a "-" sign value, the servo will rotate in the opposite direction. 
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_set_position(const rr_servo_t *servo, const float position_deg)
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
 * @brief The function commands the specified servo to rotate at the specified velocity,
 * while setting the maximum limit for the servo current (below the servo motor specifications).
 * The velocity value is the velocity of the servo’s output shaft.
 * <p>Similarly to ::rr_set_velocity(), this function can be used for geared servos only.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param velocity_deg_per_sec Velocity (in degrees/sec) at the servo flange. The value can have a "-" sign, in which case the servo will rotate in the opposite direction.
 * @param current_a Maximum user-defined current limit in Amperes.
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_set_velocity_with_limits(const rr_servo_t *servo, const float velocity_deg_per_sec, const float current_a)
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
 * @brief The function sets the position that the specified servo should reach
 * at user-defined velocity and current as a result of executing the command.
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @param position_deg Final position of the servo flange (in degrees) to be reached
 * @param velocity_deg_per_sec Velocity (in degrees/sec)at which the servo should move to the specified position
 * @param current_a Maximum user-defined current limit in Amperes
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_set_position_with_limits(const rr_servo_t *servo, const float position_deg, const float velocity_deg_per_sec, const float current_a)
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
 * @brief The function limits the input voltage supplied to the servo, enabling to adjust its motion velocity.
 * For instance, when the input voltage is 20V, setting the duty_percent parameter to 40% will result in 8V supplied to the servo.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param duty_percent User-defined percentage of the input voltage to be supplied to the servo
 * @return Status code (::rr_ret_status_t)
 * @ingroup Motion
 */
rr_ret_status_t rr_set_duty(const rr_servo_t *servo, float duty_percent)
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
 * @brief The function enables creating PVT (position-velocity-time) points to set the motion trajectory of the servo specified in the 'servo' parameter.
 * PVT points define the following:<br>
 * <ul><li>what position the servo specified in the 'servo' parameter should reach</li><br>
 * <li>how fast the servo should move to the specified position</li><br>
 * <li>how long the movement to the specified position should take</li></ul>
 * <p>The graphs below illustrate how a servo builds a trajectory based on the preset PVT points.<br> 
 * <b>Note:</b> In this case, the preset position values are 0, 45, 90, 45, and 0 degrees; the preset velocity values are 0, 30, 15, 30, 0 degrees per second;
 * the time values are equal to delta time between two adjacent points on the Time axis (e.g., 2,000ms-3,000ms=1,000ms).</p> 
 * @image html "PVT.png" "Example of calculated trajectory using PVT points" width=600 
 * @image latex "PVT.png" "Example of calculated trajectory using PVT points" width=300pt
 * Created PVT points are arranged into a motion queue that defines the motion trajectory of the specified servo. To execute the motion queue, 
 * use the ::rr_start_motion function.<br>
 * When any of the parameter values (e.g., position, velocity) exceeds user-defined limits or the servo motor specifications 
 * (whichever is the smallest value), the function returns an error.
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @param position_deg Position that the servo flange (in degrees) should reach as a result of executing the command
 * @param velocity_deg_per_sec Velocity(in degrees/sec) at which the servo should move to reach the specified position
 * @param time_ms Time (in milliseconds) it should take the servo to move from the previous position (PVT point in a motion trajectory or an initial point) to the commanded one.
 * The maximum admissible value is (2^32-1)/10 (roughly equivalent to 4.9 days). 
 * @return Status code (::rr_ret_status_t)
 * @ingroup Trajectory
 */
rr_ret_status_t rr_add_motion_point(const rr_servo_t *servo, const float position_deg, const float velocity_deg_per_sec, const uint32_t time_ms)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[12];
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &position_deg, 1);
    usb_can_put_float(data + 4, 0, &velocity_deg_per_sec, 1);
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
 * @brief The function enables creating PVAT (position-velocity-acceleration-time) points to set the motion trajectory of the servo specified in the 'servo' parameter.
 * PVAT points define the following:<br>
 * <ul><li>what position the servo specified in the 'servo' parameter should reach</li><br>
 * <li>how fast the servo should move to the specified position</li><br>
 * <li>how long the movement to the specified position should take</li></ul>
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @param position_deg Position that the servo flange (in degrees) should reach as a result of executing the command
 * @param velocity_deg_per_sec Velocity (in degrees/sec) at which the servo should move to reach the specified position
 * @param accel_deg_per_sec2 Acceleration (in degrees/sec^2) at the end of the motion point
 * @param time_ms Time (in milliseconds) it should take the servo to move from the previous position (PVT point in a motion trajectory or an initial point) to the commanded one.
 * The maximum admissible value is (2^32-1)/10 (roughly equivalent to 4.9 days). 
 * @return Status code (::rr_ret_status_t)
 * @ingroup Trajectory
 */
rr_ret_status_t rr_add_motion_point_pvat(
    const rr_servo_t *servo,
    const float position_deg,
    const float velocity_deg_per_sec,
    const float accel_deg_per_sec2,
    const uint32_t time_ms)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[16];
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &position_deg, 1);
    usb_can_put_float(data + 4, 0, &velocity_deg_per_sec, 1);
    usb_can_put_float(data + 8, 0, &accel_deg_per_sec2, 1);
    usb_can_put_uint32_t(data + 12, 0, &time_ms, 1);

    uint32_t sts = write_raw_sdo(dev, 0x2200, 3, data, sizeof(data), 1, 200);
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
 * @brief The function commands all servos connected to the specified interface (CAN bus) 
 * to move simultaneously through a number of preset PVT points (see ::rr_add_motion_point).<br> 
 * <p><b>Note:</b> When any of the servos fails to reach any of the PVT points due to an error, it will broadcast
 * a "Go to Stopped State" command to all the other servos on the same bus. The servos will stop executing the preset PVT points and go to the stopped state.
 * In the state, only Heartbeats are available. You can neither communicate with servos nor command them to execute any operations.</p>
 * <p><b>Note:</b> Once servos execute the last PVT in their preset motion queue, the queue is cleared automatically.</p>
 * @param iface Interface descriptor returned by the ::rr_init_interface function
 * @param timestamp_ms Delay (in milliseconds) before the servos associated with the interface start to move. When the value is set to 0, the servos will start moving immediately.
 * The available value range is from 0 to 2^24-1.
 * @return Status code (::rr_ret_status_t)
 * @ingroup Trajectory
 */
rr_ret_status_t rr_start_motion(rr_can_interface_t *iface, uint32_t timestamp_ms)
{
    IS_VALID_INTERFACE(iface);

    usbcan_instance_t *inst = (usbcan_instance_t *)iface->iface;
    write_timestamp(inst, timestamp_ms);
    return RET_OK;
}

/**
 * @brief The functions enables reading the total actual count of servo hardware errors (e.g., no Heartbeats/overcurrent, etc.).
 * In addition, the function returns the codes of all the detected errors as a single array. 
 * <p><b>Note</b>: The ::rr_ret_status_t codes returned by API functions only indicate that an error occured during communication between the user program and a servo.
 * If it is a hardware error, the ::rr_ret_status_t code will be ::RET_ERROR. Use ::rr_read_error_status to determine the cause of the error.</p>  
 * @param servo Servo Servo descriptor returned by the ::rr_init_servo function  
 * @param error_count Pointer to the variable where the function will save the current servo error count
 * @param error_array Pointer to the array where the function will save the codes of all errors. Default array size is ::ARRAY_ERROR_BITS_SIZE
 * <b>Note:</b> Call the ::rr_describe_emcy_bit function, to get a detailed error code description.
 * If the array is not used, set the parameter to 0.
 * @return Status code (::rr_ret_status_t)
 * @ingroup Err
 */
rr_ret_status_t rr_read_error_status(const rr_servo_t *servo, uint32_t *const error_count, uint8_t *const error_array)
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
        for(int i = 0; i < size * 8; i++)
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
 * @brief The function is always used in combination with the ::rr_param_cache_setup_entry function. It retreives from the servo the array of parameters
 * that was set up using ::rr_param_cache_setup_entry function and saves the array to the program cache. You can subsequently read the parameters from the program
 * cache with the ::rr_read_cached_parameter function. For more information, see ::rr_param_cache_setup_entry.
 * <p><b>Note</b>: After you exit the program, the cache will be cleared.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @return Status code (::rr_ret_status_t)
 * @ingroup Realtime
 */
rr_ret_status_t rr_param_cache_update(rr_servo_t *servo)
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
 * @brief The function is the fist one in the API call sequence that enables reading multiple servo paramaters (e.g., velocity, voltage,
 * and position) as a data array. Using the sequence is advisable when you need to read <b>more than one parameter at a time</b>.
 * The user can set up the array to include up to 50 parameters.
 * In all, the sequence comprises the following functions:
 * <ul> <li>::rr_param_cache_setup_entry for setting up an array of servo parameters to read</li>
 * <li>::rr_param_cache_update for retreiving the parameters from the servo and saving them to the program cache</li>
 * <li>::rr_read_cached_parameter for reading parameters from the program cache</li></ul>
 * Using the sequence of API calls allows for speeding up data acquisition by nearly two times.
 * Let's assume you need to read 49 parameters. At a bit rate of 1 MBit/s, reading them one by one will take about 35 ms, 
 * whereas reading them as an array will only take 10 ms.
 * <p><b>Note:</b> When you need to read a single parameter, it is better to use the ::rr_read_parameter function.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @param param Index of the parameter to read as indicated in the ::rr_servo_param_t list (e.g., APP_PARAM_POSITION_ROTOR)
 * @param enabled Set True/False to enable/ disable the specified parameter for reading
 * @return Status code (::rr_ret_status_t)
 * @ingroup Realtime
 */
rr_ret_status_t rr_param_cache_setup_entry(rr_servo_t *servo, const rr_servo_param_t param, bool enabled)
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
 * @brief The function enables reading a single parameter directly from the servo specified in the 'servo' parameter of the function. 
 * The function returns the current value of the parameter.
 * Additionally, the parameter is saved to the program cache, irrespective of whether 
 * it was enabled/ disabled with the ::rr_param_cache_setup_entry function.
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @param param Index of the parameter to read; you can find these indices in the ::rr_servo_param_t list (e.g., APP_PARAM_POSITION_ROTOR).
 * @param value Pointer to the variable where the function will save the reading
 * @return Status code (::rr_ret_status_t)
 * @ingroup Realtime
 */
rr_ret_status_t rr_read_parameter(rr_servo_t *servo, const rr_servo_param_t param, float *value)
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
 * @brief The function is always used in combination with the ::rr_param_cache_setup_entry and the ::rr_param_cache_update functions.
 * For more information, see ::rr_param_cache_setup_entry.
 * <p>The function enables reading parameters from the program cache. If you want to read more than one parameter,
 * you will need to make a separate API call for each of them.</p>
 * <b>Note</b>: Prior to reading a parameter, make sure to update the program cache using the ::rr_param_cache_update function.
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @param param Index of the parameter to read; you can find these indices in the ::rr_servo_param_t list (e.g., APP_PARAM_POSITION_ROTOR)
 * @param value Pointer to the variable where the function will save the reading
 * @return Status code (::rr_ret_status_t)
 * @ingroup Realtime
 */
rr_ret_status_t rr_read_cached_parameter(rr_servo_t *servo, const rr_servo_param_t param, float *value)
{
    IS_VALID_SERVO(servo);
    *value = servo->pcache[param].value;
    return RET_OK;
}

/**
 * @brief The function clears the entire motion queue of the servo specified in the 'servo' parameter of the function.
 * The servo completes the move it started before the function call and then clears all the remaining PVT points in the queue.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @return Status code (::rr_ret_status_t)
 * @ingroup Trajectory
 */
rr_ret_status_t rr_clear_points_all(const rr_servo_t *servo)
{
    return rr_clear_points(servo, 0);
}

/**
 * @brief The function removes the number of PVT points indicated in the 'num_to_clear' parameter from the tail of the motion queue
 * preset for the specified servo. When the indicated number of PVT points to be removed exceeds the actual remaining number of PVT points in the queue, 
 * the funtion clears only the actual remaining number of PVT points.
 * @param servo Servo descriptor returned by the ::rr_init_servo function
 * @param num_to_clear Number of PVT points to be removed from the motion queue of the specified servo
 * @return Status code (::rr_ret_status_t)
 * @ingroup Trajectory
 */
rr_ret_status_t rr_clear_points(const rr_servo_t *servo, const uint32_t num_to_clear)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = write_raw_sdo(dev, 0x2202, 0x01, (uint8_t *)&num_to_clear, sizeof(num_to_clear), 1, 100);
    return ret_sdo(sts);
}

/**
 * @brief The function returns the actual motion queue size of the specified servo.
 * The return value indicates how many PVT points have already been added to the motion queue.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param num Pointer to the parameter where the function will save the reading
 * @return Status code (::rr_ret_status_t)
 * @ingroup Trajectory
 */
rr_ret_status_t rr_get_points_size(const rr_servo_t *servo, uint32_t *num)
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
 * @brief The function returns how many more PVT points the user can add to the motion queue of the servo specified in the 'servo' parameter.
 * <p><b>Note:</b> Currently, the maximum motion queue size is 100 PVT.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param num Pointer to the variable where the function will save the reading
 * @return Status code (::rr_ret_status_t)
 * @ingroup Trajectory
 */
rr_ret_status_t rr_get_points_free_space(const rr_servo_t *servo, uint32_t *num)
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
 * @brief The function enables calculating the time it will take for the specified servo to get from one position to another at the specified motion parameters (e.g., velocity, acceleration).
 * <b>Note:</b>The function is executed without the servo moving.<br>
 *<p>When the start time and the end time parameters are set to 0, the function returns the calculated time value. When the parameters are set to
 * values other than 0, the function will either return OK or an error. 'OK' means the motion at the specified function parameters is possible,
 * whereas an error indicates that the motion cannot be executed.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param start_position_deg Position (in degrees) from where the specified servo should start moving
 * @param start_velocity_deg_per_sec Servo velocity (in degrees/sec) at the start of motion
 * @param start_acceleration_deg_per_sec2 Servo acceleration (in degrees/sec^2) at the start of motion
 * @param start_time_ms Initial time setting (in milliseconds)
 * @param end_position_deg Position (in degrees) where the servo should arrive
 * @param end_velocity_deg_per_sec Servo velocity (in degrees/sec) in the end of motion
 * @param end_acceleration_deg_per_sec2 Servo acceleration (in degrees/sec^2) in the end of motion
 * @param end_time_ms Final time setting (in milliseconds)
 * @param time_ms Pointer to the variable where the function will save the calculated time
 * @return Status code (::rr_ret_status_t)
 * @ingroup Trajectory
 */
rr_ret_status_t rr_invoke_time_calculation(const rr_servo_t *servo,
                                           const float start_position_deg, const float start_velocity_deg_per_sec, const float start_acceleration_deg_per_sec2, const uint32_t start_time_ms,
                                           const float end_position_deg, const float end_velocity_deg_per_sec, const float end_acceleration_deg_per_sec2, const uint32_t end_time_ms,
                                           uint32_t *time_ms)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[8 * 4];
    int p = 0;
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;

    p = usb_can_put_float(data, p, &start_position_deg, 1);
    p = usb_can_put_float(data, p, &start_velocity_deg_per_sec, 1);
    p = usb_can_put_float(data, p, &start_acceleration_deg_per_sec2, 1);
    p = usb_can_put_uint32_t(data, p, &start_time_ms, 1);

    p = usb_can_put_float(data, p, &end_position_deg, 1);
    p = usb_can_put_float(data, p, &end_velocity_deg_per_sec, 1);
    p = usb_can_put_float(data, p, &end_acceleration_deg_per_sec2, 1);
    p = usb_can_put_uint32_t(data, p, &end_time_ms, 1);

    uint32_t sts = write_raw_sdo(dev, 0x2203, 0x01, data, sizeof(data), 1, 200);

    if(sts == CO_SDO_AB_NONE)
    {
        if(time_ms == 0)
        {
            return RET_WRONG_ARG;
        }

        uint8_t data[4];
        int len = sizeof(data);
        uint32_t sts = read_raw_sdo(dev, 0x2203, 0x02, data, &len, 1, 100);

        if(sts == CO_SDO_AB_NONE && len == 4)
        {
            usb_can_get_uint32_t(data, 0, time_ms, 1);
            return RET_OK;
        }
    }
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
 * @brief The function enables setting the current position (in degrees) of the servo with the specified descriptor to any value defined by the user.
 * For instance, when the current servo position is 101 degrees and the 'position_deg' parameter is set to 25 degrees, the servo is assumed to be positioned at 25 degrees.
* <p>The setting is volatile: after a reset or a power outage, it is no longer valid.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param position_deg User-defined position (in degrees) to replace the current position value
 * @return Status code (::rr_ret_status_t)
 * @ingroup Config
 */
rr_ret_status_t rr_set_zero_position(const rr_servo_t *servo, const float position_deg)
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
 * @brief The function enables setting the current position (in degrees) of the servo with the specified descriptor to any value defined by the user and
 * saving it to the FLASH memory. If you don't want to save the newly set position, use the ::rr_set_zero_position function.<br>
 * <p><b>Note:</b>The FLASH memory limit is 1,000 write cycles. Therefore, it is not advisable to use the function on a regular basis.</p>
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param position_deg User-defined position (in degrees) to replace the current position value
 * @return Status code (::rr_ret_status_t)
 * @ingroup Config
 */
rr_ret_status_t rr_set_zero_position_and_save(const rr_servo_t *servo, const float position_deg)
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
 * @brief The function reads the maximum velocity of the servo at the current moment. It returns the smallest of the three values—the user-defined maximum velocity limit (::rr_set_max_velocity),
 * the maximum velocity value based on the servo specifications, or the calculated maximum velocity based on the supply voltage.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param velocity_deg_per_sec Maximum servo velocity (in degrees/sec)
 * @return Status code (::rr_ret_status_t)
 * @ingroup Config
 */
rr_ret_status_t rr_get_max_velocity(const rr_servo_t *servo, float *velocity_deg_per_sec)
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
 * @brief The function sets the maximum velocity limit for the servo specified in the 'servo' parameter.
 * The setting is volatile: after a reset or a power outage, it is no longer valid.
 * @param servo Servo descriptor returned by the ::rr_init_servo function 
 * @param max_velocity_deg_per_sec Velocity at the servo flange (in degrees/sec)
 * @return Status code (::rr_ret_status_t)
 * @ingroup Config
 */
rr_ret_status_t rr_set_max_velocity(const rr_servo_t *servo, const float max_velocity_deg_per_sec)
{
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint8_t data[4];
    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    usb_can_put_float(data, 0, &max_velocity_deg_per_sec, 1);
    uint32_t sts = write_raw_sdo(dev, 0x2300, 0x03, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}

//! @cond Doxygen_Suppress
/**
 * @brief The function changes device CAN ID, resets device CAN communication and check that heartbeat is present with the new CAN ID.
 * Note: servo device cache will be erased (in the API)
 * 
 * @param iface Descriptor of the interface (as returned by the ::rr_init_interface function)
 * @param servo Servo descriptor returned by the ::rr_init_servo function. Will be changed if the functon status is success
 * @param new_can_id New CAN ID. It can be any value within the range from 1 to 127.
 * @return Status code (::rr_ret_status_t)
 */
static rr_ret_status_t rr_change_id(rr_can_interface_t *iface, rr_servo_t **servo, uint8_t new_can_id)
{
    if(new_can_id < 1 || new_can_id > 127) return RET_WRONG_ARG;

    /* Check that new id is not the same */
    usbcan_device_t *dev = (usbcan_device_t *)(*servo)->dev;
    if(dev->id == new_can_id) return RET_OK;

    IS_VALID_SERVO(*servo);
    CHECK_NMT_STATE(*servo);

    /* Write new CAN ID to the dictionary */
    uint8_t data[1];
    usb_can_put_uint8_t(data, 0, &new_can_id, 1);
    uint32_t node_id_sts = write_raw_sdo(dev, 0x2100, 0x00, data, sizeof(data), 1, 100);
    if(node_id_sts) return ret_sdo(node_id_sts);

    /* Reset communication, so the servo will update it's internal CAN ID with the ID in the dictionary */
    rr_ret_status_t reset_comm_sts = rr_net_reset_communication(iface);
    if(reset_comm_sts) return reset_comm_sts;
    
    /* Deinit the servo */
    rr_ret_status_t deinit_sts = rr_deinit_servo(servo);
    if(deinit_sts) return deinit_sts;

    /* Initialize it with the new ID */
    *servo = rr_init_servo(iface, new_can_id);
    if(*servo == NULL) return RET_BAD_INSTANCE;

    return RET_OK;
}
/// @endcond

/**
 * @brief The function enables changing the default CAN identifier (ID) of the specified servo to avoid collisions on a bus line. <b>Important!</b> Each servo connected to a CAN bus must have <b>a unique ID</b>.<br>
 * <p>When called, the function resets CAN communication for the specified servo, checks that Heartbeats are generated for the new ID, 
 * and saves the new CAN ID to the EEPROM memory of the servo.</p> 
 * <p><b>Note:</b> The EEPROM memory limit is 1,000 write cycles. Therefore, it is advisable to use the function with discretion.</p>
 * @param iface Descriptor of the interface (as returned by the ::rr_init_interface function)
 * @param servo Servo descriptor returned by the ::rr_init_servo function. <b>Note</b>: All RDrive servos are supplied with <b>the same default CAN ID—32</b>.
 * @param new_can_id New CAN ID. You can set any value within the range from 1 to 127, only make sure <b>no other servo has the same ID</b>.
 * @return Status code (::rr_ret_status_t)
 * @ingroup Aux
 */
rr_ret_status_t rr_change_id_and_save(rr_can_interface_t *iface, rr_servo_t **servo, uint8_t new_can_id)
{
    rr_ret_status_t sts = rr_change_id(iface, servo, new_can_id);
    if(sts) return sts;

#define PARAM_STORE_PASSWORD 0x73617665 // s a v e

    uint32_t pass = PARAM_STORE_PASSWORD;
    uint8_t data[4];
    usb_can_put_uint32_t(data, 0, &pass, 1);
    usbcan_device_t *dev = (usbcan_device_t *)(*servo)->dev;
    uint32_t save_conf_sts = write_raw_sdo(dev, 0x1010, 0x01, data, sizeof(data), 1, 4000);
    if(save_conf_sts) return ret_sdo(save_conf_sts);

    return RET_OK;
}

/**
 * @brief The function reads hardware version of the device (unique ID of the MCU + hardware type + hardware revision)
 * 
 * @param servo Servo descriptor returned by the ::rr_init_servo function. <b>Note</b>: All RDrive servos are supplied with <b>the same default CAN ID—32</b>.
 * @param version_string Pointer to the ASCII string that will be read
 * @param version_string_size Input: size of the ::version_string, Output: size of the readed string
 * @return Status code (::rr_ret_status_t)
 */
rr_ret_status_t rr_get_hardware_version(const rr_servo_t *servo, char *version_string, int *version_string_size)
{
    // Note: see http://wiki.rozum.com/display/EMB/Versioning
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = read_raw_sdo(dev, 0x1009, 0x00, (uint8_t *)version_string, version_string_size, 1, 100);

    return ret_sdo(sts);
}

/**
 * @brief The function reads software version of the device (minor + major + firmware build date)
 * 
 * @param servo Servo descriptor returned by the ::rr_init_servo function. <b>Note</b>: All RDrive servos are supplied with <b>the same default CAN ID—32</b>.
 * @param version_string Pointer to the ASCII string that will be read
 * @param version_string_size Input: size of the ::version_string, Output: size of the readed string
 * @return Status code (::rr_ret_status_t)
 */
rr_ret_status_t rr_get_software_version(const rr_servo_t *servo, char *version_string, int *version_string_size)
{
    // Note: see http://wiki.rozum.com/display/EMB/Versioning
    IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint32_t sts = read_raw_sdo(dev, 0x100A, 0x00, (uint8_t *)version_string, version_string_size, 1, 100);

    return ret_sdo(sts);
}
