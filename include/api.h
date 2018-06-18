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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* Exported macro ------------------------------------------------------------*/
/**
 * @brief Standart debug 
 * 
 */
#define API_DEBUG(x, ...) fprintf(stderr, x, __VA_ARGS__)

/* Exported constants --------------------------------------------------------*/
/**
 * @brief Return codes of the API functions
 */
enum rr_ret_status_t
{
    RET_OK = 0,       ///< Status OK
    RET_ERROR,        ///< Generic error
    RET_BAD_INSTANCE, ///< Bad interface or servo instance (null)
    RET_BUSY,         ///< Device is busy
    RET_WRONG_TRAJ,   ///< Wrong trajectory
    RET_LOCKED,       ///< Device is locked
    RET_STOPPED,      ///< Device is in STOPPED state
    RET_TIMEOUT,      ///< Communication timeout
    RET_ZERO_SIZE,    ///< Zero size
    RET_SIZE_MISMATCH ///< Received & target size mismatch
};

/**
 * @brief Device parameter & source indexes
 * 
 */
typedef enum
{
    APP_PARAM_NULL = 0,
    APP_PARAM_POSITION,
    APP_PARAM_VELOCITY,
    APP_PARAM_POSITION_ROTOR,
    APP_PARAM_VELOCITY_ROTOR,
    APP_PARAM_POSITION_GEAR_360, //0x05
    APP_PARAM_POSITION_GEAR_EMULATED,

    APP_PARAM_CURRENT_INPUT,
    APP_PARAM_CURRENT_OUTPUT,
    APP_PARAM_VOLTAGE_INPUT,
    APP_PARAM_VOLTAGE_OUTPUT,
    APP_PARAM_CURRENT_PHASE,

    APP_PARAM_TEMPERATURE_ACTUATOR,
    APP_PARAM_TEMPERATURE_ELECTRONICS,
    APP_PARAM_TORQUE,

    APP_PARAM_ACCELERATION, //0x0f
    APP_PARAM_ACCELERATION_ROTOR,

    APP_PARAM_CURRENT_PHASE_1,
    APP_PARAM_CURRENT_PHASE_2,
    APP_PARAM_CURRENT_PHASE_3,
    APP_PARAM_CURRENT_RAW,
    APP_PARAM_CURRENT_RAW_2,
    APP_PARAM_CURRENT_RAW_3,
    APP_PARAM_ENCODER_MASTER_TRACK,
    APP_PARAM_ENCODER_NONIUS_TRACK,
    APP_PARAM_ENCODER_MOTOR_MASTER_TRACK,
    APP_PARAM_ENCODER_MOTOR_NONIUS_TRACK,
    APP_PARAM_TORQUE_ELECTRIC_CALC,
    APP_PARAM_CONTROLLER_VELOCITY_ERROR,
    APP_PARAM_CONTROLLER_VELOCITY_SETPOINT,
    APP_PARAM_CONTROLLER_VELOCITY_FEEDBACK,
    APP_PARAM_CONTROLLER_VELOCITY_OUTPUT, //0X1F
    APP_PARAM_CONTROLLER_POSITION_ERROR,
    APP_PARAM_CONTROLLER_POSITION_SETPOINT,
    APP_PARAM_CONTROLLER_POSITION_FEEDBACK,
    APP_PARAM_CONTROLLER_POSITION_OUTPUT,

    APP_PARAM_CONTROL_MODE,

    APP_PARAM_FOC_ANGLE,
    APP_PARAM_FOC_IA,
    APP_PARAM_FOC_IB,
    APP_PARAM_FOC_IQ_SET,
    APP_PARAM_FOC_ID_SET,
    APP_PARAM_FOC_IQ,
    APP_PARAM_FOC_ID,
    APP_PARAM_FOC_IQ_ERROR,
    APP_PARAM_FOC_ID_ERROR,
    APP_PARAM_FOC_UQ,
    APP_PARAM_FOC_UD, //0X2F
    APP_PARAM_FOC_UA,
    APP_PARAM_FOC_UB,
    APP_PARAM_FOC_U1,
    APP_PARAM_FOC_U2,
    APP_PARAM_FOC_U3,
    APP_PARAM_FOC_PWM1,
    APP_PARAM_FOC_PWM2,
    APP_PARAM_FOC_PWM3,
    APP_PARAM_FOC_TIMER_TOP, //0X38 56
    APP_PARAM_DUTY,

    APP_PARAM_CURRENT_PHASE_ABS, //58
    APP_PARAM_CURRENT_RMS_ABS,
    APP_PARAM_QUALITY,

    APP_PARAM_TEMP_CHARGER,
    APP_PARAM_TEMP_DISCHARGER,
    APP_PARAM_VOLTAGE_CAN_H,
    APP_PARAM_VOLTAGE_CAN_L,

    APP_PARAM_CURRENT_PHASE_AVG,
    APP_PARAM_CURRENT_RMS_AVG,

    APP_PARAM_CONTROLLER_VELOCITY_ERROR_AVG,
    APP_PARAM_CONTROLLER_POSITION_ERROR_AVG,

    APP_PARAM_STEP,

    APP_PARAM_CURRENT_PHASE_PEAK,
    APP_PARAM_CURRENT_RMS_PEAK,

    APP_PARAM_CONTROLLER_VELOCITY_ERROR_PEAK,
    APP_PARAM_CONTROLLER_POSITION_ERROR_PEAK,

    APP_PARAM_TEST_TIME,
    APP_PARAM_EFFICIENCY,

    APP_PARAM_POWER_IN,
    APP_PARAM_POWER_OUT,

    APP_PARAM_SIZE //...
} rr_servo_param_t;

