#include "api.h"

#define SERVO_ID	123

#define M() printf("%d\n", __LINE__)

int main(int argc, char *argv[])
{
	FILE *log = fopen("comm.log", "w+");

	rr_set_debug_log_stream(stderr);
	rr_can_interface_t *iface = rr_init_interface("192.168.0.124");
	rr_set_comm_log_stream(iface, stderr);

	rr_servo_t *d[10];
   
	for(int i = 0; i < 10; i++)
	{
		d[i] = rr_init_servo(iface, i + SERVO_ID);
	}


	M();
	rr_deinit_servo(&d[4]);
	M();
	rr_deinit_servo(&d[1]);
	M();
	rr_deinit_servo(&d[8]);

	M();
	rr_sleep_ms(1000);
	M();
	rr_set_velocity(d[0], 10.0);
	M();
	rr_sleep_ms(3000);
	M();
	rr_stop_and_release(d[0]);

	M();
	
	rr_deinit_interface(&iface);

	M();
	fclose(log);

	return 0;
}
