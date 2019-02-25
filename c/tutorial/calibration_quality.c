#include "api.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <stdio.h>

void enable_compensation(rr_servo_t *servo, bool en)
{
	float value = en ? 1.0 : 0.0;
	if(rr_write_raw_sdo(servo, 0x41ff, 15, (uint8_t *)&value, sizeof(value), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't enable cogging map\n");
		exit(1);
	}

	if(rr_write_raw_sdo(servo, 0x41ff, 16, (uint8_t *)&value, sizeof(value), 1, 100) != RET_OK)
	{
		API_DEBUG("Can't enable friction map\n");
		exit(1);
	}
}

#define USEC_PER_SEC            1000000

int64_t calcdiff(struct timespec t1, struct timespec t2) 
{
  int64_t diff;
  diff = USEC_PER_SEC * (long long)((int) t1.tv_sec - (int) t2.tv_sec);
  diff += ((int) t1.tv_nsec - (int) t2.tv_nsec) / 1000;
  return diff;
}

int main(int argc, char *argv[])
{
	uint8_t id;
	struct timespec tprev, tnow, tstart;
	float pos, curr;
	FILE *f;

	if(argc == 3)
	{
		id = strtol(argv[2], NULL, 0);
	}
	else
	{
		API_DEBUG("Wrong format!\nUsage: %s interface id\n", argv[0]);
		return 1;
	}

	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	rr_servo_t *servo = rr_init_servo(iface, id);

	//! [Switching to operational state]
	rr_servo_set_state_operational(servo);
	//! [Switching to operational state]
	
	rr_nmt_state_t state = 0;
	for(int i = 0; i < 20; i++)
	{
		rr_sleep_ms(100);
		rr_servo_get_state(servo, &state);
		if(state == RR_NMT_OPERATIONAL)
		{
			break;
		}
	}
	if(state != RR_NMT_OPERATIONAL)
	{
		API_DEBUG("Can't switch tot operational mode\n");
		exit(1);
	}

	rr_param_cache_setup_entry (servo, APP_PARAM_POSITION_ROTOR, true);
	rr_param_cache_setup_entry (servo, APP_PARAM_CURRENT_PHASE, true);
	enable_compensation(servo, false);

	rr_set_velocity_motor(servo, 4.0);
	API_DEBUG("Setting 4 RPM @ motor without cogging comp.\n");
	API_DEBUG("Collecting data (~60sec) ...\n");

	f = fopen("without_comp.csv", "w+");

	fprintf(f, "time_us, mpos_deg, mcurr_amp\n");
	clock_gettime(CLOCK_REALTIME, &tnow);
	tprev = tnow;
	tstart = tnow;

	while(true)
	{
		clock_gettime(CLOCK_REALTIME, &tnow);
		rr_param_cache_update(servo);
		rr_read_cached_parameter(servo, APP_PARAM_POSITION_ROTOR, &pos);
		rr_read_cached_parameter(servo, APP_PARAM_CURRENT_PHASE, &curr);

		fprintf(f, "%" PRId64 ", %f, %f\n", calcdiff(tnow, tprev), pos, curr);

		if(calcdiff(tnow, tstart) >= 60000000)
		{
			break;
		}
	}

	fclose(f);

	enable_compensation(servo, true);

	rr_set_velocity_motor(servo, -4.0);
	API_DEBUG("Setting -4 RPM @ motor with cogging comp.\n");
	API_DEBUG("Collecting data (~60sec) ...\n");

	f = fopen("with_comp.csv", "w+");

	fprintf(f, "time_us, mpos_deg, mcurr_amp\n");
	clock_gettime(CLOCK_REALTIME, &tnow);
	tprev = tnow;
	tstart = tnow;

	while(true)
	{
		clock_gettime(CLOCK_REALTIME, &tnow);
		rr_param_cache_update(servo);
		rr_read_cached_parameter(servo, APP_PARAM_POSITION_ROTOR, &pos);
		rr_read_cached_parameter(servo, APP_PARAM_CURRENT_PHASE, &curr);

		fprintf(f, "%" PRId64 ", %f, %f\n", calcdiff(tnow, tprev), pos, curr);

		if(calcdiff(tnow, tstart) >= 60000000)
		{
			break;
		}
	}

	fclose(f);

	rr_release(servo);


}


