#include "math.h"
#include "api.h"
#include <unistd.h>

#define SERVO_ID 123

int main(int argc, char *argv[])
{
    rr_can_interface_t *iface = rr_init_interface("192.168.0.124");
    rr_servo_t *d = rr_init_servo(iface, SERVO_ID);

    rr_sleep_ms(1000);

    /* rr_setVelocityrr_setVelocity */
    rr_set_velocity(d, 10.0);

    /* rr_invokeTimeCalculation + rr_getTimeCalculationResult*/
    uint32_t time;
    rr_invoke_time_calculation(d,
                             0.0, 0.0, 0.0, 0,
                             100.0, 0.0, 0.0, 0,
                             &time);
    API_DEBUG("Time is: %d\n", time);

    /* rr_readErrorStatus */
    uint8_t array[200];
    uint32_t size = 200;
    rr_read_error_status(d, array, &size);
    API_DEBUG("Array size: %d\n", size);
    for(int i = 0; i < size; i++)
    {
        API_DEBUG("\tError: %d\n", array[i]);
    }

    rr_sleep_ms(2000);

    /* rr_getMaxVelocity */
    float value;
    rr_get_max_velocity(d, &value);
    API_DEBUG("Velocity: %.3f\n", value);

    /* rr_stopAndFreeze */
    rr_stop_and_release(d);
    rr_sleep_ms(600);

    /* rr_setDuty */
    rr_set_duty(d, 0.2);
    rr_sleep_ms(1000);

    rr_set_duty(d, -0.2);
    rr_sleep_ms(1000);

    /* rr_setPosition */
    rr_set_position(d, 0.0);

    do
    {
        rr_read_parameter(d, APP_PARAM_POSITION, &value);
    } while(fabsf(value) > 0.1);
    API_DEBUG("Position: %.3f\n", value);

    /* rr_stopAndRelease */
    rr_stop_and_freeze(d);

    /* rr_setVelocityWithLimits */
    rr_set_velocity_with_limits(d, 30.0, 2.0);
    rr_sleep_ms(1000);

    /* rr_setPositionWithLimits */
    rr_set_position_with_limits(d, 0.0, 10.0, 10.0);
    do
    {
        rr_read_parameter(d, APP_PARAM_POSITION, &value);
        API_DEBUG("Position: %.3f\n", value);
    } while(fabsf(value) > 0.1);
    API_DEBUG("Position: %.3f\n", value);

    return 0;
}
