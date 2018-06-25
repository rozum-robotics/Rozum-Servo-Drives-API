/**
 * @brief Tutorial example of reading servo max velocity
 * 
 * @file read_servo_max_velocity.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * @brief Tutorial example of reading servo max velocity
 * 
 * @ingroup tutor_c_get_max_velocity
 */
int main(int argc, char *argv[])
{
    /** @code{.c} 
    */
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading servo max velocity");
    float velocity;
    rr_get_max_velocity(servo, &velocity);

    API_DEBUG("\tMax velocity: %.3f Deg/sec\n", velocity);
    /** @endcode */
}
