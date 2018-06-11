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
 * @return int 
 */
int api_initInterface(const char *interfaceName) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param id 
 * @return int 
 */
int api_initServo(const CanDevice_t *device, const uint8_t id) {}

/**
 * @brief Reboots device
 * 
 * @param device Device instance 
 * @return int 
 */
int api_reboot(const CanDevice_t *device) {}

/**
 * @brief Resets device communication
 * 
 * @param device Device instance 
 * @return int 
 */
int api_resetCommunication(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int 
 */
int api_setStateOperational(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int 
 */
int api_setStatePreOperational(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int 
 */
int api_setStateStopped(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int 
 */
int api_stopAndRelease(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int 
 */
int api_stopAndFreeze(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param currentA 
 * @return int 
 */
int api_setCurrent(const CanDevice_t *device, const float32_t currentA) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param velocityDegPerSec 
 * @return int 
 */
int api_setVelocity(const CanDevice_t *device, const float32_t velocityDegPerSec) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param positionDeg 
 * @return int 
 */
int api_setPosition(const CanDevice_t *device, const float32_t positionDeg) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param velocityDegPerSec 
 * @param currentA 
 * @return int 
 */
int api_setVelocityWithLimits(const CanDevice_t *device, const float32_t velocityDegPerSec, const float32_t currentA) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param positionDeg 
 * @param velocity 
 * @param currentA 
 * @return int 
 */
int api_setPositionWithLimits(const CanDevice_t *device, const float32_t positionDeg, const float32_t velocity, const float32_t currentA) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param dutyPercent 
 * @return int 
 */
int api_setDuty(CanDevice_t *device, float32_t dutyPercent) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param positionDeg 
 * @param velocityDeg 
 * @param timeMs 
 * @return int 
 */
int api_addMotionPoint(const CanDevice_t *device, const float32_t positionDeg, const float32_t velocityDeg, const uint32_t timeMs) {}

/**
 * @brief 
 * 
 * @param timestampMs 
 * @return int 
 */
int api_startMotion(uint32_t timestampMs) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int 
 */
int api_readErrorStatus(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param requests 
 * @return int 
 */
int api_writeArrayRequestMask(const CanDevice_t *device, const uint8_t *requests) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param requests 
 * @return int 
 */
int api_readArrayRequestMask(const CanDevice_t *device, uint8_t *requests) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param param 
 * @param value 
 * @return int 
 */
int api_readParameter(const CanDevice_t *device, const uint8_t param, const float32_t *value) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int 
 */
int api_clearPointsAll(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param numToClear 
 * @return int 
 */
int api_clearPoints(const CanDevice_t *device, const uint32_t numToClear) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param array 
 * @return int 
 */
int api_readArrayRequest(const CanDevice_t *device, float32_t *array) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param num 
 * @return int 
 */
int api_getPointsSize(CanDevice_t *device, uint32_t *num) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param num 
 * @return int 
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
 * @return int 
 */
int api_invokeTimeCalculation(const CanDevice_t *device,
                              const float32_t startPositionDeg, const float32_t startVelocityDeg, const float32_t startAccelerationDegPerSec2, const uint32_t startTimeMs,
                              const float32_t endPositionDeg, const float32_t endVelocityDeg, const float32_t endAccelerationDegPerSec2, const uint32_t endTimeMs) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param timeMs 
 * @return int 
 */
int api_getTimeCalculationResult(const CanDevice_t *device, uint32_t *timeMs) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param positionDeg 
 * @return int 
 */
int api_getZeroPosition(const CanDevice_t *device, float32_t *positionDeg) {} //???

/**
 * @brief 
 * 
 * @param device Device instance 
 * @return int 
 */
int api_setZeroPositionAndSave(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param positionDeg 
 * @return int 
 */
int api_setZeroPositionValue(const CanDevice_t *device, const float32_t positionDeg) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param velocityDegPerSec 
 * @return int 
 */
int api_getMaxVelocity(const CanDevice_t *device, float32_t *velocityDegPerSec) {}

/**
 * @brief 
 * 
 * @param device Device instance 
 * @param maxVelocityDegPerSec 
 * @return int 
 */
int api_setMaxVelocity(const CanDevice_t *device, const float maxVelocityDegPerSec) {}