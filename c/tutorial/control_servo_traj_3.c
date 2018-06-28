/**
 * @brief Setting points for three servos
 * 
 * @file control_servo_traj_3.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * \defgroup tutor_c_servomove3 Setting PVT points for three servos
 * The tutorial describes how to set up motion trajectories for three servos and to execute them simultaneously. In this example, each motion trajectory comprises two PVT (position-time-velocity) points:
 * <ul><li>one PVT commanding servos to move to the position of 100 degrees in 6,000 milliseconds</li>
 * <li>one PVT commanding servos to move to the position of -100 degrees in 6,000 milliseconds</li></ul>

 * 1. Initialize the interface.
 * \snippet control_servo_traj_3.c Adding the interface
 * 
 * 2. Initialize servo 1.
 * \snippet control_servo_traj_3.c Adding servo ID0
 * 
 * 3. Initialize servo 2.
 * \snippet control_servo_traj_3.c Adding servo ID1
 * 
 * 4. Initialize servo 3.
 * \snippet control_servo_traj_3.c Adding servo ID2
 * 
 * 5. Clear the motion queue of servo 1.
 * \snippet control_servo_traj_3.c Clear points servo ID0
 * 
 * 6. Clear the motion queue of servo 2.
 * \snippet control_servo_traj_3.c Clear points servo ID1
 * 
 * 7. Clear the motion queue of servo 3.
 * \snippet control_servo_traj_3.c Clear points servo ID2
 * 
 * <b> Adding PVT ponts to form motion queues </b>
 * 
 * 8. Set the first PVT point for servo 1, commanding it to move to the position of 100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_3.c Add motion point 1 servo ID0
 * 
 * 9. Set the first PVT point for servo 2, commanding it to move to the position of 100 degrees in 6,000 milliseconds.
 *  \snippet control_servo_traj_3.c Add motion point 1 servo ID1
 * 
 * 10. Set the first PVT point for servo 3, commanding it to move to the position of 100 degrees in 6,000 milliseconds.
 *  \snippet control_servo_traj_3.c Add motion point 1 servo ID2
 * 
 * 11.  Set the second PVT point for servo 1, commanding it to move to the position of -100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_3.c Add motion point 2 servo ID0
 * 
 * 12. Set the second PVT point for servo 2, commanding it to move to the position of -100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_3.c Add motion point 2 servo ID1
 * 
 * 13. Set the second PVT point for servo 3, commanding it to move to the position of -100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_3.c Add motion point 2 servo ID2
 * 
 * <b> Executing the resulting motion queues </b>
 * 
 * 14. Command all servos to start moving simulateneously. Each of the three servos will execute their own motion queues. Set the function parameter to 0 to get the servos moving without a delay.
 * \snippet control_servo_traj_3.c Start motion
 * 
 * 15. To ensure the program will not move on to execute another operation, set an idle period of 14,000 milliseconds.
 * \snippet control_servo_traj_1.c Sleep
 * 
 * <b> Complete tutorial code: </b>
 * \snippet control_servo_traj_1.c cccode 1
 */
int main(int argc, char *argv[])
{
    //! [cccode 1] 
    //! [Adding the interface3]
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    //! [Adding the interface3]
    //! [Adding servo first]
    rr_servo_t *servo1 = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    //! [Adding servo first]
    //! [Adding servo2]
    rr_servo_t *servo2 = rr_init_servo(iface, TUTORIAL_SERVO_1_ID);
    //! [Adding servo2]
    //! [Adding servo3]
    rr_servo_t *servo3 = rr_init_servo(iface, TUTORIAL_SERVO_2_ID);
    //! [Adding servo3]

    API_DEBUG("========== Tutorial of the %s ==========\n", "controlling three servos");

    //! [Clear points for servo1]
    rr_clear_points_all(servo1);
    //! [Clear points for servo1]
    //! [Clear points for servo2]
    rr_clear_points_all(servo2);
    //! [Clear points for servo2]
    //! [Clear points for servo3]
    rr_clear_points_all(servo3);
    //! [Clear points for servo3]
    //! [Add motion point one for servo1]
    int status = rr_add_motion_point(servo1, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add motion point one for servo1]
    //! [Add motion point two for servo2]
    status = rr_add_motion_point(servo2, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add motion point two for servo2]
    //! [Add motion point three for servo3]
    status = rr_add_motion_point(servo3, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add motion point 1 servo ID2]
    //! [Add motion point 2 servo ID0]
    status = rr_add_motion_point(servo1, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add motion point 2 servo ID0]
    //! [Add motion point 2 servo ID1]
    status = rr_add_motion_point(servo2, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add motion point 2 servo ID1]
    //! [Add motion point 2 servo ID2]
    status = rr_add_motion_point(servo3, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    //! [Add motion point 2 servo ID2]
    //! [Start motion]
    rr_start_motion(iface, 0);
    //! [Start motion]

    //! [Sleep]
    rr_sleep_ms(14000); // wait till the movement ends
    //! [Sleep]
    //! [cccode 1]
}
