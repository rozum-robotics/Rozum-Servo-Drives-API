/**
 * @brief Tutorial example of calculating a PVT point 
 * 
 * @file read_servo_trajectory_time.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include <stdlib.h>

/**
 * \defgroup tutor_c_calculate_point PVT point calculation
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
 * 3. Create a variable where the function will return the calculation result.
 * \snippet read_servo_trajectory_time.c Travel time variable
 * 
 * 4. Calculate the time it will take the servo to reach the position of 100 degrees when the other parameters are set to 0.
 * The calculation result is the minumum time value.
 * \snippet read_servo_trajectory_time.c Time calculation
 * 
 * <b> Complete tutorial code: </b>
 * \snippet read_servo_trajectory_time.c cccode 9
 */

int main(int argc, char *argv[])
{
	uint8_t id;

	if(argc == 3)
	{
		id = strtol(argv[2], NULL, 0);
	}
	else
	{
		API_DEBUG("Wrong format!\nUsage: %s interface id\n", argv[0]);
		return 1;
	}
	//! [cccode 9]
	//! [Adding interface 9]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}
	//! [Adding interface 9]
	//! [Adding servo 9]
	rr_servo_t *servo = rr_init_servo(iface, id);
	if(!servo)
	{
		API_DEBUG("Servo init error\n");
		return 1;
	}
	//! [Adding servo 9]

	API_DEBUG("========== Tutorial of the %s ==========\n", "trajectory calculation");

	//! [Travel time variable]
	uint32_t travel_time;
	//! [Travel time variable]

	//! [Time calculation]
	int status = rr_invoke_time_calculation(servo, 0.0, 0.0, 0.0, 0, 100.0, 0.0, 0.0, 0, &travel_time);
	if(status != RET_OK)
	{
		API_DEBUG("Error in the trjectory point calculation\n");
		return 1;
	}
	//! [Time calculation]

	API_DEBUG("\tCalculated travel time: %d ms.\n", travel_time);
	//! [cccode 9]
}
