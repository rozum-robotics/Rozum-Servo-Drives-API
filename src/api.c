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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
 * @brief 
 * 
 * @param interfaceName 
 * @return int Status code (::RetStatus_t)
 * @ingroup Common
 */
int api_initInterface(const char *interfaceName) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param id 
 * @return int Status code (::RetStatus_t)
 * @ingroup Common
 */
int api_initServo(const CanDevice_t *device, const uint8_t id) {}

/**
 * @brief Reboots device
 * 
 * @param device Device instance 
 * @return int Status code (::RetStatus_t)
 * @ingroup System_control
 */
int api_reboot(const CanDevice_t *device) {}

/**
 * @brief Resets device communication
 * 
 * @param device Device instance 
 * @return int Status code (::RetStatus_t)
 * @ingroup System_control
 */
int api_resetCommunication(const CanDevice_t *device) {}

/**
 * @brief Sets device/s to the operational state  
 * 
 * @param device Device instance 
 *  If device == 0 > all devices on the bus will be set to the operational state
 * @return int Status code (::RetStatus_t)
 * @ingroup System_control
 */
int api_setStateOperational(const CanDevice_t *device) {}

/**
 * @brief Sets device/s to the pre-operational state  
 * 
 * @param device Device instance 
 * @return int Status code (::RetStatus_t)
 * @ingroup System_control
 */
int api_setStatePreOperational(const CanDevice_t *device) {}

/**
 * @brief Sets device/s to the stopped state  
 * 
 * @param device Device instance 
 * @return int Status code (::RetStatus_t)
 * @ingroup System_control
 */
int api_setStateStopped(const CanDevice_t *device) {}

/**
 * @brief Stops the device and releases it
 * 
 * @param device Device instance 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_stopAndRelease(const CanDevice_t *device) {}

/**
 * @brief Stops the device and make it hold current position
 * 
 * @param device Device instance 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_stopAndFreeze(const CanDevice_t *device) {}

/**
 * @brief Sets device contol current
 * 
 * @param device Device instance 
 * @param currentA Phase current in Amperes
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_setCurrent(const CanDevice_t *device, const float32_t currentA) {}

/**
 * @brief Sets device contol velocity
 * 
 * @param device Device instance 
 * @param velocityDegPerSec Flange velocity in degrees/sec
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_setVelocity(const CanDevice_t *device, const float32_t velocityDegPerSec) {}

/**
 * @brief Sets device contol position with maximum current
 * 
 * @param device Device instance 
 * @param positionDeg Flange position in degrees
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_setPosition(const CanDevice_t *device, const float32_t positionDeg) {}

/**
 * @brief Sets device contol velocity with limited control phase current
 * 
 * @param device Device instance 
 * @param velocityDegPerSec Flange velocity in degrees/sec
 * @param currentA Phase current limit in Amperes
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_setVelocityWithLimits(const CanDevice_t *device, const float32_t velocityDegPerSec, const float32_t currentA) {}

/**
 * @brief Sets device contol position with 
 * limited control flange velocity and
 * limited control phase current
 * 
 * @param device Device instance 
 * @param positionDeg Flange position in degrees
 * @param velocityDegPerSec Flange velocity in degrees/sec
 * @param currentA Phase current limit in Amperes
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_setPositionWithLimits(const CanDevice_t *device, const float32_t positionDeg, const float32_t velocityDegPerSec, const float32_t currentA) {}

/**
 * @brief Sets device control duty.
 * Duty is the part of the input voltage that is passed to the motor to spin it
 * 
 * @param device Device instance 
 * @param dutyPercent 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_setDuty(CanDevice_t *device, float32_t dutyPercent) {}

/**
 * @brief Adds motion PVT point to the device queue
 * 
 * @param device Device instance 
 * @param positionDeg Flange position in degrees
 * @param velocityDeg Flange velocity in degrees/sec
 * @param timeMs Relative point time in milliseconds
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_addMotionPoint(const CanDevice_t *device, const float32_t positionDeg, const float32_t velocityDeg, const uint32_t timeMs) {}

/**
 * @brief Starts the device movement by the spline points.
 * This is a broadcast command.
 * Note: if any device is not completed the movement (by spline points) it will send 
 * broadcast "Goto Stopped State" command to the all devices on the bus
 * 
 * @param timestampMs Startup delay in milliseconds. 
 * Default: 0
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_startMotion(uint32_t timestampMs) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_readErrorStatus(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param requests 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_writeArrayRequestMask(const CanDevice_t *device, const uint8_t *requests) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param requests 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_readArrayRequestMask(const CanDevice_t *device, uint8_t *requests) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param param 
 * @param value 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_readParameter(const CanDevice_t *device, const uint8_t param, const float32_t *value) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_clearPointsAll(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param numToClear 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_control
 */
int api_clearPoints(const CanDevice_t *device, const uint32_t numToClear) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param array 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_readArrayRequest(const CanDevice_t *device, float32_t *array) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param num 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_getPointsSize(CanDevice_t *device, uint32_t *num) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param num 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_getPointsFreeSpace(CanDevice_t *device, uint32_t *num) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param startPositionDeg 
 * @param startVelocityDeg 
 * @param startAccelerationDegPerSec2 
 * @param startTimeMs 
 * @param endPositionDeg 
 * @param endVelocityDeg 
 * @param endAccelerationDegPerSec2 
 * @param endTimeMs 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_invokeTimeCalculation(const CanDevice_t *device,
                              const float32_t startPositionDeg, const float32_t startVelocityDeg, const float32_t startAccelerationDegPerSec2, const uint32_t startTimeMs,
                              const float32_t endPositionDeg, const float32_t endVelocityDeg, const float32_t endAccelerationDegPerSec2, const uint32_t endTimeMs) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param timeMs 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_getTimeCalculationResult(const CanDevice_t *device, uint32_t *timeMs) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param positionDeg 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_config
 */
int api_getZeroPosition(const CanDevice_t *device, float32_t *positionDeg) {} //???

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_config
 */
int api_setZeroPositionAndSave(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param velocityDegPerSec 
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_info
 */
int api_getMaxVelocity(const CanDevice_t *device, float32_t *velocityDegPerSec) {}

/**
 * @brief Sets the global limit for the device velocity.
 * Note: this is power volatile command
 * 
 * @param device Device instance 
 * @param maxVelocityDegPerSec Flange velocity in degrees/sec
 * @return int Status code (::RetStatus_t)
 * @ingroup Servo_config
 */
int api_setMaxVelocity(const CanDevice_t *device, const float maxVelocityDegPerSec) {}
