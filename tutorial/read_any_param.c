#include "api.h"
#include "tutorial.h"

int main(int argc, char *argv[])
{
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);

    API_DEBUG("========== Tutorial of %s ==========\n", "reading any device parameter (single)");

    float value;
    
    rr_read_parameter(servo, APP_PARAM_POSITION_ROTOR, &value);
    API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_POSITION_ROTOR), value);

    rr_read_parameter(servo, APP_PARAM_VELOCITY_ROTOR, &value);
    API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_VELOCITY_ROTOR), value);

    rr_read_parameter(servo, APP_PARAM_VOLTAGE_INPUT, &value);
    API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_VOLTAGE_INPUT), value);

    rr_read_parameter(servo, APP_PARAM_CURRENT_INPUT, &value);
    API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_CURRENT_INPUT), value);
}
