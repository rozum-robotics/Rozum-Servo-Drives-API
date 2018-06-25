/**
 * @brief Tutorial example of three servos PVT move
 * 
 * @file control_servo_traj_3.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * @brief Tutorial example of three servos PVT move
 * 
 * @ingroup tutor_c_servomove3
 */
int main(int argc, char *argv[])
{
    /** @code{.c} 
    */
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    rr_servo_t *servo1 = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    rr_servo_t *servo2 = rr_init_servo(iface, TUTORIAL_SERVO_1_ID);
    rr_servo_t *servo3 = rr_init_servo(iface, TUTORIAL_SERVO_2_ID);

    API_DEBUG("========== Tutorial of the %s ==========\n", "controlling three servos");

    rr_clear_points_all(servo1);
    rr_clear_points_all(servo2);
    rr_clear_points_all(servo3);

    int status = rr_add_motion_point(servo1, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    status = rr_add_motion_point(servo2, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    status = rr_add_motion_point(servo3, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    status = rr_add_motion_point(servo1, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    status = rr_add_motion_point(servo2, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    status = rr_add_motion_point(servo3, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    rr_start_motion(iface, 0);

    rr_sleep_ms(14000); // wait till tyhe movement end
    /** @endcode */
}
