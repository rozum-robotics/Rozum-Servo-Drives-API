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
 * - \ref Common
 * - \ref System_control
 * - \ref Servo_control
 * - \ref Servo_config
 * - \ref Servo_info
 * 
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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//! @cond Doxygen_Suppress
#define RR_API_WAIT_DEVICE_TIMEOUT_MS 2000
/* Private macro -------------------------------------------------------------*/
//! @cond Doxygen_Suppress
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
//! @endcond

/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static int ret_sdo(int code)
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
 * @param f - stdio stream to write communication log to, no logging if 'NULL'
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
 * @param f - stdio stream to write debug log to, no logging if 'NULL'
 * @return void
 * @ingroup Utils
 */
void rr_set_debug_log_stream(FILE *f)
{
    usbcan_set_debug_log_stream(f);
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

	i->iface = usbcan_instance_init(interface_name);

	if(!i->iface)
	{
		free(i);
		return NULL;
	}

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
    uint8_t sts = write_raw_sdo(dev, 0x2010, 0x01, &data, sizeof(data), 1, 100);

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
    uint8_t sts = write_raw_sdo(dev, 0x2010, 0x02, &data, sizeof(data), 1, 100);

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
    uint8_t sts = write_raw_sdo(dev, 0x2012, 0x01, data, sizeof(data), 1, 100);

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
    uint8_t sts = write_raw_sdo(dev, 0x2012, 0x03, data, sizeof(data), 1, 100);

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
    uint8_t sts = write_raw_sdo(dev, 0x2012, 0x04, data, sizeof(data), 1, 100);

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
    uint8_t sts = write_raw_sdo(dev, 0x2012, 0x05, data, sizeof(data), 1, 100);

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
    uint8_t sts = write_raw_sdo(dev, 0x2012, 0x06, data, sizeof(data), 1, 100);

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
    uint8_t sts = write_raw_sdo(dev, 0x2012, 0x07, data, sizeof(data), 1, 100);

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
 * @param array Pointer to the error array
 * @param size Size of the received array
 * @return int Status code (::rr_ret_status_t)
 * @ingroup Servo_info
 */
int rr_read_error_status(const rr_servo_t *servo, uint8_t *array, uint32_t *size)
{
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    int _size = *size;
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint8_t sts = read_raw_sdo(dev, 0x2000, 0, array, &_size, 1, 200);

    *size = _size;
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

    int sts = write_raw_sdo(dev, 0x2015, 1, array, sizeof(array), 1, 200);

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

    int sts = read_raw_sdo(dev, 0x2013, param, data, &size, 2, 100);
    if(sts == CO_SDO_AB_NONE && size == 4)
    {        
        usb_can_get_float(data, 0, (float *)&servo->pcache[param].value, 1);
        *value = servo->pcache[param].value;
        return RET_OK;
    }

    return ret_sdo(sts);
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
	IS_VALID_SERVO(servo);
    CHECK_NMT_STATE(servo);

    uint32_t num = 0;
	usbcan_device_t *dev = (usbcan_device_t *)servo->dev;
    uint8_t sts = write_raw_sdo(dev, 0x2202, 0x01, (uint8_t *)&num, sizeof(num), 1, 100);
    return ret_sdo(sts);
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
    uint8_t sts = write_raw_sdo(dev, 0x2202, 0x01, (uint8_t *)&num_to_clear, sizeof(num_to_clear), 1, 100); 
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
    uint8_t sts = read_raw_sdo(dev, 0x2202, 0x02, data, &len, 1, 100);

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
    uint8_t sts = read_raw_sdo(dev, 0x2202, 0x03, data, &len, 1, 100);

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

    uint8_t sts = write_raw_sdo(dev, 0x2203, 0x01, data, sizeof(data), 1, 200);

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
    uint8_t sts = read_raw_sdo(dev, 0x2203, 0x02, data, &len, 1, 100);
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
    uint8_t sts = write_raw_sdo(dev, 0x2208, 0x01, data, sizeof(data), 0, 200);

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
    uint8_t sts = write_raw_sdo(dev, 0x2208, 0x02, data, sizeof(data), 0, 200);

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
    uint8_t sts = read_raw_sdo(dev, 0x2207, 0x02, data, &len, 1, 100);
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
    uint8_t sts = write_raw_sdo(dev, 0x2300, 0x03, data, sizeof(data), 1, 100);

    return ret_sdo(sts);
}
