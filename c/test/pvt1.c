#include "api.h"
#include <sys/time.h>
#include <math.h>

#define SERVO_ID 37

//#define M() printf("%d\n", __LINE__)
#define M()

#define TIME_DELTA_MS(x, y) ((x.tv_sec - y.tv_sec) * 1000 + (x.tv_usec - y.tv_usec) / 1000)

void nmt_cb(rr_can_interface_t *interface, int servo_id, rr_nmt_state_t nmt_state)
{
	printf("NMT CB\n");
}

void emcy_cb(rr_can_interface_t *interface, int servo_id, uint16_t code, uint8_t reg, uint8_t bits, uint32_t info)
{	
	printf("EMCY CB\n");
}

typedef struct
{
	float p;
	float v;
	float t;
} task_entry_t;


task_entry_t task[] = 
{
	{ 0.0,   0.0,  0.0},
	{ 5.0, 100.0, 120.0},
	{10.0, 100.0, 50.0},
	{15.0, 100.0, 50.0},
	{20.0, 100.0, 50.0},
	{25.0, 100.0, 50.0},
	{30.0, 100.0, 50.0},
	{35.0, 100.0, 50.0},
	{40.0, 100.0, 50.0},
	{45.0, 100.0, 50.0},
	{50.0, 130.0, 50.0},
	{55.0, 100.0, 50.0},
	{60.0, 100.0, 50.0},
	{65.0, 100.0, 50.0},
	{70.0, 100.0, 50.0},
	{75.0, 100.0, 50.0},
	{80.0, 100.0, 50.0},
	{85.0, 100.0, 50.0},
	{90.0, 0.0, 120.0},
};

/*
task_entry_t task[] = 
{
	{ 0.0,   0.0,  0.0},
	{ 45, 30.0, 2000.0},
	{ 90, 0.0, 2000.0},
	{ 45, -30.0, 2000.0},
	{0.0, 0.0, 2000.0},
};
*/


#define GO_HOME_SPD	30.0

int main(int argc, char *argv[])
{
	FILE *f;
    FILE *log;

	struct timeval ref, now;
	float p, v, t;
	float fin_time = 0;
	float pi;
	int e;

    log = fopen("comm.log", "w+");

    rr_set_debug_log_stream(stderr);

    rr_can_interface_t *iface = rr_init_interface("192.168.0.126");
	rr_setup_nmt_callback(iface, nmt_cb);
	rr_setup_emcy_callback(iface, emcy_cb);

    rr_servo_t *servo = rr_init_servo(iface, SERVO_ID);

    rr_param_cache_setup_entry(servo, APP_PARAM_CONTROLLER_POSITION_SETPOINT, true);
    rr_param_cache_setup_entry(servo, APP_PARAM_CONTROLLER_VELOCITY_SETPOINT, true);

	f = fopen("task.csv", "w");

	for(e = 0; e < sizeof(task) / sizeof(task_entry_t); e++)
	{
		fin_time += task[e].t;
		fprintf(f, "%f, %f, %f\n", task[e].p, task[e].v, fin_time);
		if(e == 0)
		{
			rr_read_parameter(servo, APP_PARAM_POSITION, &pi);

			rr_set_position_with_limits(servo, task[e].p, GO_HOME_SPD, 0);
			rr_sleep_ms(1000.0 * fabsf(pi - task[e].p) / GO_HOME_SPD + 100.0);

			rr_read_parameter(servo, APP_PARAM_POSITION, &pi);
			rr_clear_points_all(servo);
		}
		else
		{
			rr_add_motion_point(servo, pi + task[e].p, task[e].v, task[e].t);
		}
	}

	fclose(f);

	rr_start_motion(iface, 0);
	fin_time += 10;

	gettimeofday(&ref, NULL);
	now = ref;
	f = fopen("data.csv", "w");

	while(TIME_DELTA_MS(now, ref) <= fin_time)
	{
    	rr_param_cache_update(servo);
		gettimeofday(&now, NULL);
		t = TIME_DELTA_MS(now, ref);
		rr_read_cached_parameter(servo, APP_PARAM_CONTROLLER_POSITION_SETPOINT, &p);
		rr_read_cached_parameter(servo, APP_PARAM_CONTROLLER_VELOCITY_SETPOINT, &v);
		fprintf(f, "%f, %f, %f\n", p, v, t);
	}

	fclose(f);


    rr_sleep_ms(3000);
    rr_stop_and_release(servo);
    rr_deinit_interface(&iface);

    fclose(log);

    return 0;
}
