/**
 * @brief Tutorial example of reading motion queue parameters
 * 
 * @file read_servo_motion_queue.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * \defgroup tutor_c_read_motion_queue Reading motion queue parameters
 * This tutorial describes how to determine the actual size of a motion queue. In this example, we will read the number of free 
 * and occupied PVT points in a motion queue before and after adding PVT (position-velocity-time) points to the motion queue.
 * <b>Note:</b> Currently, the maximum motion queue size is 100 PVT.
 * 
 * 1. Initialize the interface.
 * \snippet read_servo_motion_queue.c Adding interface 8
 * 
 * 2. Initialize the servo.
 * \snippet read_servo_motion_queue.c Adding servo 8
 * 
 * 3. Clear the motion queue of the servo.
 * \snippet read_servo_motion_queue.c Clear points 8
 * 
 * <b> Reading the initial motion queue size</b>
 * 
 * 4. Create a variable where the function will save the motion queue size values (free and occupied PVT points).
 * \snippet read_servo_motion_queue.c Points size variable
 * 
 * 5. Read how many PVT points have been already added to the motion queue.
 * \snippet read_servo_motion_queue.c Points size1
 * 
 * 6. Read how many more PVT points can be added to the motion queue.
 * \snippet read_servo_motion_queue.c Points free1
 * 
 * <b> Reading the motion queue size after adding new PVT points to the motion queue</b>
 * 
 * 7. Add PVT point 1 to the motion queue, setting the time parameter to 10000000 ms.
 * \snippet read_servo_motion_queue.c Add point1
 * 
 * 8. Add PVT point 2 to the motion queue, setting the time parameter to 10000000 ms.
 * \snippet read_servo_motion_queue.c Add point2
 * 
 * 9. Read how many PVT points are already in the motion queue.
 * \snippet read_servo_motion_queue.c Points size2
 * 
 * 10. Read how many more PVT points can be added to the motion queue.
 * \snippet read_servo_motion_queue.c  Points free2
 *
 * <b> Complete tutorial code: </b>
 * \snippet read_servo_motion_queue.c cccode 8
 */ 
 
int main(int argc, char *argv[])
{
    //! [cccode 8] 
    //! [Adding interface 8]
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    //! [Adding interface 8]
    //! [Adding servo 8]
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    //! [Adding servo 8]

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading motion queue parameters");

    API_DEBUG("Clearing points\n");
    //! [Clear points 8]
    rr_clear_points_all(servo);
    //! [Clear points 8]
    

    //! [Points size variable]
    uint32_t num;
    //! [Points size variable]
    //! [Points size1]
    rr_get_points_size(servo, &num);
    //! [Points size1]
    API_DEBUG("\tPoints in the queue before: %d\n", num);

    //! [Points free1]
    rr_get_points_free_space(servo, &num);
    //! [Points free1]
    API_DEBUG("\tPoints queue free size before: %d\n", num);

    API_DEBUG("Appending points\n");
    
    //! [Add point1]
    rr_add_motion_point(servo, 0.0, 0.0, 10000000);
    //! [Add point1]
    //! [Add point2]
    rr_add_motion_point(servo, 0.0, 0.0, 10000000);
    //! [Add point2]

    //! [Points size2]
    rr_get_points_size(servo, &num);
    //! [Points size2]
    API_DEBUG("\tPoints in the queue after: %d\n", num);

    //! [Points free2]
    rr_get_points_free_space(servo, &num);
    //! [Points free2]
    API_DEBUG("\tPoints queue free size after: %d\n", num);
    //! [cccode 8]
}
