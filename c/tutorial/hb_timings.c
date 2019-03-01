#include "api.h"
#include <stdlib.h>
#include <inttypes.h>


int main(int argc, char *argv[])
{
	uint8_t id;

	int64_t min, max;

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


	API_DEBUG("Reset communication\n");
	if(rr_servo_reset_communication(servo) == RET_OK)
	{
		API_DEBUG("Reset communication done\n");
	}
	else
	{
		API_DEBUG("Failed to reset communication\n");
	}
	
	API_DEBUG("Reboot\n");
	if(rr_servo_reboot(servo) == RET_OK)
	{
		API_DEBUG("Reboot done\n");
	}
	else
	{
		API_DEBUG("Failed to reboot\n");
	}

	API_DEBUG("Stop\n");
	if(rr_servo_set_state_stopped(servo) == RET_OK)
	{
		API_DEBUG("Stop mode entered\n");
	}
	else
	{
		API_DEBUG("Failed to enter pre-op mode\n");
	}

	API_DEBUG("Pre-op\n");
	if(rr_servo_set_state_pre_operational(servo) == RET_OK)
	{
		API_DEBUG("Pre-op mode entered\n");
	}
	else
	{
		API_DEBUG("Failed to enter pre-op mode\n");
	}

	API_DEBUG("Operational\n");
	if(rr_servo_set_state_pre_operational(servo) == RET_OK)
	{
		API_DEBUG("Operational mode entered\n");
	}
	else
	{
		API_DEBUG("Failed to enter operational mode\n");
	}

	rr_servo_get_hb_stat(servo, &min, &max);
	API_DEBUG("HB timings stat: min %" PRId64 " max %" PRId64 "\n", min, max);

	rr_servo_clear_hb_stat(servo);

	API_DEBUG("Wait 5s ...\n");
	rr_sleep_ms(5000);

	rr_servo_get_hb_stat(servo, &min, &max);
	API_DEBUG("HB timings stat: min %" PRId64 " max %" PRId64 "\n", min, max);
}


