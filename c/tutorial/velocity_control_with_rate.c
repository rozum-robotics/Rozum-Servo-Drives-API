/**
 * @brief Example of using velocity rate limiter
 * @file velocity_control_with_rate.c
 * @author Rozum
 * @date 2018-07-11
 */

#include "api.h"
#include <stdlib.h>
#include <math.h>

/**
 * \defgroup tutor_c_cogging Example of using velocity rate limiter
 *The tutorial demonstrates how to use velocity control with velocity rate limiter.
 *
*/
  
int main(int argc, char *argv[])
{
	uint8_t id;
	float vel_des, rate_des, time_s;

	if(argc == 6)
	{
		id = strtol(argv[2], NULL, 0);
		vel_des = atof(argv[3]);
		rate_des = atof(argv[4]);
		time_s = atof(argv[5]);
	}
	else
	{
		API_DEBUG("Wrong format!\nUsage: %s interface id vel_deg_per_sec rate_rpm_per_sec time_sec\n", argv[0]);
		return 1;
	}


	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	rr_servo_t * servo = rr_init_servo(iface, id);

	if(!servo)
	{
		API_DEBUG("Can't find servo with specified ID\n");
		exit(1);
	}

	
	if(rr_servo_set_state_operational(servo) != RET_OK)
	{
		API_DEBUG("Can't put servo to operational mode\n");
		exit(1);
	}


	float rate_act = 0;
	rr_get_velocity_rate(servo, &rate_act);

	API_DEBUG("Actual velocity rate: %f\n", rate_act);

	rr_set_velocity_rate(servo, rate_des);
	rr_set_velocity(servo, vel_des);

	rr_sleep_ms(time_s * 1000.0);

	rr_set_velocity(servo, 0);
	rate_des *= 2.0;
	rr_set_velocity_rate(servo, rate_des);

	float vel_rotor_rpm;

	rr_read_parameter(servo, APP_PARAM_VELOCITY_ROTOR, &vel_rotor_rpm);
	
	rr_sleep_ms(fabsf(vel_rotor_rpm) / rate_des * 1000.0);
	
}