/* Exported types ------------------------------------------------------------*/
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
    void *dev; ///< Device internals
    param_cache_entry_t pcache[APP_PARAM_SIZE]; ///< Device sources cells
} rr_servo_t;

/**
 * @brief Interface instance structure
 * 
 */
typedef struct
{
    void *iface; ///< Interface internals
} rr_can_interface_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* Exported C++ declarations ------------------------------------------------ */

/* Ref: http://dev.rozum.com/rozum-java/leonardo/blob/develop/devices/motor/cyber-api/src/main/java/com/rozum/cyber/api/protocol/prt3/CyberProtocol3.java */

void rr_sleep_ms(int ms);

void rr_set_debug_log_stream(FILE *f);
void rr_set_comm_log_stream(const rr_can_interface_t *interface, FILE *f);

rr_can_interface_t *rr_init_interface(const char *interface_name);
int rr_deinit_interface(rr_can_interface_t **interface);
rr_servo_t *rr_init_servo(rr_can_interface_t *interface, const uint8_t id);
int rr_deinit_servo(rr_servo_t **servo);

int rr_servo_reboot(const rr_servo_t *servo);
int rr_servo_reset_communication(const rr_servo_t *servo);
int rr_servo_set_state_operational(const rr_servo_t *servo);
int rr_servo_set_state_pre_operational(const rr_servo_t *servo);
int rr_servo_set_state_stopped(const rr_servo_t *servo);

int rr_net_reboot(const rr_can_interface_t *interface);
int rr_net_reset_communication(const rr_can_interface_t *interface);
int rr_net_set_state_operational(const rr_can_interface_t *interface);
int rr_net_set_state_pre_operational(const rr_can_interface_t *interface);
int rr_net_set_state_stopped(const rr_can_interface_t *interface);

int rr_stop_and_release(const rr_servo_t *servo);
int rr_stop_and_freeze(const rr_servo_t *servo);

int rr_set_current(const rr_servo_t *servo, const float current_a);
int rr_set_velocity(const rr_servo_t *servo, const float velocity_deg_per_sec);
int rr_set_position(const rr_servo_t *servo, const float position_deg);
int rr_set_velocity_with_limits(const rr_servo_t *servo, const float velocity_deg_per_sec, const float current_a);
int rr_set_position_with_limits(const rr_servo_t *servo, const float position_deg, const float velocity_deg_per_sec, const float current_a);
int rr_set_duty(const rr_servo_t *servo, float duty_percent);

int rr_add_motion_point(const rr_servo_t *servo, const float position_deg, const float velocity_deg, const uint32_t time_ms);
int rr_start_motion(rr_can_interface_t *interface, uint32_t timestamp_ms);

int rr_read_error_status(const rr_servo_t *servo, uint8_t *array, uint32_t *size);

int rr_param_cache_update(rr_servo_t *servo);
int rr_param_cache_setup_entry(rr_servo_t *servo, const rr_servo_param_t param, bool enabled);

int rr_read_parameter(rr_servo_t *servo, const rr_servo_param_t param, float *value);

int rr_clear_points_all(const rr_servo_t *servo);
int rr_clear_points(const rr_servo_t *servo, const uint32_t num_to_clear);

int rr_get_points_size(const rr_servo_t *servo, uint32_t *num);
int rr_get_points_free_space(const rr_servo_t *servo, uint32_t *num);

int rr_invoke_time_calculation(const rr_servo_t *servo,
                              const float start_position_deg, const float start_velocity_deg, const float start_acceleration_deg_per_sec2, const uint32_t start_time_ms,
                              const float end_position_deg, const float end_velocity_deg, const float end_acceleration_deg_per_sec2, const uint32_t end_time_ms);
int rr_get_time_calculation_result(const rr_servo_t *servo, uint32_t *time_ms);

int rr_set_zero_position(const rr_servo_t *servo, const float position_deg);
int rr_set_zero_position_and_save(const rr_servo_t *servo, const float position_deg);

int rr_get_max_velocity(const rr_servo_t *servo, float *velocity_deg_per_sec);
int rr_set_max_velocity(const rr_servo_t *servo, const float max_velocity_deg_per_sec);

#endif /* _ROZUM_API_H */
