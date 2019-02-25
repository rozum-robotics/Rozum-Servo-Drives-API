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
	rr_servo_t *servo = rr_init_servo(iface, id);

	//! [Switching to operational state]
	rr_servo_set_state_operational(servo);
	//! [Switching to operational state]
	
	rr_nmt_state_t state = 0;
	for(int i = 0; i < 20; i++)
	{
		rr_sleep_ms(100);
		rr_servo_get_state(servo, &state);
		if(state == RR_NMT_OPERATIONAL)
		{
			break;
		}
	}
	if(state != RR_NMT_OPERATIONAL)
	{
		API_DEBUG("Can't switch tot operational mode\n");
		exit(1);
	}


	uint8_t cogging_cmd[] = {0x1a, 0, 0, 0, 0};
	float value = 0, value_prev = 0;    

	if(rr_write_raw_sdo(servo, 0x4010, 0, (uint8_t *)cogging_cmd, sizeof(cogging_cmd), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't start calibration\n");
		exit(1);
	}

	while(true)
	{
		rr_sleep_ms(100);
		rr_read_parameter(servo, APP_PARAM_CONTROLLER_VELOCITY_SETPOINT, &value);
		if(value != value_prev)
		{
			API_DEBUG("APP_PARAM_CONTROLLER_VELOCITY_SETPOINT value: %.3f\n", value);
		}
		value_prev = value;

		if(fabsf(value) < 1)
		{
			API_DEBUG("Calibration finished\n");
			break;
		}
	}

	value = 1.0;
	if(rr_write_raw_sdo(servo, 0x41ff, 15, (uint8_t *)&value, sizeof(value), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't enable cogging map\n");
		exit(1);
	}

	if(rr_write_raw_sdo(servo, 0x41ff, 16, (uint8_t *)&value, sizeof(value), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't enable friction map\n");
		exit(1);
	}

	API_DEBUG("Saving to flash\n");

	if(rr_write_raw_sdo(servo, 0x1010, 1, (uint8_t *)"evas", 4, 1, 4000) != RET_OK)
	{
		API_DEBUG("Can't save to flash\n");
	}
}


