/**
 * @brief Tutorial example of reading the maximum servo
 * 
 * @file read_servo_max_velocity.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * \defgroup tutor_c_get_max_velocity Reading the maximum servo velocity
 * This tutorial describes how to read the maximum velocity at which the servo can move at the current moment. <b> Note: </b> The function will return the least of
 * the three limits: the servo motor specifications, the user-defined maximum velocity limit (see ::rr_set_velocity_with_limits),
 *  or the calculated value based on the input voltage.
 * 
 * 1. Initialize the interface.
 * \snippet read_servo_max_velocity.c Adding interface 7
 * 
 * 2. Initialize the servo.
 * \snippet read_servo_max_velocity.c Adding servo 7
 * 
 * <b> Reading the maximum servo velocity </b>
 * 
 * 3. Create a variable where the function will return the maximum servo velocity.
 * \snippet read_servo_max_velocity.c Velocity variable
 * 
 * 4. Read the current maximum velocity value.
 * \snippet read_servo_max_velocity.c Read max velocity
 * 
 * <b> Complete tutorial code: </b>
 * \snippet read_servo_max_velocity.c cccode 1
 */
int main(int argc, char *argv[])
{
    //! [cccode 1] 
    //! [Adding interface 7]
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    //! [Adding interface 7]
    //! [Adding servo 7]
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    //! [Adding servo 7]

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading servo max velocity");
   
    //! [Velocity variable]
    float velocity;
    //! [Velocity variable]
    //! [Read max velocity]
    rr_get_max_velocity(servo, &velocity);
    //! [Read max velocity]

    API_DEBUG("\tMax velocity: %.3f Deg/sec\n", velocity);
    //! [cccode 1]
}
