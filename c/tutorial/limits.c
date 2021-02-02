#include "api.h"
#include <stdlib.h>
#include <math.h>

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

	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}
	rr_servo_t *servo = rr_init_servo(iface, id);
	if(!servo)
	{
		API_DEBUG("Servo init error\n");
		return 1;
	}

	rr_servo_set_state_operational(servo);

	float lim[] = {-100, -90,  90, 100};

	printf("writing limits\n");

	for(int i = 0; i < 4; i++)
	{
		if(rr_write_raw_sdo(servo, 0x2302, i + 1, (uint8_t *)&lim[i], sizeof(float), 1, 100) != RET_OK)
		{
			API_DEBUG("Can't write limit: 0x2302-sub%d\n", i);
			exit(1);
		}
	}

	printf("reading limits back\n");

	for(int i = 0; i < 4; i++)
	{
		float v = 0;
		uint32_t len = sizeof(v);
		if(rr_read_raw_sdo(servo, 0x2302, i + 1, (uint8_t *)&v, &len, 1, 100) != RET_OK)
		{
			API_DEBUG("Can't write limit: 0x2302-sub%d\n", i);
			exit(1);
		}

		printf("0x2302-sub%d: %f\n", i, v);	
	}

	printf("Move to '0'\n");
	uint32_t time_to_get_ms;
	if(rr_set_position_with_limits(servo, 0, 100, 100, &time_to_get_ms) != RET_OK)
	{
		API_DEBUG("Can't move to position\n");
		exit(1);
	}
    	rr_sleep_ms(time_to_get_ms + 100); // wait till the movement ends

	printf("move in positive direction in vel. mode ~ 10s\n");
	rr_set_velocity_rate(servo, 1000);
	rr_set_velocity(servo, 1000);

	rr_sleep_ms(10000);

	float value = 0;
	rr_read_parameter(servo, APP_PARAM_POSITION, &value);
	printf("%.3f\n", value);

	printf("move in negative direction in vel. mode ~10s\n");
	rr_set_velocity_rate(servo, 1000);
	rr_set_velocity(servo, -1000);

	rr_sleep_ms(10000);

	rr_read_parameter(servo, APP_PARAM_POSITION, &value);
	printf("%.3f\n", value);
}


