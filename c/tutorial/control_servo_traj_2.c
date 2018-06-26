/**
 * @brief Setting PVT points for two servos
 * 
 * @file control_servo_traj_2.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * \defgroup tutor_c_servomove2 Setting PVT points for two servos
 * 
 * 1.Initialize the interface.
 * 
 * \snippet control_servo_traj_2.c Adding the interface
 * 
 * 2.Initialize the first servo.
 * \snippet control_servo_traj_2.c Adding servo ID0
 * 
 * 3. Initialize the second servo.
 * \snippet control_servo_traj_2.c Adding servo ID1
 * 
 * 4. Clear the motion queue of servo 1.
 * \snippet control_servo_traj_2.c Clear points servo ID0
 * 
 * 5. Clear the motion queue of servo 2.
 * \snippet control_servo_traj_2.c Clear points servo ID1
 * 
 * <b> Adding PVT points to motion queues</b>
 * 
 * 6. Set the first PVT point for servo 1, commanding it to move to the position of 100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_2.c Add point1 servo ID0
 * 
 * 7. Set the first PVT point for servo 2, commanding it to move to the position of 100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_2.c Add point1 servo ID1
 * 
 * 8. Set the second PVT point for servo 1, commanding it to move to the position of -100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_2.c Add point2 servo ID0
 * 
 * 9. Set the second PVT point for servo 2, commanding it to move to the position of -100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_2.c Add point2 servo ID1
 * 
 * <b> Executing the motion queue</b>
 * 
 * 10. Command both motors to move through the preset PVT points. Make sure to set the function parameter to 0.
 * \snippet control_servo_traj_2.c Start motion
 * 
 * 11. To ensure the servo will keep on working and will be available for master heartbeats, set a latency period of 14,000 milliseconds.
 * \snippet control_servo_traj_2.c Sleep
 * 
 * <b> Complete tutorial code: </b>
 * \snippet control_servo_traj_2.c cccode 1
 */
int main(int argc, char *argv[])
{
    //! [cccode 1] 
    //! [Adding the interface]
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    //! [Adding the interface]
    //! [Adding servo ID0]
    rr_servo_t *servo1 = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    //! [Adding servo ID0]
    //! [Adding servo ID1]
    rr_servo_t *servo2 = rr_init_servo(iface, TUTORIAL_SERVO_1_ID);
    //! [Adding servo ID1]

    API_DEBUG("========== Tutorial of the %s ==========\n", "controlling two servos");

    //! [Clear points servo ID0]
    rr_clear_points_all(servo1);
    //! [Clear points servo ID0]
    //! [Clear points servo ID1]
    rr_clear_points_all(servo2);
    //! [Clear points servo ID1]

    //! [Add point1 servo ID0]
    int status = rr_add_motion_point(servo1, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add point1 servo ID0]
    //! [Add point1 servo ID1]
    status = rr_add_motion_point(servo2, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add point1 servo ID1]
    //! [Add point2 servo ID0]
    status = rr_add_motion_point(servo1, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add point2 servo ID0]
    //! [Add point2 servo ID1]
    status = rr_add_motion_point(servo2, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add point2 servo ID1]
    //! [Start motion]
    rr_start_motion(iface, 0);
    //! [Start motion]

    //! [Sleep]
    rr_sleep_ms(14000); //wait till the movement end
    //! [Sleep]
    //! [cccode 1]
}
