/**
 * @brief Tutorial example of one servo PVT move.
 * @file control_servo_traj_1.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * @brief Tutorial example of one servo PVT move.
 * 
 * @ingroup tutor_c_servomove1
 */
int main(int argc, char *argv[])
{
    /** @code{.c} 
    */
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);

    API_DEBUG("========== Tutorial of the %s ==========\n", "controlling one servo");

    rr_clear_points_all(servo);
    API_DEBUG("Appending points\n");
    int status = rr_add_motion_point(servo, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    status = rr_add_motion_point(servo, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    rr_start_motion(iface, 0);

    rr_sleep_ms(14000); // wait till the movement end
    /** @endcode */
}
