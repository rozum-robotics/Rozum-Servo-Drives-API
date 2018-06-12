#include "api.h"
#include "math.h"
#include <unistd.h>

#define SERVO_ID 123

int main(int argc, char *argv[])
{
    CanInterface_t iface;
    CanDevice_t d;

    iface = api_initInterface("/dev/ttyS3");
    d = api_initDevice(&iface, SERVO_ID);

    api_sleepMs(1000);

    /* api_setVelocityapi_setVelocity */
    api_setVelocity(&d, 10.0);

    /* api_invokeTimeCalculation + api_getTimeCalculationResult*/
    api_invokeTimeCalculation(&d,
                              0.0, 0.0, 0.0, 0,
                              100.0, 0.0, 0.0, 0);

    uint32_t time;
    api_getTimeCalculationResult(&d, &time);
    API_DEBUG("Time is: %d", time);

    /* api_readErrorStatus */
    uint8_t array[200];
    uint32_t size = 200;
    api_readErrorStatus(&d, array, &size);
    API_DEBUG("Array size: %d\n", size);
    for(int i = 0; i < size; i++)
    {
        API_DEBUG("\tError: %d\n", array[i]);
    }

    api_sleepMs(2000);

    /* api_getMaxVelocity */
    float value;
    api_getMaxVelocity(&d, &value);
    API_DEBUG("Velocity: %.3f", value);

    /* api_stopAndFreeze */
    api_stopAndRelease(&d);
    api_sleepMs(600);

    /* api_setDuty */
    api_setDuty(&d, 0.2);
    api_sleepMs(1000);

    api_setDuty(&d, -0.2);
    api_sleepMs(1000);

    /* api_setPosition */
    api_setPosition(&d, 0.0);

    do
    {
        api_readParameter(&d, APP_PARAM_POSITION, &value);
    } while(fabsf(value) > 0.1);
    API_DEBUG("Position: %.3f", value);

    /* api_stopAndRelease */
    api_stopAndFreeze(&d);

    /* api_setVelocityWithLimits */
    api_setVelocityWithLimits(&d, 30.0, 2.0);
    api_sleepMs(1000);

    /* api_setPositionWithLimits */
    api_setPositionWithLimits(&d, 0.0, 10.0, 10.0);
    do
    {
        api_readParameter(&d, APP_PARAM_POSITION, &value);
        API_DEBUG("Position: %.3f", value);
    } while(fabsf(value) > 0.1);
    API_DEBUG("Position: %.3f", value);

    return 0;
}
