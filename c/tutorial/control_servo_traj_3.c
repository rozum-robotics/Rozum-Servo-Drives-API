/**
 * @brief Setting points for three servos
 * 
 * @file control_servo_traj_3.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include <stdlib.h>

/**
 * \defgroup tutor_c_servomove3 PVT trajectory for three servos
 * The tutorial describes how to set up motion trajectories for three servos and to execute them simultaneously. In this example, each motion trajectory comprises two PVT (position-velocity-time) points:
 * <ul><li>one PVT commanding servos to move to the position of 100 degrees in 6,000 milliseconds</li>
 * <li>one PVT commanding servos to move to the position of -100 degrees in 6,000 milliseconds</li></ul>
 * 
 * <p><b>Important!</b>Before setting up PVT points, make sure to change the default CAN IDs of the servos (see the \ref tutor_c_changeID1 tutorial).</p>
 * 
 *<b>Note:</b> When you set a PVT trajectory to move more than one servo simultaneously, mind that the clock rate of the servos can differ by up to 2-3%.
 * Therefore, if the preset PVT trajectory is rather long, servos can get desynchronized. To avoid the desynchronization, we have implemented the following mechanism:
 * <ul><li>The device controlling the servos broadcasts a sync CAN frame to all servos on an interface. The frame should have the following format:<br>
 * ID = 0x27f, data = uint32 (4 bytes),<br>
 * <b>Where:</b> ‘data’ stands for the microseconds counter value by modulus of 600,000,000, starting from any value.</li>
 * <li>Servos receive the frame and try to adjust their clock rates to that of the device using the PLL.
 * The adjustment proper starts after the servos receive the second frame and can take up to 5 seconds, depending on the broadcasting frequency.
 * The higher the broadcasting frequency, the less time the adjustment takes.</li>
 * <li>The broadcasting frequency is 5 Hz minimum. The recommended frequency range is from 10 to 20 Hz.
 * When the sync frames are not broadcast or the broadcast frequency is below 5 Hz, the clock rate of servos is as usual.</li>
 *
 * 1. Initialize the interface.
 * \snippet control_servo_traj_3.c Adding the interface3
 * 
 * 2. Initialize servo 1.
 * \snippet control_servo_traj_3.c Adding servo one
 * 
 * 3. Initialize servo 2.
 * \snippet control_servo_traj_3.c Adding servo two
 * 
 * 4. Initialize servo 3.
 * \snippet control_servo_traj_3.c Adding servo three
 * 
 * 5. Clear points in the motion queue of servo 1.
 * \snippet control_servo_traj_3.c Clear points servo one
 * 
 * 6. Clear points in the motion queue of servo 2.
 * \snippet control_servo_traj_3.c Clear points servo two
 * 
 * 7. Clear points in the motion queue of servo 3.
 * \snippet control_servo_traj_3.c Clear points servo three
 * 
 * <b> Adding PVT ponts to form motion queues </b>
 * 
 * 8. Set the first PVT point for servo 1, commanding it to move to the position of 100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_3.c Add point one servo one
 * 
 * 9. Set the first PVT point for servo 2, commanding it to move to the position of 100 degrees in 6,000 milliseconds.
 *  \snippet control_servo_traj_3.c Add point one servo two
 * 
 * 10. Set the first PVT point for servo 3, commanding it to move to the position of 100 degrees in 6,000 milliseconds.
 *  \snippet control_servo_traj_3.c Add point one servo three
 * 
 * 11.  Set the second PVT point for servo 1, commanding it to move to the position of -100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_3.c Add point two servo one
 * 
 * 12. Set the second PVT point for servo 2, commanding it to move to the position of -100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_3.c Add point two servo two
 * 
 * 13. Set the second PVT point for servo 3, commanding it to move to the position of -100 degrees in 6,000 milliseconds.
 * \snippet control_servo_traj_3.c Add point two servo three
 * 
 * <b> Executing the resulting motion queues </b>
 * 
 * 14. Command all servos to start moving simulateneously. Each of the three servos will execute their own motion queues. Set the function parameter to 0 to get the servos moving without a delay.
 * \snippet control_servo_traj_3.c Start motion3
 * 
 * 15. To ensure the program will not move on to execute another operation, set an idle period of 14,000 milliseconds.
 * \snippet control_servo_traj_3.c Sleep3
 * 
 * <b> Complete tutorial code: </b>
 * \snippet control_servo_traj_3.c cccode 3
 */
