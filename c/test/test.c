#include "api.h"

#define SERVO_ID 36

//#define M() printf("%d\n", __LINE__)
#define M()

void nmt_cb(rr_can_interface_t *interface, int servo_id, rr_nmt_state_t nmt_state)
{
	printf("NMT CB\n");
}

void emcy_cb(rr_can_interface_t *interface, int servo_id, uint16_t code, uint8_t reg, uint8_t bits, uint32_t info)
{	
	printf("EMCY CB\n");
}


int main(int argc, char *argv[])
{
    FILE *log = fopen("comm.log", "w+");

    rr_set_debug_log_stream(stderr);

    rr_can_interface_t *iface = rr_init_interface("/dev/ttyACM0");
    //rr_set_comm_log_stream(iface, stderr);
	//
	rr_setup_nmt_callback(iface, nmt_cb);
	rr_setup_emcy_callback(iface, emcy_cb);

    rr_servo_t *servo = rr_init_servo(iface, SERVO_ID);
    /*
	float vdc;
	rr_read_parameter(servo, APP_PARAM_VOLTAGE_INPUT, &vdc);

	printf("voltage: %f\n", vdc);
	*/

    //rr_set_velocity(servo, 10.0);
    //


    rr_param_cache_setup_entry(servo, APP_PARAM_POSITION, true);
    rr_param_cache_setup_entry(servo, APP_PARAM_VELOCITY, true);
    rr_param_cache_setup_entry(servo, APP_PARAM_VOLTAGE_INPUT, true);
    rr_param_cache_setup_entry(servo, APP_PARAM_CURRENT_INPUT, true);

    rr_param_cache_update(servo);

    for(int i = 0; i < APP_PARAM_SIZE; i++)
    {
        if(servo->pcache[i].activated)
        {
            printf("%d: %f\n", i, servo->pcache[i].value);
        }
    }

    rr_param_cache_setup_entry(servo, APP_PARAM_VOLTAGE_INPUT, false);

    rr_param_cache_update(servo);

    for(int i = 0; i < APP_PARAM_SIZE; i++)
    {
        if(servo->pcache[i].activated)
        {
            printf("%d: %f\n", i, servo->pcache[i].value);
        }
    }

    rr_sleep_ms(3000);
    rr_release(servo);
    rr_deinit_interface(&iface);

    fclose(log);

    return 0;
}
