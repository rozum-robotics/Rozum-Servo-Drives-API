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
#include <stdio.h>
//#include "common/servo_api.h"
//#include "math.h"

#define API_DEBUG(x, ...) fprintf(stderr, x, __VA_ARGS__)

/* Exported types ------------------------------------------------------------*/
/**
 * @brief Device instance structure
 * 
 */
typedef struct
{
    uint8_t id;       ///< Device CAN ID
    uint8_t nmtState; ///< Device NMT state
    //    time_t lastHartbeatTimestamp; ///< Last timestamp of the device heartbeat
} CanDevice_t;

/* Exported constants --------------------------------------------------------*/
/**
 * @brief Return codes of the API functions
 * 
 */
enum RetStatus_t
{
    RET_OK = 0,     ///< Status OK
    RET_ERROR,      ///< Generic error
    RET_BUSY,       ///< Device is busy
    RET_WRONG_TRAJ, ///< Wrong trajectory
    RET_LOCKED,     ///< Device is locked
    RET_STOPPED,    ///< Device is in STOPPED state
    RET_TIMEOUT     ///< Communication timeout
};

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
} AppParam_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* Exported C++ declarations ------------------------------------------------ */

/* Ref: http://dev.rozum.com/rozum-java/leonardo/blob/develop/devices/motor/cyber-api/src/main/java/com/rozum/cyber/api/protocol/prt3/CyberProtocol3.java */

int api_initInterface(const char *interfaceName);
int api_initServo(const CanDevice_t *device, const uint8_t id);

int api_reboot(const CanDevice_t *device);
int api_resetCommunication(const CanDevice_t *device);
int api_setStateOperational(const CanDevice_t *device);
int api_setStatePreOperational(const CanDevice_t *device);
int api_setStateStopped(const CanDevice_t *device);

int api_stopAndRelease(const CanDevice_t *device);
int api_stopAndFreeze(const CanDevice_t *device);

int api_setCurrent(const CanDevice_t *device, const float currentA);
int api_setVelocity(const CanDevice_t *device, const float velocityDegPerSec);
int api_setPosition(const CanDevice_t *device, const float positionDeg);
int api_setVelocityWithLimits(const CanDevice_t *device, const float velocityDegPerSec, const float currentA);
int api_setPositionWithLimits(const CanDevice_t *device, const float positionDeg, const float velocityDegPerSec, const float currentA);
int api_setDuty(CanDevice_t *device, float dutyPercent);

int api_addMotionPoint(const CanDevice_t *device, const float positionDeg, const float velocityDeg, const uint32_t timeMs);
int api_startMotion(uint32_t timestampMs);

int api_readErrorStatus(const CanDevice_t *device);

int api_writeArrayRequestMask(const CanDevice_t *device, const uint8_t *requests);
int api_readArrayRequestMask(const CanDevice_t *device, uint8_t *requests);

int api_readParameter(const CanDevice_t *device, const uint8_t param, const float *value);
int api_clearPointsAll(const CanDevice_t *device);
int api_clearPoints(const CanDevice_t *device, const uint32_t numToClear);
int api_readArrayRequest(const CanDevice_t *device, float *array);

int api_getPointsSize(CanDevice_t *device, uint32_t *num);
int api_getPointsFreeSpace(CanDevice_t *device, uint32_t *num);

int api_invokeTimeCalculation(const CanDevice_t *device,
                              const float startPositionDeg, const float startVelocityDeg, const float startAccelerationDegPerSec2, const uint32_t startTimeMs,
                              const float endPositionDeg, const float endVelocityDeg, const float endAccelerationDegPerSec2, const uint32_t endTimeMs);
int api_getTimeCalculationResult(const CanDevice_t *device, uint32_t *timeMs);

int api_getZeroPosition(const CanDevice_t *device, float *positionDeg); //???
int api_setZeroPositionAndSave(const CanDevice_t *device);

int api_getMaxVelocity(const CanDevice_t *device, float *velocityDegPerSec);
int api_setMaxVelocity(const CanDevice_t *device, const float maxVelocityDegPerSec);

#endif /* _ROZUM_API_H */
