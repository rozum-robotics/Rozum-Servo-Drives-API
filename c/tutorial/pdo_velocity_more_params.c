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
			tpdo3_t pdo3;
			memcpy(&pdo3, data, len);
			printf("input voltage: %f V, input curr: %f A\n", pdo3.input_voltage * 0.001, pdo3.input_current * 0.001);
		}
			break;
	}

	return;
}

void pdo_configure(rr_servo_t *servo)
{
	uint32_t tpdo3_cobid;
	int l = 4;
	if(rr_read_raw_sdo(servo, 0x1803, 1, (uint8_t *)&tpdo3_cobid, &l, 1, 100) != RET_OK) exit(1);
	//destroy PDOs
	tpdo3_cobid |= 0x80000000ul;
	if(rr_write_raw_sdo(servo, 0x1803, 1, (uint8_t *)&tpdo3_cobid, 4, 1, 100) != RET_OK) exit(1);
	
	//setup mappings
	uint8_t obj_cnt = 0;
	uint32_t tpdo3_map1 = 0x50010d10;
	uint32_t tpdo3_map2 = 0x50010e10;
	if(rr_write_raw_sdo(servo, 0x1A03, 0, (uint8_t *)&obj_cnt, 1, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1A03, 1, (uint8_t *)&tpdo3_map1, 4, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1A03, 2, (uint8_t *)&tpdo3_map2, 4, 1, 100) != RET_OK) exit(1);
	
	obj_cnt = 2;
	if(rr_write_raw_sdo(servo, 0x1A03, 0, (uint8_t *)&obj_cnt, 1, 1, 100) != RET_OK) exit(1);
	
	//create PDOs
	tpdo3_cobid &= ~0x80000000ul;
	if(rr_write_raw_sdo(servo, 0x1803, 1, (uint8_t *)&tpdo3_cobid, 4, 1, 100) != RET_OK) exit(1);
	
	uint8_t pdo_mode = 1;
	if(rr_write_raw_sdo(servo, 0x1803, 2, (uint8_t *)&pdo_mode, 1, 1, 100) != RET_OK) exit(1);
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

