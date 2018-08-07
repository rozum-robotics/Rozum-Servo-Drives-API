/**
 * @brief Rozum Robotics API Header File
 * 
 * @file api.h
 * @author Rozum
 * @date 2018-06-01
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _ROZUM_API_H
#define _ROZUM_API_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Exported macro ------------------------------------------------------------*/
/**
 * @brief Standard debug 
 * 
 */
#define API_DEBUG(...) fprintf(stderr, __VA_ARGS__)

/**
 * @brief Make a string from the variable 
 * 
 */
#define STRFY(x) #x

/* Exported constants --------------------------------------------------------*/
/**
 * @brief Default size of the error bits array
 * 
 */
#define ARRAY_ERROR_BITS_SIZE (64)

/* Exported types ------------------------------------------------------------*/
/**
 * @brief Return codes of the API functions
 */
typedef enum
{
    RET_OK = 0,        ///< Status OK
    RET_ERROR,         ///< Generic error
    RET_BAD_INSTANCE,  ///< Bad interface or servo instance (null)
    RET_BUSY,          ///< Device is busy
    RET_WRONG_TRAJ,    ///< Wrong trajectory
    RET_LOCKED,        ///< Device is locked
    RET_STOPPED,       ///< Device is in the STOPPED state
    RET_TIMEOUT,       ///< Communication timeout
    RET_ZERO_SIZE,     ///< Zero size
    RET_SIZE_MISMATCH, ///< Received and target size mismatch
    RET_WRONG_ARG      ///< Wrong function argument
} rr_ret_status_t;

/**
 * @brief Device parameter and source indices
 * 
 */
typedef enum
{
    APP_PARAM_NULL = 0,                     ///< Not used
    APP_PARAM_POSITION,                     ///< Actual multi-turn position of the output shaft (degrees)
    APP_PARAM_VELOCITY,                     ///< Actual velocity of the output shaft (degrees per second)
    APP_PARAM_POSITION_ROTOR,               ///< Actual position of the motor shaft (degrees)
    APP_PARAM_VELOCITY_ROTOR,               ///< Actual velocity of the motor shaft (degrees per second)
    APP_PARAM_POSITION_GEAR_360,            ///< Actual single-turn position of the output shaft (from 0 to 360 degrees)
    APP_PARAM_POSITION_GEAR_EMULATED,       ///< Actual multi-turn position of the motor shaft multiplied by gear ratio (degrees)
    APP_PARAM_CURRENT_INPUT,                ///< Actual DC current (Amperes)
    APP_PARAM_CURRENT_OUTPUT,               ///< Not used
    APP_PARAM_VOLTAGE_INPUT,                ///< Actual DC voltage (Volts)
    APP_PARAM_VOLTAGE_OUTPUT,               ///< Not used
    APP_PARAM_CURRENT_PHASE,                ///< Actual magnitude of AC current (Amperes)
    APP_PARAM_TEMPERATURE_ACTUATOR,         ///< Not used
    APP_PARAM_TEMPERATURE_ELECTRONICS,      ///< Actual temperature of the motor controller
    APP_PARAM_TORQUE,                       ///< Not used
    APP_PARAM_ACCELERATION,                 ///< Not used
    APP_PARAM_ACCELERATION_ROTOR,           ///< Not used
    APP_PARAM_CURRENT_PHASE_1,              ///< Actual phase 1 current
    APP_PARAM_CURRENT_PHASE_2,              ///< Actual phase 2 current
    APP_PARAM_CURRENT_PHASE_3,              ///< Actual phase 3 current
    APP_PARAM_CURRENT_RAW,                  ///< Not used
    APP_PARAM_CURRENT_RAW_2,                ///< Not used
    APP_PARAM_CURRENT_RAW_3,                ///< Not used
    APP_PARAM_ENCODER_MASTER_TRACK,         ///< Internal use only
    APP_PARAM_ENCODER_NONIUS_TRACK,         ///< Internal use only
    APP_PARAM_ENCODER_MOTOR_MASTER_TRACK,   ///< Internal use only
    APP_PARAM_ENCODER_MOTOR_NONIUS_TRACK,   ///< Internal use only
    APP_PARAM_TORQUE_ELECTRIC_CALC,         ///< Internal use only
    APP_PARAM_CONTROLLER_VELOCITY_ERROR,    ///< Velocity following error
    APP_PARAM_CONTROLLER_VELOCITY_SETPOINT, ///< Velocity target
    APP_PARAM_CONTROLLER_VELOCITY_FEEDBACK, ///< Actual velocity (degrees per second)
    APP_PARAM_CONTROLLER_VELOCITY_OUTPUT,   ///< Not used
    APP_PARAM_CONTROLLER_POSITION_ERROR,    ///< Position following error
    APP_PARAM_CONTROLLER_POSITION_SETPOINT, ///< Position target
    APP_PARAM_CONTROLLER_POSITION_FEEDBACK, ///< Actual position (degrees)
    APP_PARAM_CONTROLLER_POSITION_OUTPUT,   ///< Not used
    APP_PARAM_CONTROL_MODE,                 ///< Internal use only
    APP_PARAM_FOC_ANGLE,                    ///< Internal use only
    APP_PARAM_FOC_IA,                       ///< Internal use only
    APP_PARAM_FOC_IB,                       ///< Internal use only
    APP_PARAM_FOC_IQ_SET,                   ///< Internal use only
    APP_PARAM_FOC_ID_SET,                   ///< Internal use only
    APP_PARAM_FOC_IQ,                       ///< Internal use only
    APP_PARAM_FOC_ID,                       ///< Internal use only
    APP_PARAM_FOC_IQ_ERROR,                 ///< Internal use only
    APP_PARAM_FOC_ID_ERROR,                 ///< Internal use only
    APP_PARAM_FOC_UQ,                       ///< Internal use only
    APP_PARAM_FOC_UD,                       ///< Internal use only
    APP_PARAM_FOC_UA,                       ///< Internal use only
    APP_PARAM_FOC_UB,                       ///< Internal use only
    APP_PARAM_FOC_U1,                       ///< Internal use only
    APP_PARAM_FOC_U2,                       ///< Internal use only
    APP_PARAM_FOC_U3,                       ///< Internal use only
    APP_PARAM_FOC_PWM1,                     ///< Internal use only
    APP_PARAM_FOC_PWM2,                     ///< Internal use only
    APP_PARAM_FOC_PWM3,                     ///< Internal use only
    APP_PARAM_FOC_TIMER_TOP,                ///< Internal use only
    APP_PARAM_DUTY,                         ///< Internal use only
    APP_PARAM_SIZE,                         ///< Use when you need to define the total parameter array size
} rr_servo_param_t;

