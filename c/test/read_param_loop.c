/**
 * @brief Tutorial example of reading device parameters
 * 
 * @file read_any_param.c
 * @author your name
 * @date 2018-06-25
 */

#include "api.h"
#include "tutorial.h"

#include <sys/time.h>

#define TIME_DELTA_MS(x, y) ((x.tv_sec - y.tv_sec) * 1000 + (x.tv_usec - y.tv_usec) / 1000)
#define TIME_DELTA_US(x, y) ((x.tv_sec - y.tv_sec) * 1000000 + (x.tv_usec - y.tv_usec) )
 
int main(int argc, char *argv[])
{
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    float value;
	struct timeval tprev, tnow;

	for(int i = 0; i < 1000; i++)
	{
		gettimeofday(&tprev, NULL);
		rr_read_parameter(servo, APP_PARAM_POSITION_ROTOR, &value);
		gettimeofday(&tnow, NULL);
		fprintf(stderr, "%d: time_ms %ld, value %f\n", i, TIME_DELTA_US(tnow, tprev), value);
	}

}
