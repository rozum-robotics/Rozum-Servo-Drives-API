/**
 * @brief Setting positions with limits
 * @file time_optimal_movement.c
 * @author Rozum
 * @date 2018-12-05
 */

#include "api.h"
#include <stdlib.h>

/**
 * \defgroup tutor_c_time_optimal_movement Setting position with limits
 * The tutorial demonstrates how to use the ::rr_set_position_with_limits function.
 *
 * 1. Read the parameters as required to run the tutorial.
 * \snippet time_optimal_movement.c Read param tutorial2
 *
 * 2. Initiate the interface.
 * \snippet time_optimal_movement.c Init interface35
 *
 * 3. Initiate the servo. 
 * \snippet time_optimal_movement.c Init servo35
 *
 * 4. Set the servo to the operational state.
 * \snippet time_optimal_movement.c Switching to operational2
 *
 * 5. Clear the existing motion queue.
 * \snippet time_optimal_movement.c Clear points all
 *
 * 6. Run the ::rr_set_position_with_limits function, setting its parameters to the values read at Step 1.
 * \snippet time_optimal_movement.c Set position with limits
 *
 * 7. Set a variable to save data about how long the servo completes the preset motion.
 * \snippet time_optimal_movement.c Set variable to save time
 *
 * 8. Set the rr_sleep_ms function to the time as requred to keep the program running until the servo completes the preset motion.
 * \snippet time_optimal_movement.c Set sleep time
 *
 *<b> Complete tutorial code: </b>
 * \snippet time_optimal_movement.c time_optimal_movement_code_full
 */

//! [time_optimal_movement_code_full]
//! [Read param tutorial2]
int main(int argc, char *argv[])
{
	uint8_t id;
	float ang, vel, acc;

	if(argc == 6)
	{
		id = strtol(argv[2], NULL, 0);
		ang = strtof(argv[3], NULL);
		vel = strtof(argv[4], NULL);
		acc = strtof(argv[5], NULL);
	}
	else
	{
		API_DEBUG("Wrong format!\nUsage: %s interface id angle max_vel max_acc\n", argv[0]);
		return 1;
	}
	//! [Read param tutorial2]
	//! [Init interface35]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}
	//! [Init interface35]
	//! [Init servo35]
	rr_servo_t *servo = rr_init_servo(iface, id);
	if(!servo)
	{
		API_DEBUG("Servo init error\n");
		return 1;
	}
	//! [Init servo35]

	//! [Switching to operational2]
	rr_servo_set_state_operational(servo);
	//! [Switching to operational2]

	//! [Clear points all]
	rr_clear_points_all(servo);
	//! [Clear points all]

	//! [Set position with limits]
	API_DEBUG("Appending points\n");
	uint32_t time_to_get_ms = 0;
	int status = rr_set_position_with_limits(servo, ang, vel, acc, &time_to_get_ms);
	if(status != RET_OK)
	{
		API_DEBUG("Error in point appending: %d\n", status);
		return 1;
	}
	//! [Set position with limits]

	//! [Set variable to save time]
	API_DEBUG("Time to get: %d ms\n", time_to_get_ms);
	//! [Set variable to save time]
	//! [Set sleep time]
	rr_sleep_ms(time_to_get_ms + 100); // wait till the movement ends
	//! [Set sleep time]
}
//! [time_optimal_movement_code_full]

