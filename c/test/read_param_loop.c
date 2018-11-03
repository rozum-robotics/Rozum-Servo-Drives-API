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
#include <stdlib.h>

#define TIME_DELTA_MS(x, y) ((x.tv_sec - y.tv_sec) * 1000 + (x.tv_usec - y.tv_usec) / 1000)
#define TIME_DELTA_US(x, y) ((x.tv_sec - y.tv_sec) * 1000000 + (x.tv_usec - y.tv_usec) )
 
int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("Usage: %s port id\n", argv[0]);
		exit(1);
	}

    rr_can_interface_t *iface = rr_init_interface(argv[1]);
    rr_servo_t *servo = rr_init_servo(iface, strtol(argv[2], 0, 0));
    float value;
	struct timeval tprev, tnow;

	while(1)
	{
		gettimeofday(&tprev, NULL);
		rr_read_parameter(servo, APP_PARAM_POSITION_ROTOR, &value);
		gettimeofday(&tnow, NULL);
		fprintf(stderr, "time_us %ld, value %f\n", TIME_DELTA_US(tnow, tprev), value);
	}

}