int main(int argc, char *argv[])
{
	uint8_t id1, id2, id3;

	if(argc == 5)
	{
		id1 = strtol(argv[2], NULL, 0);
		id2 = strtol(argv[3], NULL, 0);
		id3 = strtol(argv[4], NULL, 0);
	}
	else
	{
		API_DEBUG("Wrong format!\nUsage: %s interface id1 id2 id3\n", argv[0]);
		return 1;
	}
	//! [cccode 3] 
	//! [Adding the interface3]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}
	//! [Adding the interface3]
	//! [Adding servo one]
	rr_servo_t *servo1 = rr_init_servo(iface, id1);
	if(!servo1)
	{
		API_DEBUG("Servo #1 init error\n");
		return 1;
	}
	//! [Adding servo one]
	//! [Adding servo two]
	rr_servo_t *servo2 = rr_init_servo(iface, id2);
	if(!servo2)
	{
		API_DEBUG("Servo #2 init error\n");
		return 1;
	}
	//! [Adding servo two]
	//! [Adding servo three]
	rr_servo_t *servo3 = rr_init_servo(iface, id3);
	if(!servo3)
	{
		API_DEBUG("Servo #2 init error\n");
		return 1;
	}
	//! [Adding servo three]

	//! [Switching to operational state]
	rr_servo_set_state_operational(servo1);
	rr_servo_set_state_operational(servo2);
	rr_servo_set_state_operational(servo3);
	//! [Switching to operational state]

	API_DEBUG("========== Tutorial of the %s ==========\n", "controlling three servos");

	//! [Clear points servo one]
	rr_clear_points_all(servo1);
	//! [Clear points servo one]
	//! [Clear points servo two]
	rr_clear_points_all(servo2);
	//! [Clear points servo two]
	//! [Clear points servo three]
	rr_clear_points_all(servo3);
	//! [Clear points servo three]
	//! [Add point one servo one]
	int status = rr_add_motion_point(servo1, 100.0, 0.0, 6000);
	if(status != RET_OK)
	{
		API_DEBUG("Error in the trjectory point calculation: %d\n", status);
		return 1;
	}
	//! [Add point one servo one]
	//! [Add point one servo two]
	status = rr_add_motion_point(servo2, 100.0, 0.0, 6000);
	if(status != RET_OK)
	{
		API_DEBUG("Error in the trjectory point calculation: %d\n", status);
		return 1;
	}
	//! [Add point one servo two]
	//! [Add point one servo three]
	status = rr_add_motion_point(servo3, 100.0, 0.0, 6000);
	if(status != RET_OK)
	{
		API_DEBUG("Error in the trjectory point calculation: %d\n", status);
		return 1;
	}
	//! [Add point one servo three]
	//! [Add point two servo one]
	status = rr_add_motion_point(servo1, -100.0, 0.0, 6000);
	if(status != RET_OK)
	{
		API_DEBUG("Error in the trjectory point calculation: %d\n", status);
		return 1;
	}
	//! [Add point two servo one]
	//! [Add point two servo two]
	status = rr_add_motion_point(servo2, -100.0, 0.0, 6000);
	if(status != RET_OK)
	{
		API_DEBUG("Error in the trjectory point calculation: %d\n", status);
		return 1;
	}
	//! [Add point two servo two]
	//! [Add point two servo three]
	status = rr_add_motion_point(servo3, -100.0, 0.0, 6000);
	if(status != RET_OK)
	{
		API_DEBUG("Error in the trjectory point calculation: %d\n", status);
		return 1;
	}
	//! [Add point two servo three]
	//! [Start motion3]
	rr_start_motion(iface, 0);
	//! [Start motion3]

	//! [Sleep3]
	rr_sleep_ms(14000); // wait till the movement ends
	//! [Sleep3]
	//! [cccode 3]
}
