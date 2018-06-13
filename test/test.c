#include "api.h"

#define SERVO_ID	99

#define M() printf("%d\n", __LINE__)

int main(int argc, char *argv[])
{
	FILE *log = fopen("comm.log", "w+");

	M();
	rr_set_debug_log_stream(stderr);

	M();
	rr_can_interface_t *iface = rr_init_interface("192.168.0.124");
	//rr_set_comm_log_stream(iface, stderr);

	M();
	rr_servo_t *servo = rr_init_servo(iface, SERVO_ID);
   
	M();
	float vdc;
	rr_read_parameter(servo, APP_PARAM_VOLTAGE_INPUT, &vdc);

	printf("voltage: %f\n", vdc);

	M();
	rr_sleep_ms(3000);
	M();
	rr_stop_and_release(servo);

	M();
	rr_deinit_interface(&iface);

	M();
	fclose(log);

	return 0;
}
