#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct
{
	uint8_t mode;
	uint8_t iwin;
	int16_t des_curr;
	float des_vel;
} rpdo0_t; //to servo

typedef struct
{
	float pos;
	int16_t act_vel;
	int16_t act_curr;
} tpdo0_t; //from servo

typedef struct
{
	uint16_t input_voltage;
	uint16_t input_current;
} tpdo3_t; //from servo

void pdo_cb(rr_can_interface_t *interface, int id, rr_pdo_n_t pdo_n, int len, uint8_t *data)
{

	switch(pdo_n)
	{
		case TPDO0:
		{
			tpdo0_t pdo0;
			memcpy(&pdo0, data, len);
			printf("pos: %f deg, vel: %f deg/s, curr: %f A\n", pdo0.pos, pdo0.act_vel * 0.02, pdo0.act_curr * 0.0016);
		}
			break;
		case TPDO1:
			break;
		case TPDO2:
			break;
		case TPDO3:
		{
			tpdo3_t pdo3 = {0};
			memcpy(&pdo3, data, len);
			printf("input voltage: %f V, input curr: %f A\n", pdo3.input_voltage * 0.001, pdo3.input_current * 0.001);
		}
			break;
	}

	return;
}




void pdo_configure(rr_servo_t *s)
{
	rr_pdo_clear_map(s, RPDO2);
	rr_pdo_clear_map(s, RPDO3);
	rr_pdo_clear_map(s, TPDO2);
	rr_pdo_clear_map(s, TPDO3);
	rr_pdo_add_map(s, TPDO3, 0x5001, 0x0d, 16);
	rr_pdo_add_map(s, TPDO3, 0x5001, 0x0e, 16);
}

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

	pdo_configure(servo);

	rr_servo_set_state_operational(servo);
	rr_setup_pdo_callback(iface, pdo_cb);

	double ph = 0, f = 1.0;

	while(true)
	{
		rpdo0_t rpdo0 = 
		{
			.mode = 1, //velocity
			.iwin = 0,
			.des_vel = 1500 * sin(ph),
			.des_curr = 0
		};

		ph += 2.0 * M_PI * f * 0.1;
		ph -= ph > 2.0 * M_PI ? 2.0 * M_PI : 0;

		rr_send_pdo(iface, id, RPDO0, sizeof(rpdo0), (uint8_t *)&rpdo0);
		rr_send_pdo_sync(iface);

		rr_sleep_ms(100);
	}
}

