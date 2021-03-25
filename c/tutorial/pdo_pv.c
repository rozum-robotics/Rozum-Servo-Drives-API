#ifndef WIN32
#define LINUX_RT_FEATURES
#endif

//have to be incleded first
#include "rt.h"
#include "api.h"
#include "math_macro.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

//define control loop cycle time to 250Hz
const double dt = 1.0 / 250.0;

//PDO objects definition
typedef struct __attribute__((packed))
{
	uint8_t mode;
	uint8_t iwin;
	int16_t des_curr;
	float des_vel;
} rpdo0_t;

typedef struct __attribute__((packed))
{
	float pos;
	int16_t act_vel;
	int16_t act_curr;
} tpdo0_t;

typedef struct __attribute__((packed))
{
	uint8_t queue_fill;
} tpdo2_t;

typedef struct __attribute__((packed))
{
	float pos;
	float vel;
} pv_t; //PV object


//PDO configuration
void pdo_configure(rr_servo_t *s)
{
	rr_pdo_clear_map(s, RPDO2);
	rr_pdo_clear_map(s, RPDO3);
	rr_pdo_clear_map(s, TPDO2);
	rr_pdo_clear_map(s, TPDO3);
	rr_pdo_add_map(s, RPDO2, 0x5000, 0x07, 64);
	rr_pdo_add_map(s, RPDO3, 0x6040, 0x00, 16);
	rr_pdo_add_map(s, TPDO2, 0x5001, 0x0c, 8);
	rr_pdo_set_trans_type_sync(s, RPDO2, 1);
	rr_pdo_set_trans_type_sync(s, RPDO3, 1);
	rr_pdo_set_trans_type_sync(s, TPDO2, 1);
}

//PDO callback
void pdo_cb(rr_can_interface_t *iface, int id, rr_pdo_n_t pdo_n, int len, uint8_t *data)
{
    static int cnt = 0;
    static tpdo2_t pdo2;
	switch(pdo_n)
	{
		case TPDO0:
		    {
			    tpdo0_t pdo0;
			    memcpy(&pdo0, data, len);
			    printf("%d, %d, %f, %f, %f\n", 
			        cnt++, (int)pdo2.queue_fill, 
			        pdo0.pos, pdo0.act_vel * 0.02, pdo0.act_curr * 0.0016);
		    }
			break;
		case TPDO1:
			break;
		case TPDO2:
			memcpy(&pdo2, data, len);
			break;
		case TPDO3:
			break;
	}

	return;
}


//application entry point
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
   Binding process and NIC interrupt to the same CPU 
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
	
	float pd;
	
	if(rr_read_parameter(servo, APP_PARAM_POSITION, &pd) != RET_OK)
	{
		printf("error: can't read servo position\n");
	}
	
	rr_setup_pdo_callback(iface, pdo_cb);
	
	//reset cycle time
	rr_pdo_set_cycle_time(servo, 0);

    //reset communication (and cycle time errors if they are)	
	rr_servo_reset_communication(servo);

    //configure PDOs    
	pdo_configure(servo);
	
	rr_servo_set_state_operational(servo);
	
	//set cycle time
	rr_pdo_set_cycle_time(servo, 1.0e6 * dt);

	if(!high_prio)
	{
		printf("!!! WARNING: Setting of high priority for process has failed. Servo may work unstable.\n");
	}
	
	//set point position to actual one
	pv_t pv = {.pos = pd, .vel = 0};
	uint16_t cw = 1 << 4;
	double ph = 0;
	
	//preload one point
    rr_send_pdo(iface, id, RPDO2, sizeof(pv), (uint8_t *)&pv);
	rr_send_pdo_sync(iface);
	//start movement
  	rr_send_pdo(iface, id, RPDO3, sizeof(cw), (uint8_t *)&cw);
  	
    interval_sleep(0);

	while(true)
	{
	    rr_send_pdo(iface, id, RPDO2, sizeof(pv), (uint8_t *)&pv);
		rr_send_pdo_sync(iface);
 
    	double f = 0.5;
    	ph += 2.0 * M_PI * f * dt;
    	ph -= ph >= 2.0 * M_PI ? 2.0 * M_PI : 0;
    	pv.pos = 15.0 * cos(ph) - 15.0 + pd;
    	pv.vel = -15.0 * sin(ph) * 2.0 * M_PI * f;    	
		
		interval_sleep(1.0e9 * dt);	
	}
}


