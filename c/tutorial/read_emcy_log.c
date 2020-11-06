/**
 * @brief Reading emergency (EMY) log buffer
 * @file read_emcy_log.c
 * @author Rozum
 * @date 2018-06-25
 */

#include "api.h"
#include <stdlib.h>
#include <inttypes.h>

/**
 * \defgroup tutor_c_read_emcy_log Reading emergency (EMY) log
 * The tutorial describes how to get and read messages from the emergency log buffer.
 * The EMCY log stores EMCY messages about any events when a servo is out of normal operating state 
 * (e.g., overcurrent, high temperature)or goes back to the normal state.
 *
 * 1. Read the parameters as required to run the tutorial.
 * \snippet read_emcy_log.c Read tutorial param3
 * 2. Initiate the interface.
 * \snippet read_emcy_log.c Init interface34
 * 3. Initiate the servo.
 * \snippet read_emcy_log.c Init servo34
 * 4. Read EMCY messages from the EMCY log.
 * \snippet read_emcy_log.c read emcy
 * Reading the messages is according to the first in-fist out pinciple.
 * The function continues running until it reads the last EMCY message from the log.
 * 
 * <b> Complete tutorial code: </b>
 * \snippet read_emcy_log.c read_emcy_log_code_full
 */

//! [read_emcy_log_code_full]

//! [Read tutorial param3]  
int main(int argc, char *argv[])
{
	uint8_t id;
	emcy_log_entry_t *emcy;

	if(argc == 3)
	{
		id = strtol(argv[2], NULL, 0);
	}
	else
	{
		API_DEBUG("Wrong format!\nUsage: %s interface id\n", argv[0]);
		return 1;
	}
	//! [Read tutorial param3]

	//! [Init interface34]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}
	//! [Init interface34]

	//! [Init servo34]
	rr_servo_t *servo = rr_init_servo(iface, id);
	if(!servo)
	{
		API_DEBUG("Servo init error\n");
		return 1;
	}
	//! [Init servo34]

	//! [read emcy]
	while((emcy = rr_emcy_log_pop(iface)))
	{
		printf("id: %d, code: 0x%." PRIx16 ", reg: 0x%.2" PRIx8 ", bits: 0x%.2" PRIx8 ", info: 0x%.8" PRIx32 "\n",
				(int)emcy->id, emcy->err_code, emcy->err_reg, emcy->err_bits, emcy->err_info);
	}
	//! [read emcy]
	//! [read_emcy_log_code_full]
}
