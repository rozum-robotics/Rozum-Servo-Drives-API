/* Includes ------------------------------------------------------------------*/
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
 * @param device 
 * @param id 
 * @return int 
 */
int api_initServo(const CanDevice_t *device, const uint8_t id) {}

/**
 * @brief 
 * 
 * @param device 
 * @return int 
 */
int api_reboot(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @return int 
 */
int api_rebootAll(void) {}

/**
 * @brief 
 * 
 * @param device 
 * @return int 
 */
int api_setStateOperational(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @return int 
 */
int api_setStateOperationalAll(void) {}

/**
 * @brief 
 * 
 * @param device 
 * @return int 
 */
int api_setStatePreOperational(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @return int 
 */
int api_setStatePreOperationalAll(void) {}

/**
 * @brief 
 * 
 * @param device 
 * @return int 
 */
int api_setStateStopped(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @return int 
 */
int api_setStateStoppedAll(void) {}

/**
 * @brief 
 * 
 * @param device 
 * @return int 
 */
int api_stopAndRelease(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device 
 * @return int 
 */
int api_stopAndFreeze(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device 
 * @param currentA 
 * @return int 
 */
int api_setCurrent(const CanDevice_t *device, const float32_t currentA) {}

/**
 * @brief 
 * 
 * @param device 
 * @param velocityDegPerSec 
 * @return int 
 */
int api_setVelocity(const CanDevice_t *device, const float32_t velocityDegPerSec) {}

/**
 * @brief 
 * 
 * @param device 
 * @param positionDeg 
 * @return int 
 */
int api_setPosition(const CanDevice_t *device, const float32_t positionDeg) {}

/**
 * @brief 
 * 
 * @param device 
 * @param velocityDegPerSec 
 * @param currentA 
 * @return int 
 */
int api_setVelocityWithLimits(const CanDevice_t *device, const float32_t velocityDegPerSec, const float32_t currentA) {}

/**
 * @brief 
 * 
 * @param device 
 * @param positionDeg 
 * @param velocity 
 * @param currentA 
 * @return int 
 */
int api_setPositionWithLimits(const CanDevice_t *device, const float32_t positionDeg, const float32_t velocity, const float32_t currentA) {}

/**
 * @brief 
 * 
 * @param device 
 * @param dutyPercent 
 * @return int 
 */
int api_setDuty(CanDevice_t *device, float32_t dutyPercent) {}

/**
 * @brief 
 * 
 * @param device 
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
 * @param device 
 * @return int 
 */
int api_readErrorStatus(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device 
 * @param requests 
 * @return int 
 */
int api_writeArrayRequestMask(const CanDevice_t *device, const uint8_t *requests) {}

/**
 * @brief 
 * 
 * @param device 
 * @param requests 
 * @return int 
 */
int api_readArrayRequestMask(const CanDevice_t *device, uint8_t *requests) {}

/**
 * @brief 
 * 
 * @param device 
 * @param param 
 * @param value 
 * @return int 
 */
int api_readParameter(const CanDevice_t *device, const uint8_t param, const float32_t *value) {}

/**
 * @brief 
 * 
 * @param device 
 * @return int 
 */
int api_clearPointsAll(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device 
 * @param numToClear 
 * @return int 
 */
int api_clearPoints(const CanDevice_t *device, const uint32_t numToClear) {}

/**
 * @brief 
 * 
 * @param device 
 * @param array 
 * @return int 
 */
int api_readArrayRequest(const CanDevice_t *device, float32_t *array) {}

/**
 * @brief 
 * 
 * @param device 
 * @param num 
 * @return int 
 */
int api_getPointsSize(CanDevice_t *device, uint32_t *num) {}

/**
 * @brief 
 * 
 * @param device 
 * @param num 
 * @return int 
 */
int api_getPointsFreeSpace(CanDevice_t *device, uint32_t *num) {}

/**
 * @brief 
 * 
 * @param device 
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
 * @param device 
 * @param timeMs 
 * @return int 
 */
int api_getTimeCalculationResult(const CanDevice_t *device, uint32_t *timeMs) {}

/**
 * @brief 
 * 
 * @param device 
 * @param positionDeg 
 * @return int 
 */
int api_getZeroPosition(const CanDevice_t *device, float32_t *positionDeg) {} //???

/**
 * @brief 
 * 
 * @param device 
 * @return int 
 */
int api_setZeroPositionAndSave(const CanDevice_t *device) {}

/**
 * @brief 
 * 
 * @param device 
 * @param positionDeg 
 * @return int 
 */
int api_setZeroPositionValue(const CanDevice_t *device, const float32_t positionDeg) {}

/**
 * @brief 
 * 
 * @param device 
 * @param velocityDegPerSec 
 * @return int 
 */
int api_getMaxVelocity(const CanDevice_t *device, float32_t *velocityDegPerSec) {}

/**
 * @brief 
 * 
 * @param device 
 * @param maxVelocityDegPerSec 
 * @return int 
 */
int api_setMaxVelocity(const CanDevice_t *device, const float maxVelocityDegPerSec) {}