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

static uint16_t map_obj(rr_pdo_n_t n)
{
	static const uint16_t o[] = 
	{
		0x1600, 0x1601, 0x1602, 0x1603,
		0x1A00, 0x1A01, 0x1A02, 0x1A03
	};
	return o[n];
}

static uint16_t tr_type_obj(rr_pdo_n_t n)
{
	static const uint16_t o[] = 
	{
		0x1400, 0x1401, 0x1402, 0x1403, 
		0x1800, 0x1801, 0x1802, 0x1803
	};
	return o[n];
}

bool rr_pdo_disable(rr_servo_t *s, rr_pdo_n_t n)
{
	uint32_t cob_id;
	int l = 4;

	if(rr_read_raw_sdo(s, tr_type_obj(n), 1, (uint8_t *)&cob_id, &l, 1, 100) != RET_OK) return false;
	cob_id |= 0x80000000ul;
	if(rr_write_raw_sdo(s, tr_type_obj(n), 1, (uint8_t *)&cob_id, l, 1, 100) != RET_OK) return false;
	
	return true;
}

bool rr_pdo_enable(rr_servo_t *s, rr_pdo_n_t n)
{
	uint32_t cob_id;
	int l = 4;

	if(rr_read_raw_sdo(s, tr_type_obj(n), 1, (uint8_t *)&cob_id, &l, 1, 100) != RET_OK) return false;
	cob_id &= ~0x80000000ul;
	if(rr_write_raw_sdo(s, tr_type_obj(n), 1, (uint8_t *)&cob_id, l, 1, 100) != RET_OK) return false;
	
	return true;
}

bool rr_pdo_set_map_count(rr_servo_t *s, rr_pdo_n_t n, uint8_t cnt)
{
	if(rr_write_raw_sdo(s, map_obj(n), 0, &cnt, 1, 1, 100) != RET_OK) return false;

	return true;
}

int rr_pdo_get_map_count(rr_servo_t *s, rr_pdo_n_t n, uint8_t *cnt)
{
	int l = 1;
	if(rr_read_raw_sdo(s, map_obj(n), 0, cnt, &l, 1, 100) != RET_OK) return false;
	
	return true;
}


bool rr_pdo_clear_map(rr_servo_t *s, rr_pdo_n_t n)
{
	if(!rr_pdo_disable(s, n)) return false;
	if(!rr_pdo_set_map_count(s, n, 0)) return false;

	return true;	
}

bool rr_pdo_write_map(rr_servo_t *s, rr_pdo_n_t n, uint8_t map_entry, uint32_t map_value)
{
	if(rr_write_raw_sdo(s, map_obj(n), map_entry, (uint8_t *)&map_value, 4, 1, 100) != RET_OK) 
	{
		return false;
	}

	return true;
}

bool rr_pdo_read_map(rr_servo_t *s, rr_pdo_n_t n, uint8_t map_entry, uint32_t *map_value)
{
	int l = 4;

	if(rr_read_raw_sdo(s, map_obj(n), map_entry, (uint8_t *)map_value, &l, 1, 100) != RET_OK) 
	{
		return false;
	}

	return true;
}

bool rr_pdo_get_byte_len(rr_servo_t *s, rr_pdo_n_t n, int *len)
{
	uint8_t map_cnt = 0;
	uint32_t map_value;

	if(!rr_pdo_get_map_count(s, n, &map_cnt)) return false;

	*len = 0;

	for(int i = 0; i < map_cnt; i++)
	{
		if(!rr_pdo_read_map(s, n, i + 1, &map_value)) return false;
		*len += (map_value & 0xff) >> 3;		
	}
	
	return true;
}

bool rr_pdo_add_map(rr_servo_t *s, rr_pdo_n_t n, uint16_t idx, uint8_t sidx, uint8_t bit_len)
{
	if(bit_len & 7)
	{
		API_DEBUG("Bit length not multiple of 8\n");
		return false;
	}

	int byte_len;

	if(!rr_pdo_get_byte_len(s, n, &byte_len)) return false;

	if((byte_len + (bit_len >> 3)) > 8)
	{
		API_DEBUG("Adding this map will exceed the allowed PDO length (8 bytes)\n");
		return false;
	}
	
	uint8_t map_cnt = 0;
	uint32_t map = (uint32_t)idx << 16 | (uint32_t)sidx << 8 | (uint32_t)bit_len;

	if(!rr_pdo_get_map_count(s, n, &map_cnt)) return false;
	map_cnt++;
	if(!rr_pdo_disable(s, n)) return false;
	if(!rr_pdo_set_map_count(s, n, 0)) return false;
	if(!rr_pdo_write_map(s, n, map_cnt, map)) return false;
	if(!rr_pdo_set_map_count(s, n, map_cnt)) return false;
	if(!rr_pdo_enable(s, n)) return false;

	return true;
}



void pdo_configure(rr_servo_t *s)
{
	rr_pdo_clear_map(s, TPDO3);
	rr_pdo_add_map(s, TPDO3, 0x5001, 0x0d, 16);
	rr_pdo_add_map(s, TPDO3, 0x5001, 0x0e, 16);
	rr_pdo_add_map(s, TPDO3, 0x5001, 0x0e, 16);
	rr_pdo_add_map(s, TPDO3, 0x5001, 0x0e, 16);
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

