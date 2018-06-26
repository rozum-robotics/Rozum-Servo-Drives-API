/**
 * @brief Tutorial example of reading the maximum velocity of the servo
 * 
 * @file read_servo_max_velocity.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * \defgroup tutor_c_get_max_velocity Reading the maximum velocity of the servo
 * This tutorial describes how you can read the maximum velocity at which the servo can move at a given moment. <b> Note</b>: The function will return the least of
 * the three limits: the servo motor specifications, the user-defined maximum velocity limit, and the value based on the input voltage.
 * 
 * 1. Initialize the interface.
 * \snippet read_servo_max_velocity.c Adding the interface
 * 
 * 2. Initialize the servo.
 * \snippet read_servo_max_velocity.c Adding the servo
 * 
 * <b> Reading the maximum servo velocity </b>
 * 
 * 3. Create an array where the function will read the maximum velocity value.
 * \snippet read_servo_max_velocity.c Velocity variable
 * 
 * 4. Read the current maximum velocity value.
 * \snippet read_servo_max_velocity.c Read max velocity
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
