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

	while(1)
	{

		printf("init iface\n");
		rr_can_interface_t *iface = rr_init_interface(argv[1]);
		printf("init servo\n");
		rr_servo_t *servo = rr_init_servo(iface, id);
		printf("putting to operational\n");
		rr_servo_set_state_operational(servo);

		float value;

		printf("reading parameter\n");
		rr_read_parameter(servo, APP_PARAM_POSITION, &value);

		printf("position: %f\n", value);

		printf("deinit\n");
		rr_deinit_interface (&iface);
	}
	
}


