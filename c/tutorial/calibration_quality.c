/**
 * @brief Calibration quality
 * @file calibration_quality.c
 * @author Rozum
 * @date 2018-07-11
 */

#include "api.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <stdio.h>

/**
 * \defgroup tutor_c_calibration_quality Checking calibration quality
 *The tutorial describes how to verify the results of the calibration to mitigate cogging effects.
 *The verification is by reading rotor position and phase current at 4 RPM and -4RPM into CSV files.
 *Based on the files, dependance curves are built, making it possible to evaluate whether current settings
 *of the cogging compensation table provide the required mitigation effect.
 *
 *1. Read the parameters as required to run the tutorial to check the calibration quality.
 *\snippet calibration_quality.c Read tutorial param2
 *
 *2. Initialize the interface.
 *\snippet calibration_quality.c Init interface32
 *
 *3. Initialize the servo.
 *\snippet calibration_quality.c Init servo32
 *
 *4. Set the servo to the operational state.
 *\snippet calibration_quality.c Switching to operational
 *
 *5. Set up a parameter array to read, comprising:
 *<ul><li>rotor position</li>
 *<li>phase current</li></ul>
 *\snippet calibration_quality.c Set up param entry
 *
 *6. Disable the cogging compensation table.
 *\snippet calibration_quality.c Disable cog table
 *
 *7. Set the servo to rotate at 4 RPM.
 *\snippet calibration_quality.c Set velocity1
 *
 *8. Open a CSV file to save the measured values of the parameters you set at Step 5.
 *\snippet calibration_quality.c Open file1
 *
 *9. Record the start time of the measurements.
 *\snippet calibration_quality.c Record time1
 *
 *10. Start a measurement cycle, saving measured values. The total measurement cycle time is 1 minute.
 *\snippet calibration_quality.c Measure param1
 *
 *11. Close the CSV file you opened at Step 8.
 *\snippet calibration_quality.c Close file1
 *
 *12. Enable the cogging compensation table.
 *\snippet calibration_quality.c Enable cog table
 *
 *13. Set the servo to rotate at -4RPM.
 *\snippet calibration_quality.c Set velocity2
 *
 *14. Open a new CSV file to save measurements.
 *\snippet calibration_quality.c Open file2
 *
 *15. Record the start time of the measurements.
 *\snippet calibration_quality.c Record time2
 *
 *16. Start a new measurement cycle, saving measured values. The total measurement cycle time is 1 minute.
 *\snippet calibration_quality.c Measure param2
 
 *17. Close the CSV file you opened at Step 14.
 *\snippet calibration_quality.c Close file2
 *
 *As a result, you get two CSV files with measured rotor position ad phase current. Use the files to build dependance curves
 *to evaluate whether current settings of the cogging compensation table have provided the required mitigation effect.
 *
 *18. Set the servo to the released state.
 *\snippet calibration_quality.c Release
 *
 * <b> Complete tutorial code: </b>
 * \snippet calibration_quality.c calibration_quality_code_full
 */
//! [calibration_quality_code_full]
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
//! [Read tutorial param2]
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
//! [Read tutorial param2]
	//! [Init interface32]
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	//! [Init interface32]
	//! [Init servo32]
	rr_servo_t *servo = rr_init_servo(iface, id);
	//! [Init servo32]
	//! [Switching to operational]
	rr_servo_set_state_operational(servo);
		
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
    //! [Switching to operational]
	//! [Set up param entry]
	rr_param_cache_setup_entry (servo, APP_PARAM_POSITION_ROTOR, true);
	rr_param_cache_setup_entry (servo, APP_PARAM_CURRENT_PHASE, true);
	//! [Set up param entry]
	//! [Disable cog table]
	enable_compensation(servo, false);
	//! [Disable cog table]
	//! [Set velocity1]
	rr_set_velocity_motor(servo, 4.0);
	API_DEBUG("Setting 4 RPM @ motor without cogging comp.\n");
	API_DEBUG("Collecting data (~60sec) ...\n");
	//! [Set velocity1]
 	//! [Open file1]
	f = fopen("without_comp.csv", "w+");
	//! [Open file1]
	
	//! [Record time1]
	fprintf(f, "time_us, mpos_deg, mcurr_amp\n");
	clock_gettime(CLOCK_REALTIME, &tnow);
	tprev = tnow;
	tstart = tnow;
	//! [Record time1]
	
	//! [Measure param1]
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
   	//! [Measure param1]
	
	//! [Close file1]
	fclose(f);
    //! [Close file1]
	
	//! [Enable cog table]
	enable_compensation(servo, true);
	//! [Enable cog table]
	
	//! [Set velocity2]
	rr_set_velocity_motor(servo, -4.0);
	API_DEBUG("Setting -4 RPM @ motor with cogging comp.\n");
	API_DEBUG("Collecting data (~60sec) ...\n");
	//! [Set velocity2]
	
	//! [Open file2]
	f = fopen("with_comp.csv", "w+");
	//! [Open file2]
	
	//! [Record time2]
	fprintf(f, "time_us, mpos_deg, mcurr_amp\n");
	clock_gettime(CLOCK_REALTIME, &tnow);
	tprev = tnow;
	tstart = tnow;
	//! [Record time2]
	
	//! [Measure param2]
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
	//! [Measure param2]
	
	//! [Close file2]
	fclose(f);
	//! [Close file2]
	
	//! [Release]
	rr_release(servo);
	//! [Release]
		    }
//! [calibration_quality_code_full]


