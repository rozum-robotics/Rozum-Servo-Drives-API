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

	rr_servo_set_state_operational(servo);

	int32_t mt = 4;
	float value = 0;

	rr_read_parameter(servo, APP_PARAM_POSITION, &value);
	printf("Position before preset: %.3f\n", value);

	printf("Preset MT by 4 revolutions...\n");

	if(rr_write_raw_sdo(servo, 0x2208, 4, (uint8_t *)&mt, sizeof(mt), 1, 100) != RET_OK)
	{
		API_DEBUG("Write SDO error\n");
		exit(1);
	}

	rr_read_parameter(servo, APP_PARAM_POSITION, &value);
	printf("Position after preset: %.3f\n", value);

	
	printf("Preset MT by -4 revolutions...\n");
	mt = -4;

	if(rr_write_raw_sdo(servo, 0x2208, 4, (uint8_t *)&mt, sizeof(mt), 1, 100) != RET_OK)
	{
		API_DEBUG("Write SDO error\n");
		exit(1);
	}

	rr_read_parameter(servo, APP_PARAM_POSITION, &value);
	printf("Position second preset: %.3f\n", value);
}


