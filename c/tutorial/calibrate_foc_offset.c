/**
 * @brief Calibrating of FOC offset angle
 * @file calibrate_foc_offset.c
 * @author Rozum
 * @date 2018-07-19
 */

#include "api.h"
#include <math.h>
#include <stdlib.h>

#define CALIBRATION_CURRENT_A (8.0f)

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

	float current_a = CALIBRATION_CURRENT_A;
	if(rr_write_raw_sdo(servo, 0x4206, 4, (uint8_t *)&current_a, sizeof(float), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't start calibration\n");
		exit(2);
	}

	float start = 1.0f;
	if(rr_write_raw_sdo(servo, 0x4206, 1, (uint8_t *)&start, sizeof(float), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't start calibration\n");
		exit(2);
	}

	while(1)
	{
		float state = -1.0f;
		int size = sizeof(float);
		if(rr_read_raw_sdo(servo, 0x4206, 1, (uint8_t *)&state, &size, 1, 100) != RET_OK)
		{
			API_DEBUG("Calibration state polling error\n");
			exit(3);
		}
		if(size != 4)
		{
			API_DEBUG("Calibration state polling error (wrong size)\n");
			exit(4);
		}

		if(state == 0.0f)
		{
			break;
		}
		else if(state != 1.0f)
		{
			API_DEBUG("Calibration state error: %.0f\n", state);
			exit(5);
		}

		rr_sleep_ms(200);
	}

	float ratio = -1.0f, offset = -1;
	int size = sizeof(float);
	if(rr_read_raw_sdo(servo, 0x4206, 2, (uint8_t *)&ratio, &size, 1, 100) != RET_OK)
	{
		API_DEBUG("Failed to read ratio\n");
		exit(6);
	}
	if(rr_read_raw_sdo(servo, 0x4206, 3, (uint8_t *)&offset, &size, 1, 100) != RET_OK)
	{
		API_DEBUG("Failed to read offset\n");
		exit(7);
	}

	API_DEBUG("Calculated Pole pairs: %.0f, Offset: %.3f\n", ratio, offset);

	API_DEBUG("Saving to flash...");

	if(rr_write_raw_sdo(servo, 0x4300, 2, (uint8_t *)&ratio, sizeof(float), 1, 100) != RET_OK)
	{
		API_DEBUG("Failed to write ratio\n");
		exit(8);
	}

	if(rr_write_raw_sdo(servo, 0x4300, 3, (uint8_t *)&offset, sizeof(float), 1, 100) != RET_OK)
	{
		API_DEBUG("Failed to write ratio\n");
		exit(9);
	}

	if(rr_write_raw_sdo(servo, 0x1010, 1, (uint8_t *)"evas", 4, 1, 4000) != RET_OK)
	{
		API_DEBUG("Can't save to flash\n");
		exit(10);
	}
	API_DEBUG("OK\n");
}
