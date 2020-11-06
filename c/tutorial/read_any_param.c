/**
 * @brief Tutorial example of reading device parameters
 * 
 * @file read_any_param.c
 * @author your name
 * @date 2018-06-25
 */

#include "api.h"
#include <stdlib.h>

/**
 * \defgroup tutor_c_param Reading device parameters
 * 
 * The tutorial describes how to read a sequence of single variables representing actual servo parameters (e.g., position, voltage, etc.)
 * <b>Note</b>: For reference, the tutorial includes more than one parameter.
 * In practice, however, if you need to read more than one parameter, refer to the tutorial <b>Setting up parameter cache and reading cached parameters</b>.
 * 
 * 1. Initialize the interface.
 * \snippet read_any_param.c Adding interface 4
 * 
 * 2. Initialize the servo.
 * \snippet read_any_param.c Adding servo 4
 * 
 * <b> Reading current device parameters </b>
 * 
 * 3. Create a variable where the function will save the parameters.
 * \snippet read_any_param.c Read parameter variable
 * 
 * 4. Read the actual rotor position.
 * \snippet read_any_param.c Read rotor position
 * 
 * 5. Read the actual rotor velocity.
 * \snippet read_any_param.c Read rotor velocity
 * 
 * 6. Read the actual input voltage.
 * \snippet read_any_param.c Read voltage
 * 
 * 7. Read the actual input current.
 * \snippet read_any_param.c Read current
 * 
 * <b> Complete tutorial code: </b>
 * \snippet read_any_param.c cccode 4
 */

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
	//! [cccode 4] 
	//! [Adding interface 4]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);

	//! [Adding interface 4]
	//! [Adding servo 4]
	rr_servo_t *servo = rr_init_servo(iface, id);
	if(!servo)
	{
		API_DEBUG("Servo init error\n");
		return 1;
	}
	//! [Adding servo 4]

	API_DEBUG("========== Tutorial of %s ==========\n", "reading any device parameter (single)");

	//! [Read parameter variable]
	float value;
	//! [Read parameter variable]

	//! [Read rotor position]
	rr_read_parameter(servo, APP_PARAM_POSITION_ROTOR, &value);
	//! [Read rotor position]
	API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_POSITION_ROTOR), value);

	//! [Read rotor velocity]
	rr_read_parameter(servo, APP_PARAM_VELOCITY_ROTOR, &value);
	//! [Read rotor velocity]
	API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_VELOCITY_ROTOR), value);

	//! [Read voltage]
	rr_read_parameter(servo, APP_PARAM_VOLTAGE_INPUT, &value);
	//! [Read voltage]
	API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_VOLTAGE_INPUT), value);

	//! [Read current]
	rr_read_parameter(servo, APP_PARAM_CURRENT_INPUT, &value);
	//! [Read current]
	API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_CURRENT_INPUT), value);
	//! [cccode 4]
}


