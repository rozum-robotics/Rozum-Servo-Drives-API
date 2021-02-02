/**
 * @brief Setting PVT points for one servo
 * @file control_servo_traj_1.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include <stdlib.h>

/**
 * \defgroup tutor_c_servomove1 PVT trajectory for one servo
 * The tutorial describes how to set up and execute a motion trajectory for one servo. In this example, the motion trajectory comprises two PVT (position-velocity-time) points:
 * <ul><li>one PVT commanding the servo to move to the position of 100 degrees in 6,000 milliseconds</li>
 * <li>one PVT commanding the servo to move to the position of -100 degrees in 6,000 milliseconds</li></ul>
 * 
 * 1. Initialize the interface.
 * \snippet control_servo_traj_1.c Adding the interface1
 * 
 * 2. Initialize the servo.
 * \snippet control_servo_traj_1.c Adding the servo1
 * 
 * 3. Clear the motion queue.
 * \snippet control_servo_traj_1.c Clear points all1
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
 * \snippet control_servo_traj_1.c Start motion1
 * 
 * 8. To ensure the program will not move on to execute another operation, set an idle period of 14,000 milliseconds.
 * \snippet control_servo_traj_1.c Sleep1
 * 
 * <b> Complete tutorial code: </b>
 * \snippet control_servo_traj_1.c cccode 1
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
	//! [cccode 1] 
	//! [Adding the interface1]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}
	//! [Adding the interface1]

	//! [Adding the servo1]
	rr_servo_t *servo = rr_init_servo(iface, id);
	if(!servo)
	{
		API_DEBUG("Servo init error\n");
		return 1;
	}
	//! [Adding the servo1]

	//! [Switching to operational state]
	rr_servo_set_state_operational(servo);
	//! [Switching to operational state]

	API_DEBUG("========== Tutorial of the %s ==========\n", "controlling one servo");

	//! [Clear points all1]
	rr_clear_points_all(servo);
	//! [Clear points all1]
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
	//! [Start motion1]
	rr_start_motion(iface, 0);
	//! [Start motion1]
	//! [Sleep1]
	rr_sleep_ms(14000); // wait till the movement ends
	//! [Sleep1]
	//! [cccode 1]
}
