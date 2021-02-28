#include "api.h"
#include <stdlib.h>
#include <string.h>

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
			break;
	}

	return;
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
	rr_servo_set_state_operational(servo);

	rr_setup_pdo_callback(iface, pdo_cb);

	while(true)
	{
		rpdo0_t rpdo0 = 
		{
			.mode = 1, //velocity
			.iwin = 0,
			.des_vel = 500,
			.des_curr = 0
		};

		rr_send_pdo(iface, id, RPDO0, sizeof(rpdo0), (uint8_t *)&rpdo0);
		rr_send_pdo_sync(iface);

		rr_sleep_ms(100);
	}


}
