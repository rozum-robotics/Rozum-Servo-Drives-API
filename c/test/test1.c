#include "api.h"

#define SERVO_ID 36

int main(int argc, char *argv[])
{
    FILE *log = fopen("comm.log", "w+");

    rr_set_debug_log_stream(stderr);

    rr_can_interface_t *iface = rr_init_interface("192.168.0.50:17700");

    rr_servo_t *servo = rr_init_servo(iface, SERVO_ID);

    rr_param_cache_setup_entry(servo, APP_PARAM_POSITION, true);
    rr_param_cache_setup_entry(servo, APP_PARAM_VELOCITY, true);
    rr_param_cache_setup_entry(servo, APP_PARAM_VOLTAGE_INPUT, true);
    rr_param_cache_setup_entry(servo, APP_PARAM_CURRENT_INPUT, true);

    rr_param_cache_setup_entry(servo, APP_PARAM_CURRENT_INPUT, false);
    rr_param_cache_setup_entry(servo, APP_PARAM_VOLTAGE_INPUT, false);
    rr_param_cache_setup_entry(servo, APP_PARAM_VELOCITY, false);
    rr_param_cache_setup_entry(servo, APP_PARAM_POSITION, false);

    rr_release(servo);
    rr_deinit_interface(&iface);

    fclose(log);

    return 0;
}
