/**
 * @brief Calibrating to mitigate cogging effects
 * @file calibrate_cogging.c
 * @author Rozum
 * @date 2018-07-11
 */

#include "api.h"
#include <stdlib.h>
#include <math.h>

/**
 * \defgroup tutor_c_cogging Calibrating to mitigate cogging effects
 *The tutorial describes how to calibrate a servo to optimize its motion trajectory, while
 *compensating for torque fluctuations due to cogging effects.
 *
 *1. Read the parameters as required to run the cogging calibration tutorial.
 *\snippet calibrate_cogging.c Read tutorial param1
 *
 *2. Initialize the interface.
 *\snippet calibrate_cogging.c Init interface31
 *
 *3. Initialize the servo.
 *\snippet calibrate_cogging.c Init servo31
 *
 *4. Set the servo to the operational state.
 *\snippet calibrate_cogging.c Switching to operational state1
 *
 *5. Start calibration. During the procedure, the servo rotates to +/-45 degrees relative to its start position.
 *\snippet calibrate_cogging.c Start calibration
 *
 *6. Read the VELOCITY_SETPOINT parameter from time to time until the output is 0. As soon as the parameter reaches the value, calibration is over.
 *\snippet calibrate_cogging.c Read vel setpoint
 *
 *7. Enable the resulting cogging compensation table.
 *\snippet calibrate_cogging.c Enable cog table
 *
 *8. Save the cogging compensation table to the FLASH memory.
 *\snippet calibrate_cogging.c Save to flash
 *
 * <b> Complete tutorial code: </b>
 * \snippet calibrate_cogging.c calibrate_cogging_code_full
 */

//! [calibrate_cogging_code_full]
//! [Read tutorial param1]
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
	//! [Read tutorial param1]

	//! [Init interface31]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}
	//! [Init interface31]

	//! [Init servo31]
	rr_servo_t *servo = rr_init_servo(iface, id);
	if(!servo)
	{
		API_DEBUG("Servo init error\n");
		return 1;
	}
	//! [Init servo31]

	//! [Switching to operational state1]
	rr_servo_set_state_operational(servo);

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
	//! [Switching to operational state1]

	//! [Start calibration]
	uint8_t cogging_cmd[] = {0x1a, 0, 0, 0, 0};
	uint8_t cogging_status = 0, cogging_status_prev = 0;

	if(rr_write_raw_sdo(servo, 0x4010, 0, (uint8_t *)cogging_cmd, sizeof(cogging_cmd), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't start calibration\n");
		exit(1);
	}
	//![Start calibration]

	//![Read vel setpoint]

	for(int i = 0; i < 10; i++)
	{
		rr_sleep_ms(10);
		int l = sizeof(cogging_status);
		if(rr_read_raw_sdo(servo, 0x4210, 0, &cogging_status, &l, 1, 100) != RET_OK)
		{
			API_DEBUG("Can't read procedure status\n");
			exit(1);
		}
		if(cogging_status)
		{
			API_DEBUG("Calibration started\n");
			break;
		}

	}
	cogging_status_prev = cogging_status;
	while(true)
	{
		rr_sleep_ms(100);
		int l = sizeof(cogging_status);
		if(rr_read_raw_sdo(servo, 0x4210, 0, &cogging_status, &l, 1, 100) != RET_OK)
		{
			API_DEBUG("Can't read procedure status\n");
			exit(1);
		}
		if(cogging_status != cogging_status_prev)
		{
			API_DEBUG("Actual status %d\n", (int)cogging_status);
		}
		cogging_status_prev = cogging_status;
		if(!cogging_status)
		{
			API_DEBUG("Calibration finished\n");
			break;
		}
	}
	//![Read vel setpoint]

	//![Enable cog table]

	float enable = 1;

	if(rr_write_raw_sdo(servo, 0x41ff, 15, (uint8_t *)&enable, sizeof(enable), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't enable friction map\n");
		exit(1);
	}
	//![Enable cog table]

	//![Save to flash]
	API_DEBUG("Saving to flash\n");

	if(rr_write_raw_sdo(servo, 0x1010, 1, (uint8_t *)"evas", 4, 1, 4000) != RET_OK)
	{
		API_DEBUG("Can't save to flash\n");
	}
	//![Save to flash]
	//![calibrate_cogging_code_full]
}