/**
 * @brief Network management (NMT) states
 * 
 * @image html "nmt_states.png" 
 * @image latex "nmt_states.png" 
 * 
 */
typedef enum
{
    RR_NMT_INITIALIZING = 0,      /**< Device is initializing */
    RR_NMT_BOOT = 2,              /**< Device is executing a bootloader application */
    RR_NMT_PRE_OPERATIONAL = 127, /**< Device is in the pre-operational state */
    RR_NMT_OPERATIONAL = 5,       /**< Device is in the operational state */
    RR_NMT_STOPPED = 4,           /**< Device is stopped */
    RR_NMT_HB_TIMEOUT = -1,       /**< Device heartbeat timeout (device disappeared from the bus)*/
} rr_nmt_state_t;

/**
 * @brief Device information source instance
 * 
 */
typedef struct
{
    float value;       ///< Source value
    uint8_t activated; ///< Source activation flag
} param_cache_entry_t;

/**
 * @brief Device instance structure
 * 
 */
typedef struct
{
    void *dev;                                  ///< Device internals
    param_cache_entry_t pcache[APP_PARAM_SIZE]; ///< Device source cells
} rr_servo_t;

/**
 * @brief Interface instance structure
 * 
 */
typedef struct
{
    void *iface;   ///< Interface internals
    void *nmt_cb;  ///< NMT callback pointer
    void *emcy_cb; ///< EMCY callback pointer
} rr_can_interface_t;

/**
 * @brief Type of the intiated network management (NMT) callback<br>
 * @param interface Descriptor of the interface (see ::rr_init_interface) where the NMT event occured
 * @param servo_id Descriptor of the servo (see ::rr_init_servo) where the NMT event occured
 * @param nmt_state Network management state (::rr_nmt_state_t) that the servo entered
 * 
 */
typedef void (*rr_nmt_cb_t)(rr_can_interface_t *interface, int servo_id, rr_nmt_state_t nmt_state);

/**
 * @brief Type of the intiated emergency (EMCY) callback<br>
 * @param interface Descriptor of the interface (see ::rr_init_interface) where the EMCY event occured
 * @param servo_id Descriptor of the servo (see ::rr_init_servo) where the EMCY event occured
 * @param code Error code
 * @param reg Register field of the EMCY message (see CanOpen documentation)
 * @param bits Bits field of the EMCY message (see CanOpen documentation)
 * @param info Additional field (see CanOpen documentation)
 */
typedef void (*rr_emcy_cb_t)(rr_can_interface_t *interface, int servo_id, uint16_t code, uint8_t reg, uint8_t bits, uint32_t info);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* Exported C++ declarations ------------------------------------------------ */

/* Ref: http://dev.rozum.com/rozum-java/leonardo/blob/develop/devices/motor/cyber-api/src/main/java/com/rozum/cyber/api/protocol/prt3/CyberProtocol3.java */

void rr_sleep_ms(int ms);

