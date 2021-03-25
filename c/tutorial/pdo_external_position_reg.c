/* 
This example shows how to control servo by position with external position controller.
Servo itself runs in velocity mode. This program can be easily extended to number of devices 
making some synchronized movements.

Servo will move to desired position (actual position + TDIST) and back.
*/

#ifndef WIN32
#define LINUX_RT_FEATURES
#endif

//have to be incleded first
#include "rt.h"
#include "api.h"
#include "pprof.h"
#include "math_macro.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

/*
Here is definition of PDO objects to communicate with servo
*/
typedef struct
{
	uint8_t mode;
	uint8_t iwin;
	int16_t des_curr;
	float des_vel;
} rpdo0_t; //to servo

typedef struct
{
	float pos;
	int16_t act_vel;
	int16_t act_curr;
} tpdo0_t; //from servo

//profiler instance
pprof_t p;
//set profiler config
const double VMAX = 50.0, AMAX = 70;
//set proportional position regulator gain
//position regulator will be simply P-regulator
const double PKP = 10.0;
//define contro; loop cycle time to 200Hz
const double dt = 1.0 / 200.0;
//storage for desired position
float pd;
//setup travel distance
const double TDIST = 300.0;

//small state machine to control desired position
typedef enum
{
	ST_START_DELAY,
	ST_GO_FORWARD,
	ST_MIDLE_DELAY,
	ST_GO_BACK,
	ST_FINAL_DELAY,
	ST_FINISHED
} state_t;
state_t state = 0;
double dly_cntr = 0;

/*
Callback function for receiving incoming PDO
*/
void pdo_cb(rr_can_interface_t *iface, int id, rr_pdo_n_t pdo_n, int len, uint8_t *data)
{
	//desired position delayed for 1 and 2 cycles
	//this crucial to use delayed position when calculating error
	//because PDOs always delayed one cycle at send and one at receive
	static double p_d1, p_d2;

	//callback counter (used for variables initialization)
	static int cntr = 0;
	
	//calculate time between callback calls
	static struct timeval t0;
	struct timeval t1;

	gettimeofday(&t1, 0);
	if(cntr == 0)
	{
		t0 = t1;
	}
	double dt = t1.tv_sec - t0.tv_sec + 1.0e-6 * (t1.tv_usec - t0.tv_usec); 
	t0 = t1;

	switch(pdo_n)
	{
		case TPDO0:
		{
			tpdo0_t tpdo0;
			memcpy(&tpdo0, data, len);

			//profiler needs some oversampling for better performance
			for(int i = 0; i < 100; i++)
			{
				pprof_process(&p, pd, VMAX, AMAX, dt / 100.0);
			}

			//calculate velocity setpoint: profiler velocity command plus Kp * pos_error
			double des_vel = p.v + PKP * (p_d2 - tpdo0.pos) * (cntr > 1 ? 1.0 : 0.0);

			//prepare PDO for transmitting
			rpdo0_t rpdo0 = 
			{
				.mode = 1, //velocity
				.iwin = 0,
				.des_vel = des_vel / 0.06, //converting deg/s to RPMs on fast shaft (gear ratio 100)
				.des_curr = 0
			};

			rr_send_pdo(iface, id, RPDO0, sizeof(rpdo0), (uint8_t *)&rpdo0);

			printf("%f, %f, %f, %f, %f\n", 
					p_d2, p.v,
					tpdo0.pos, tpdo0.act_vel * 0.02, tpdo0.act_curr * 0.0016);


			p_d2 = p_d1;
			p_d1 = p.p;

			//process state machine
			switch(state)
			{
				case ST_START_DELAY:
					if(dly_cntr < 0.5)
					{
						dly_cntr += dt;
					}
					else
					{
						dly_cntr = 0;
						state = ST_GO_FORWARD;
						pd += TDIST;
					}
					break;
				case ST_GO_FORWARD:
					if(!pprof_is_running(&p))
					{
						state = ST_MIDLE_DELAY;
					}
					break;
				case ST_MIDLE_DELAY:
					if(dly_cntr < 0.5)
					{
						dly_cntr += dt;
					}
					else
					{
						dly_cntr = 0;
						state = ST_GO_BACK;
						pd -= TDIST;
					}
					break;
				case ST_GO_BACK:
					if(!pprof_is_running(&p))
					{
						state = ST_FINAL_DELAY;
					}
					break;
				case ST_FINAL_DELAY:
					if(dly_cntr < 0.5)
					{
						dly_cntr += dt;
					}
					else
					{
						state = ST_FINISHED;
					}
					break;
				default:
					break;

			}
			cntr++;
		}
		break;
		case TPDO1:
		break;
		case TPDO2:
		break;
		case TPDO3:
		break;
	}

	return;
}

int main(int argc, char *argv[])
{
	bool high_prio = false;
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
	rr_can_interface_t *iface = rr_init_interface(argv[1]);
	if(!iface)
	{
		API_DEBUG("Interface init error\n");
		return 1;
	}

	rr_servo_t *servo = rr_init_servo(iface, id);
	if(!servo)
	{
		API_DEBUG("Servo init error\n");
		return 1;
	}

#ifdef LINUX_RT_FEATURES
/*
   For real effect from RT features it's recommended to
   use RT kernel patch together with 
   'isolcpus = {your-preffered-cpu-number}' kernel option.
*/
#define CPU_N 0
/*
   When working through an ethernet adapter set this 
   field to your network interface name.
   The binding process and NIC interrupt to the same CPU 
   removes unnecessary inter-CPU data exchange thus 
   reducing latency and cycle time stability.
*/
#define NIC_NAME "enp59s0"
    //disable CPU sleep states
	set_latency_target(0);
	//bind process to specified CPU
	set_cpu_affinity(1 << CPU_N);
	//bind network card interrupt to specified CPU
	set_nic_irq_affinity(NIC_NAME, 1 << CPU_N);
	//set lowest process niceness
	set_process_niceness(-20);
	//set process priority to some high value
	high_prio = set_process_priority(pthread_self(), 98);
#endif

	rr_setup_pdo_callback(iface, pdo_cb);

	//reset cycle time
	rr_pdo_set_cycle_time(servo, 0);

    //reset communication (and cycle time errors if they are)	
	rr_servo_reset_communication(servo);

	rr_servo_set_state_operational(servo);

	//read current servo position
	if(rr_read_parameter(servo, APP_PARAM_POSITION, &pd) != RET_OK)
	{
		printf("error: can't read servo position\n");
	}

	float vel_rate_orig;
	//get current velocity rate to restore later
	rr_get_velocity_rate(servo, &vel_rate_orig);
	//set very high velocity rate, profiler will take care of changing velocity with correct rate
	rr_set_velocity_rate(servo, 1e6);

	//prepare profiler
	pprof_init(&p);
	pprof_set(&p, pd, 0);

	//set cycle time, the servo will turn off if cycle time exceeded 1.5 times the nominal value
	if(high_prio)
	{
		//make sure we are running high priority process
		//process with generic priority may suffer from high jitter
		//and servo may go to pre-op state if cycly time violated
		rr_pdo_set_cycle_time(servo, 1.0e6 * dt);
	}

	interval_sleep(0);	

	//start sending sync with rate of control loop
	while(state != ST_FINISHED)
	{
		rr_send_pdo_sync(iface);

		interval_sleep(1.0e9 * dt);	
	}


	rr_set_velocity_rate(servo, vel_rate_orig);
}
