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
#include "common/servo_api.h"
#include "math.h"

/* Exported types ------------------------------------------------------------*/
/**
 * @brief Device instance structure
 * 
 */
typedef struct
{
    uint8_t id;                   ///< Device CAN ID
    uint8_t nmtState;             ///< Device NMT state
    time_t lastHartbeatTimestamp; ///< Last timestamp of the device heartbeat
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

int api_setCurrent(const CanDevice_t *device, const float32_t currentA);
int api_setVelocity(const CanDevice_t *device, const float32_t velocityDegPerSec);
int api_setPosition(const CanDevice_t *device, const float32_t positionDeg);
int api_setVelocityWithLimits(const CanDevice_t *device, const float32_t velocityDegPerSec, const float32_t currentA);
int api_setPositionWithLimits(const CanDevice_t *device, const float32_t positionDeg, const float32_t velocityDegPerSec, const float32_t currentA);
int api_setDuty(CanDevice_t *device, float32_t dutyPercent);

int api_addMotionPoint(const CanDevice_t *device, const float32_t positionDeg, const float32_t velocityDeg, const uint32_t timeMs);
int api_startMotion(uint32_t timestampMs);

int api_readErrorStatus(const CanDevice_t *device);

int api_writeArrayRequestMask(const CanDevice_t *device, const uint8_t *requests);
int api_readArrayRequestMask(const CanDevice_t *device, uint8_t *requests);

int api_readParameter(const CanDevice_t *device, const uint8_t param, const float32_t *value);
int api_clearPointsAll(const CanDevice_t *device);
int api_clearPoints(const CanDevice_t *device, const uint32_t numToClear);
int api_readArrayRequest(const CanDevice_t *device, float32_t *array);

int api_getPointsSize(CanDevice_t *device, uint32_t *num);
int api_getPointsFreeSpace(CanDevice_t *device, uint32_t *num);

int api_invokeTimeCalculation(const CanDevice_t *device,
                              const float32_t startPositionDeg, const float32_t startVelocityDeg, const float32_t startAccelerationDegPerSec2, const uint32_t startTimeMs,
                              const float32_t endPositionDeg, const float32_t endVelocityDeg, const float32_t endAccelerationDegPerSec2, const uint32_t endTimeMs);
int api_getTimeCalculationResult(const CanDevice_t *device, uint32_t *timeMs);

int api_getZeroPosition(const CanDevice_t *device, float32_t *positionDeg); //???
int api_setZeroPositionAndSave(const CanDevice_t *device);

int api_getMaxVelocity(const CanDevice_t *device, float32_t *velocityDegPerSec);
int api_setMaxVelocity(const CanDevice_t *device, const float maxVelocityDegPerSec);

#endif /* _ROZUM_API_H */
