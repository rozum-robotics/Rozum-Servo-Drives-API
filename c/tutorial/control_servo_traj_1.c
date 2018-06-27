/**
 * @brief Setting PVT points for one servo
 * @file control_servo_traj_1.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * \defgroup tutor_c_servomove1
 * The tutorial describes how to set up and execute a motion trajectory for one servo. In this example, the motion trajectory comprises two PVT (position-time-velocity) points:
 * <ul><li>one PVT commanding the servo to move to the position of 100 degrees in 6,000 milliseconds</li>
 * <li>one PVT commanding the servo to move to the position of -100 degrees in 6,000 milliseconds</li></ul>
 * 
 * 1. Initialize the interface.
 * \snippet control_servo_traj_1.c Adding the interface
 * 
 * 2. Initialize the servo.
 * \snippet control_servo_traj_1.c Adding the servo
 * 
 * 3. Clear the motion queue.
 * \snippet control_servo_traj_1.c Clear points all
 * 
 * <b> Adding PVT points to form a motion queue </b>
 * 
 * 4. Set the first PVT point, commanding the servo to move to the position of 100 degrees in 6,000 milliseconds. <b>Note</b>: When a point is added successfully to the motion queue, the function will return OK.
 * Otherwise, the function returns an error warning and quits the program.
 * \snippet control_servo_traj_1.c Add motion point first
 * 
 * 5. Set the second PVT point, commanding the servo to move to the position of -100 degrees in 6,000 milliseconds. <b>Note</b>: When a point is added successfully to the motion queue, the function will return OK.
 * Otherwise, the function returns an error warning and quits the program.
 * \snippet control_servo_traj_1.c Add motion point second
 * 
 * <b> Executing the resulting motion queue </b>
 * 
 * 7. Command the servo to move through the PVT points you added to the motion queue. Set the function parameter to 0 to get the servo moving without a delay.
 * \snippet control_servo_traj_1.c Start motion
 * 
 * 8. To ensure the servo will remain available for master heartbeat communication during motion, set a latency period of 14,000 milliseconds.
 * \snippet control_servo_traj_1.c Sleep
 * 
 * <b> Complete tutorial code: </b>
 * \snippet control_servo_traj_1.c cccode 1
 */
 
int main(int argc, char *argv[])
{
    //! [cccode 1] 
    //! [Adding the interface]
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    //! [Adding the interface]
    
    //! [Adding the servo]
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    //! [Adding the servo]

    API_DEBUG("========== Tutorial of the %s ==========\n", "controlling one servo");

    //! [Clear points all]
    rr_clear_points_all(servo);
    //! [Clear points all]
    API_DEBUG("Appending points\n");
    //! [Add motion point first]
    int status = rr_add_motion_point(servo, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add motion point first]
    //! [Add motion point second]
    status = rr_add_motion_point(servo, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add motion point second]
    //! [Start motion]
    rr_start_motion(iface, 0);
    //! [Start motion]
    //! [Sleep]
    rr_sleep_ms(14000); // wait till the movement ends
    //! [Sleep]
    //! [cccode 1]
}
