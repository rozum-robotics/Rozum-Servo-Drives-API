/**
 * @brief Detecting available CAN devices and their states
 * @file discovery.c
 * @author Rozum
 * @date 2018-07-11
 */

#include "api.h"
#include <stdlib.h>

/**
 * \defgroup tutor_c_discovery Detecting available CAN devices
 *The tutorial demonstrates how to identify all devices connected to a CAN interface and to get device data,
 *such as device name, as well as its hardware and software versions.
 *
 *1. Create variables to save the states of potentially available CAN devices. The total number of variables is up to 128.
 *\snippet discovery.c Create variables
 *2. Initiate the interface.
 *\snippet discovery.c Init interface33
 *3. Enable the RR_NMT_HB_TIMEOUT variable.
 *\snippet discovery.c Enable hb variable
 *4. Run a work cycle to scan the CAN bus, reading the states of potentially available CAN devices every 100 ms. Compare the read values with the previous states.
 *\snippet discovery.c Scan
 *5. Run an auxiliary function to display the following data about available CAN devices:
 *<ul><li> Hardware and software version</li>
 *<li>Device name</li></ul>
 *\snippet discovery.c auxiliary to display
 *The same sequence of scanning the CAN bus repeats multiple times
 *until you stop the program by pressing the Ctrl-C hotkey combination.
 *
 * <b> Complete tutorial code: </b>
 * \snippet discovery.c discovery_code_full
  */
   
//! [discovery_code_full]
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

     //! [auxiliary to display]
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
//! [auxiliary to display]

//! [Create variables]
int main(int argc, char *argv[])
{
	rr_nmt_state_t states[MAX_CO_DEV];
	rr_nmt_state_t state;

	if(argc != 2)
	{
		API_DEBUG("Wrong format!\nUsage: %s interface\n", argv[0]);
		return 1;
	}
	//! [Create variables]

	//! [Init interface33]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}
	//! [Init interface33]

	//! [Enable hb variable]
	for(int i = 0; i < MAX_CO_DEV; i++)
	{
		states[i] = RR_NMT_HB_TIMEOUT;
	}
	//! [Enable hb variable]
	API_DEBUG("Waiting for devices ...\nPress Ctrl-C to stop\n");

	rr_set_debug_log_stream(NULL);

	//! [Scan]
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
	//! [Scan]
}
//! [discovery_code_full]

