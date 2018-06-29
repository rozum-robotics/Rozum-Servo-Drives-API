/**
 * @brief Tutorial example of calculating a PVT point 
 * 
 * @file read_servo_trajectory_time.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

/**
 * \defgroup tutor_c_calculate_point
 * This tutorial describes how you can calculate and read the minimum time that it will take the servo to reach the position of 100 degrees.
 * <b>Note:</b> Following the instructions in the tutorial, you can get the said travel time value without actually moving the servo.
 * 
 * 1. Initialize the interface.
 * \snippet read_servo_trajectory_time.c Adding interface 9
 * 
 * 2. Initialize the servo.
 * \snippet read_servo_trajectory_time.c Adding servo 9
 * 
 * <b> Calculating the time to reach the specified position </b>
 * 
 * 3. Calculate the time it will take the servo to reach the position of 100 degrees when the other parameters are set to 0.
 * The calculation result is the minumum time value.
 * \snippet read_servo_trajectory_time.c Time calculation
 * 
 * <b> Reading the calculation result </b>
 * 
 * 4. Create a variable where the function will return the calculation result.
 * \snippet read_servo_trajectory_time.c Travel time variable
 * 
 * 5. Read the calculation result.
 * \snippet read_servo_trajectory_time.c Get calculation result
 * 
 * <b> Complete tutorial code: </b>
 * \snippet read_servo_trajectory_time.c cccode 1
 */
 
int main(int argc, char *argv[])
{
    //! [cccode 1] 
    //! [Adding interface 9]
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    //! [Adding interface 9]
    //! [Adding servo 9]
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    //! [Adding servo 9]

    API_DEBUG("========== Tutorial of the %s ==========\n", "trajectory calculation");

    //! [Time calculation]
    int status = rr_invoke_time_calculation(servo, 0.0, 0.0, 0.0, 0, 100.0, 0.0, 0.0, 0);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation\n");
        return 1;
    }
    //! [Time calculation]
    //! [Travel time variable]
    uint32_t travel_time;
    //! [Travel time variable]
    //! [Get calculation result]
    rr_get_time_calculation_result(servo, &travel_time);
    //! [Get calculation result]
    API_DEBUG("\tCalculated travel time: %d ms.\n", travel_time);
     //! [cccode 1]
}
