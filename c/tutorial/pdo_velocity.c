#ifndef WIN32
#define LINUX_RT_FEATURES
#endif

//have to be incleded first
#include "rt.h"
#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CYCLE_TIME_US 100000

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

void pdo_cb(rr_can_interface_t *iface, int id, rr_pdo_n_t pdo_n, int len, uint8_t *data)
{

	switch(pdo_n)
	{
		case TPDO0:
		{
			tpdo0_t pdo0;
			memcpy(&pdo0, data, len);
			printf("pos: %f deg, vel: %f deg/s, curr: %f A\n", pdo0.pos, pdo0.act_vel * 0.02, pdo0.act_curr * 0.0016);
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

void pdo_configure(rr_servo_t *s)
{
	rr_pdo_clear_map(s, RPDO2);
	rr_pdo_clear_map(s, RPDO3);
	rr_pdo_clear_map(s, TPDO2);
	rr_pdo_clear_map(s, TPDO3);
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

	pdo_configure(servo);

	rr_servo_set_state_operational(servo);

	//set cycle time, the servo will turn off if cycle time exceeded 1.5 times the nominal value
	if(high_prio)
	{
		//make sure we are running high priority process
		//process with generic priority may suffer from high jitter
		//and servo may go to pre-op state if cycly time violated
		rr_pdo_set_cycle_time(servo, CYCLE_TIME_US);
	}

	interval_sleep(0);	

	double ph = 0, f = 1.0;

	while(true)
	{
		rpdo0_t rpdo0 = 
		{
			.mode = 1, //velocity
			.iwin = 0,
			.des_vel = 1500 * sin(ph),
			.des_curr = 0
		};

		ph += 2.0 * M_PI * f * (1.0e-6 * CYCLE_TIME_US);
		ph -= ph > 2.0 * M_PI ? 2.0 * M_PI : 0;

		rr_send_pdo(iface, id, RPDO0, sizeof(rpdo0), (uint8_t *)&rpdo0);
		rr_send_pdo_sync(iface);

		interval_sleep(CYCLE_TIME_US * 1000);	
	}
}

