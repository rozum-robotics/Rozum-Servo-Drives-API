#ifndef WIN32
//#define LINUX_RT_FEATURES
#endif

#ifdef LINUX_RT_FEATURES
#define _GNU_SOURCE
#endif

#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#ifdef LINUX_RT_FEATURES
#include "rt.h"
#endif

#include <math.h>

//helpfull math macros
#define MAX(a, b)               (((a) > (b)) ? (a) : (b))
#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#define SQ(a)                   ((a) * (a))
#define ABS(a)                  ( ((a) < 0) ? -(a) : (a) )
#define CLIP(a, l, h)  			( MAX((MIN((a), (h))), (l)) )
#define SIGN(a)                 ( ((a) > 0) ? 1 : (((a) == 0) ? 0 : -1) )


//sleep function to produce exact mean long term cycle time
#ifdef LINUX_RT_FEATURES
/*
    Make init. step if (ns == 0).
    It uses absolute sleep feature of linux kernel to
    produce stable matched cycle time.
*/
void interval_sleep(uint64_t ns)
{
	static struct timespec tprev, tnow, tnext;
		
	if(!ns)
	{
    	clock_gettime(CLOCK_REALTIME, &tprev);
	    tnext = tprev;
	    return;
	}

    tnext.tv_nsec += ns;
	tsnorm(&tnext);
	clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &tnext, NULL);
	clock_gettime(CLOCK_REALTIME, &tnow);
}

#else
/*
    Make init. step if (ns == 0).
    We build this I-regulator to tune cycle time using regular delay when 
    absolute delay unavailable.
*/
void interval_sleep(uint64_t ns)
{
	double d;
    static double a = 0;
    static struct timeval t0, t1;
    static bool first = true;  
    
    if(!ns)
    {
        a = 0;
        gettimeofday(&t0, 0);
        first = true;
        return;
    }

    gettimeofday(&t1, 0);
    d = 1.0e6 * (t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec);
    if(!first)
    {
        a = a + (1.0e-3 * ns - d) * 0.0000001;
		//fprintf(stderr, "%f, %f\n", d, a);
    }
    t0 = t1;
	rr_sleep_ms(ns / 1000000 + a / 1000);
	first = false;
}
#endif

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
void pdo_configure(rr_servo_t *servo)
{
	uint32_t rpdo2_cobid, rpdo3_cobid, tpdo2_cobid;
	int l = 4;
	if(rr_read_raw_sdo(servo, 0x1402, 1, (uint8_t *)&rpdo2_cobid, &l, 1, 100) != RET_OK) exit(1);
	if(rr_read_raw_sdo(servo, 0x1403, 1, (uint8_t *)&rpdo3_cobid, &l, 1, 100) != RET_OK) exit(1);
	if(rr_read_raw_sdo(servo, 0x1802, 1, (uint8_t *)&tpdo2_cobid, &l, 1, 100) != RET_OK) exit(1);
	//destroy PDOs
	rpdo2_cobid |= 0x80000000ul;
	rpdo3_cobid |= 0x80000000ul;
	tpdo2_cobid |= 0x80000000ul;
    if(rr_write_raw_sdo(servo, 0x1402, 1, (uint8_t *)&rpdo2_cobid, 4, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1403, 1, (uint8_t *)&rpdo3_cobid, 4, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1802, 1, (uint8_t *)&tpdo2_cobid, 4, 1, 100) != RET_OK) exit(1);
	
	//setup mappings
	uint8_t obj_cnt = 0;
	uint32_t rpdo2_map = 0x50000740, rpdo3_map = 0x60400010;
	uint32_t tpdo2_map1 = 0x50010c08;
	if(rr_write_raw_sdo(servo, 0x1602, 0, (uint8_t *)&obj_cnt, 1, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1603, 0, (uint8_t *)&obj_cnt, 1, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1A02, 0, (uint8_t *)&obj_cnt, 1, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1602, 1, (uint8_t *)&rpdo2_map, 4, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1603, 1, (uint8_t *)&rpdo3_map, 4, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1A02, 1, (uint8_t *)&tpdo2_map1, 4, 1, 100) != RET_OK) exit(1);
	
	obj_cnt = 1;
	if(rr_write_raw_sdo(servo, 0x1602, 0, (uint8_t *)&obj_cnt, 1, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1603, 0, (uint8_t *)&obj_cnt, 1, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1A02, 0, (uint8_t *)&obj_cnt, 1, 1, 100) != RET_OK) exit(1);
	
	//create PDOs
	rpdo2_cobid &= ~0x80000000ul;
	rpdo3_cobid &= ~0x80000000ul;
	tpdo2_cobid &= ~0x80000000ul;
    if(rr_write_raw_sdo(servo, 0x1402, 1, (uint8_t *)&rpdo2_cobid, 4, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1403, 1, (uint8_t *)&rpdo3_cobid, 4, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1802, 1, (uint8_t *)&tpdo2_cobid, 4, 1, 100) != RET_OK) exit(1);
	
	uint8_t pdo_mode = 1;
    if(rr_write_raw_sdo(servo, 0x1402, 2, (uint8_t *)&pdo_mode, 1, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1403, 2, (uint8_t *)&pdo_mode, 1, 1, 100) != RET_OK) exit(1);
	if(rr_write_raw_sdo(servo, 0x1802, 2, (uint8_t *)&pdo_mode, 1, 1, 100) != RET_OK) exit(1);
}


//PDO callback
void pdo_cb(rr_can_interface_t *interface, int id, rr_pdo_n_t pdo_n, int len, uint8_t *data)
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
		    {
			    
			    memcpy(&pdo2, data, len);
			}
			break;
		case TPDO3:
			break;
	}

	return;
}


//application entry point
int main(int argc, char *argv[])
{
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
	set_process_priority(pthread_self(), 98);
#endif
	
	float pd;
	
	if(rr_read_parameter(servo, APP_PARAM_POSITION, &pd) != RET_OK)
	{
		printf("error: can't read servo position\n");
	}
	
	rr_setup_pdo_callback(iface, pdo_cb);
	
	//reset cycle time
	uint32_t dt_us = 0;
	if(rr_write_raw_sdo(servo, 0x1006, 0, (uint8_t *)&dt_us, sizeof(dt_us), 1, 100) != RET_OK) exit(1);

    //reset communication (and cycle time errors if they are)	
	rr_servo_reset_communication(servo);

    //configure PDOs    
	pdo_configure(servo);
	
	rr_servo_set_state_operational(servo);
	
	//set cycle time
	dt_us = 10000;
	if(rr_write_raw_sdo(servo, 0x1006, 0, (uint8_t *)&dt_us, sizeof(dt_us), 1, 100) != RET_OK) exit(1);
	
	//set point position to actual one
	pv_t pv = {.pos = pd, .vel = 0};
	uint16_t cw = 1 << 4;
	double ph = 0, dt = dt_us * 1.0e-6;
	
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
		
		interval_sleep(dt_us * 1000);	
	}
}


