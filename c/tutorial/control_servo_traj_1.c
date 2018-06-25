/**
 * @brief The tutorial describes how to set two PVT (point, velocity, time) points for one servo and
 * to execute the resulting motion trajectory.
 * @file control_servo_traj_1.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * @brief The tutorial describes how to set two PVT (point, velocity, time) points for one servo and
 * to execute the resulting motion trajectory.<br>
 * The general sequence of actions is as follows:
 * 1. Open the USB port where your CAN-USB dongle is connected using the ::rr_init_interface function.
 * 2. Initiate the servo for which you want to set a motion trajectory using the ::rr_init_servo function.
 * 3. Call the ::rr_clear_points_all function to clear the existing motion queue.
 * 4. Call the ::rr_add_motion_point to set the first PVT point, commanding the motor to move to the position of 100 degrees in 6 seconds.
 * The function parameters will be as follows:
 * servo: servo descriptor returned by the ::rr_init_servo function.
 * position_deg: 100.0 degrees
 * velocity_deg: 0.0 degrees
 * time_ms: 6000 milliseconds
 * When the point is added successfully to the motion queue, the function will return OK. When adding the point fails, the function returns an error warning and quits the program.
 * 5. Call the ::rr_add_motion_point to set the first PVT point, commanding the motor to move to the position of -100 degrees in 6 seconds.
 * The function parameters will be as follows:
 * servo: servo descriptor returned by the ::rr_init_servo function
 * position_deg: -100.0 degrees
 * velocity_deg: 0.0 degrees
 * time_ms: 6000 milliseconds
 * 6. Call the ::rr_start_motion to execute the motion queue comprising the two points added in Step 4 and Step 5.
 * 7. Call the ::rr_sleep_ms function to set a latency period of 14 seconds (or 14000 milliseconds). Calling the function ensures that master Heartbeats will be still available and the servo will not stop.
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