void rr_set_debug_log_stream(FILE *f);
void rr_set_comm_log_stream(const rr_can_interface_t *interface, FILE *f);
void rr_setup_nmt_callback(rr_can_interface_t *interface, rr_nmt_cb_t cb);
void rr_setup_emcy_callback(rr_can_interface_t *interface, rr_emcy_cb_t cb);
const char *rr_describe_nmt(rr_nmt_state_t state);
const char *rr_describe_emcy_code(uint16_t code);
const char *rr_describe_emcy_bit(uint8_t bit);

rr_can_interface_t *rr_init_interface(const char *interface_name);
rr_ret_status_t rr_deinit_interface(rr_can_interface_t **interface);
rr_servo_t *rr_init_servo(rr_can_interface_t *interface, const uint8_t id);
rr_ret_status_t rr_deinit_servo(rr_servo_t **servo);

rr_ret_status_t rr_servo_reboot(const rr_servo_t *servo);
rr_ret_status_t rr_servo_reset_communication(const rr_servo_t *servo);
rr_ret_status_t rr_servo_set_state_operational(const rr_servo_t *servo);
rr_ret_status_t rr_servo_set_state_pre_operational(const rr_servo_t *servo);
rr_ret_status_t rr_servo_set_state_stopped(const rr_servo_t *servo);

rr_ret_status_t rr_net_reboot(const rr_can_interface_t *interface);
rr_ret_status_t rr_net_reset_communication(const rr_can_interface_t *interface);
rr_ret_status_t rr_net_set_state_operational(const rr_can_interface_t *interface);
rr_ret_status_t rr_net_set_state_pre_operational(const rr_can_interface_t *interface);
rr_ret_status_t rr_net_set_state_stopped(const rr_can_interface_t *interface);

rr_ret_status_t rr_stop_and_release(const rr_servo_t *servo);
rr_ret_status_t rr_stop_and_freeze(const rr_servo_t *servo);

rr_ret_status_t rr_set_current(const rr_servo_t *servo, const float current_a);
rr_ret_status_t rr_set_velocity(const rr_servo_t *servo, const float velocity_deg_per_sec);
rr_ret_status_t rr_set_position(const rr_servo_t *servo, const float position_deg);
rr_ret_status_t rr_set_velocity_with_limits(const rr_servo_t *servo, const float velocity_deg_per_sec, const float current_a);
rr_ret_status_t rr_set_position_with_limits(const rr_servo_t *servo, const float position_deg, const float velocity_deg_per_sec, const float current_a);
rr_ret_status_t rr_set_duty(const rr_servo_t *servo, float duty_percent);

rr_ret_status_t rr_add_motion_point(const rr_servo_t *servo, const float position_deg, const float velocity_deg, const uint32_t time_ms);
rr_ret_status_t rr_start_motion(rr_can_interface_t *interface, uint32_t timestamp_ms);

rr_ret_status_t rr_read_error_status(const rr_servo_t *servo, uint32_t *const error_count, uint8_t *const error_array);

rr_ret_status_t rr_param_cache_update(rr_servo_t *servo);
rr_ret_status_t rr_param_cache_setup_entry(rr_servo_t *servo, const rr_servo_param_t param, bool enabled);

rr_ret_status_t rr_read_parameter(rr_servo_t *servo, const rr_servo_param_t param, float *value);
rr_ret_status_t rr_read_cached_parameter(rr_servo_t *servo, const rr_servo_param_t param, float *value);

rr_ret_status_t rr_clear_points_all(const rr_servo_t *servo);
rr_ret_status_t rr_clear_points(const rr_servo_t *servo, const uint32_t num_to_clear);

rr_ret_status_t rr_get_points_size(const rr_servo_t *servo, uint32_t *num);
rr_ret_status_t rr_get_points_free_space(const rr_servo_t *servo, uint32_t *num);

rr_ret_status_t rr_invoke_time_calculation(const rr_servo_t *servo,
                                           const float start_position_deg, const float start_velocity_deg, const float start_acceleration_deg_per_sec2, const uint32_t start_time_ms,
                                           const float end_position_deg, const float end_velocity_deg, const float end_acceleration_deg_per_sec2, const uint32_t end_time_ms,
                                           uint32_t *time_ms);

rr_ret_status_t rr_set_zero_position(const rr_servo_t *servo, const float position_deg);
rr_ret_status_t rr_set_zero_position_and_save(const rr_servo_t *servo, const float position_deg);

rr_ret_status_t rr_get_max_velocity(const rr_servo_t *servo, float *velocity_deg_per_sec);
rr_ret_status_t rr_set_max_velocity(const rr_servo_t *servo, const float max_velocity_deg_per_sec);

rr_ret_status_t rr_change_id_and_save(rr_can_interface_t *interface, rr_servo_t *servo, uint8_t new_can_id);

#endif /* _ROZUM_API_H */
