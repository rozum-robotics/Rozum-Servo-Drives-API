#include "api.h"
#include <stdlib.h>

void sdo_read_str(rr_servo_t *servo, uint16_t idx, uint8_t sidx, char *buf, int max_sz)
{
	if(!buf)
	{
		return;
	}
	if(max_sz <= 0)
	{
		buf[0] = 0;
	}
	max_sz--;
    	if(rr_read_raw_sdo(servo, idx, sidx, buf, &max_sz, 1, 200) == RET_OK)
	{
		buf[max_sz] = '\0';
	}
	else
	{
		buf[0] = '\0';
	}

}

void get_dev_info(rr_can_interface_t *iface, int id)
{
	rr_servo_t *servo = rr_init_servo(iface, id);

	char name[1024];
	char hw[1024];
	char sw[1024];

    	sdo_read_str(servo, 0x1008, 0, name, sizeof(name));
    	sdo_read_str(servo, 0x1009, 0, hw, sizeof(hw));
    	sdo_read_str(servo, 0x100a, 0, sw, sizeof(sw));

	API_DEBUG("Device %d:\n  NAME: %s\n  HW: %s\n  SW: %s\n", id,
			name[0] ? name : "N/A",
			hw[0] ? hw : "N/A",
			sw[0] ? sw : "N/A");

	rr_deinit_servo(&servo);
}

int main(int argc, char *argv[])
{
	uint8_t id;
	float ang, vel, acc;
	rr_nmt_state_t states[MAX_CO_DEV];
	rr_nmt_state_t state;

	if(argc != 2)
	{
		API_DEBUG("Wrong format!\nUsage: %s interface\n", argv[0]);
		return 1;
	}

	rr_can_interface_t *iface = rr_init_interface(argv[1]);

	for(int i = 0; i < MAX_CO_DEV; i++)
	{
		states[i] = RR_NMT_HB_TIMEOUT;
	}

	API_DEBUG("Waiting for devices ...\nPress Ctrl-C to stop\n");

	while(true)
	{
		rr_sleep_ms(100);

		for(int i = 0; i < MAX_CO_DEV; i++)
		{
			rr_net_get_state(iface, i, &state);
			if(states[i] != state)
			{
				states[i] = state;
				if(state != RR_NMT_HB_TIMEOUT)
				{
					get_dev_info(iface, i);
				}
				else
				{
					API_DEBUG("DEVICE %d disappeared\n", i);
				}
			}
		}
	}
}


