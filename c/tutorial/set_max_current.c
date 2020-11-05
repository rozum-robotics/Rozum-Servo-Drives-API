#include "api.h"
#include <stdlib.h>
#include <math.h>

  
int main(int argc, char *argv[])
{
	uint8_t id;
	float imax;

	if(argc == 4)
	{
		id = strtol(argv[2], NULL, 0);
		imax = fabs(atof(argv[3]));
	}
	else
	{
		API_DEBUG("Wrong format!\nUsage: %s interface id imax\n", argv[0]);
		return 1;
	}
	
	imax = imax > 70.0 ? 70.0 : imax;

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


	if(rr_write_raw_sdo(servo, 0x4306, 1, (uint8_t *)&imax, sizeof(imax), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't write SDO\n");
		exit(1);
	}

	if(rr_write_raw_sdo(servo, 0x4306, 6, (uint8_t *)&imax, sizeof(imax), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't write SDO\n");
		exit(1);
	}
	
	if(rr_write_raw_sdo(servo, 0x1010, 1, (uint8_t *)"evas", 4, 1, 4000) != RET_OK)
	{
		API_DEBUG("Can't save to flash\n");
	}
	
}


