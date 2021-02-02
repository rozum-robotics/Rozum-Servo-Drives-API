/**
 * @brief Setting PVT points for two servos
 * 
 * @file control_servo_traj_2.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include <stdlib.h>

/**
 * \defgroup tutor_c_servomove2 PVT trajectory for two servos
 * The tutorial describes how to set up motion trajectories for two servos and to execute them simultaneously. In this example, each motion trajectory comprises two PVT (position-velocity-time) points:
 * <ul><li>one PVT commanding servos to move to the position of 100 degrees in 6,000 milliseconds</li>
 * <li>one PVT commanding servos to move to the position of -100 degrees in 6,000 milliseconds</li></ul>
 * 
 * <p><b>Important!</b>Before setting up PVT points, make sure to change the default CAN ID of at least one of the servos (see the \ref tutor_c_changeID1 tutorial).</p>
 * 
 *
 * <b>Note:</b> When you set a PVT trajectory to move more than one servo simultaneously, mind that the clock rate of the servos can differ by up to 2-3%.
 * Therefore, if the preset PVT trajectory is rather long, servos can get desynchronized. To avoid the desynchronization, we have implemented the following mechanism:
 * <ul><li>The device controlling the servos broadcasts a sync CAN frame to all servos on an interface. The frame should have the following format:<br>
 * ID = 0x27f, data = uint32 (4 bytes),<br>
 * <b>Where:</b> ‘data’ stands for the microseconds counter value by modulus of 600,000,000, starting from any value.</li>
 * <li>Servos receive the frame and try to adjust their clock rates to that of the device using the PLL.
 * The adjustment proper starts after the servos receive the second frame and can take up to 5 seconds, depending on the broadcasting frequency.
 * The higher the broadcasting frequency, the less time the adjustment takes.</li>
 * <li>The broadcasting frequency is 5 Hz minimum. The recommended frequency range is from 10 to 20 Hz.
 * When the sync frames are not broadcast or the broadcast frequency is below 5 Hz, the clock rate of servos is as usual.</li>


 * 1. Initialize the interface.
 * \snippet control_servo_traj_2.c Adding the interface2
 * 
 * 2. Initialize servo 1.
 * \snippet control_servo_traj_2.c Adding servo ID0
 * 
 * 3. Initialize servo 2.
 * \snippet control_servo_traj_2.c Adding servo ID1
 * 
 * 4. Clear the motion queue of servo 1.
 * \snippet control_servo_traj_2.c Clear points servo ID0
 * 
 * 5. Clear the motion queue of servo 2.
 * \snippet control_servo_traj_2.c Clear points servo ID1
 * 
 * <b> Adding PVT points to form motion queues</b>
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
 * <b> Executing the resulting motion queues</b>
 * 
 * 10. Command all servos to move simultaneously. Each of the two servos will execute their preset motion queues. Set the function parameter to 0 to get the servos moving without a delay.
 * \snippet control_servo_traj_2.c Start motion2
 * 
 * 11. To ensure the program will not move on to execute another operation, set an idle period of 14,000 milliseconds.
 * \snippet control_servo_traj_2.c Sleep2
 * 
 * <b> Complete tutorial code: </b>
 * \snippet control_servo_traj_2.c cccode 2
 */
int main(int argc, char *argv[])
{
	uint8_t id1, id2;

	if(argc == 4)
	{
		id1 = strtol(argv[2], NULL, 0);
		id2 = strtol(argv[3], NULL, 0);
	}
	else
	{
		API_DEBUG("Wrong format!\nUsage: %s interface id1 id2\n", argv[0]);
		return 1;
	}
	//! [cccode 2] 
	//! [Adding the interface2]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}
	//! [Adding the interface2]
	//! [Adding servo ID0]
	rr_servo_t *servo1 = rr_init_servo(iface, id1);
	if(!servo1)
	{
		API_DEBUG("Servo #1 init error\n");
		return 1;
	}
	//! [Adding servo ID0]
	//! [Adding servo ID1]
	rr_servo_t *servo2 = rr_init_servo(iface, id2);
	if(!servo2)
	{
		API_DEBUG("Servo #2 init error\n");
		return 1;
	}
	//! [Adding servo ID1]

	//! [Switching to operational state]
	rr_servo_set_state_operational(servo1);
	rr_servo_set_state_operational(servo2);
	//! [Switching to operational state]

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
	//! [Start motion2]
	rr_start_motion(iface, 0);
	//! [Start motion2]

	//! [Sleep2]
	rr_sleep_ms(14000); //wait till the movement ends
	//! [Sleep2]
	//! [cccode 2]
}
