#ifndef RT_H__
#define RT_H__

#ifdef LINUX_RT_FEATURES
#define _GNU_SOURCE

#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define NSEC_PER_SEC            1000000000

static inline void tsnorm(struct timespec *ts) 
{
	while (ts->tv_nsec >= NSEC_PER_SEC)
	{
		ts->tv_nsec -= NSEC_PER_SEC;
		ts->tv_sec++;
	}
}

static inline int tsgreater(struct timespec *a, struct timespec *b) 
{
	return ((a->tv_sec > b->tv_sec) ||
			(a->tv_sec == b->tv_sec && a->tv_nsec > b->tv_nsec));
}

static inline int64_t tscalcdiff_ns(struct timespec t1, struct timespec t2) 
{
	int64_t diff;

	diff = NSEC_PER_SEC * ((int64_t)t1.tv_sec - (int64_t)t2.tv_sec);
	diff += (int64_t)t1.tv_nsec - (int64_t)t2.tv_nsec;
	return diff;
}

static inline bool set_process_priority(pthread_t thread, int p)
{
	int policy;
	struct sched_param schedparam;

	fprintf(stderr, "thread ID %lu\n", (unsigned long int)thread);
	
	if(pthread_getschedparam(thread, &policy, &schedparam) != 0)
	{
		fprintf(stderr, "can't get scheduler parameteres\n");
		return false;
	}
	fprintf(stderr, "current policy %d, current priority %d\n", policy, schedparam.sched_priority);
	policy = SCHED_RR;   
	schedparam.sched_priority = p;   

	pthread_setschedparam(thread, policy, &schedparam);

	pthread_getschedparam(thread, &policy, &schedparam);   

	fprintf(stderr, "sched policy: %d, priority (actual/requested): %d/%d\n", 
			policy, schedparam.sched_priority, p);


	if(schedparam.sched_priority != p)
	{
		return false;
	}

	return true;
}

static inline void set_process_niceness(int nr)
{
	int na = nice(nr - nice(0));

	fprintf(stderr, "niceness (actual/requested): %d/%d\n", na, nr);
}

static inline void set_latency_target(int target)
{
	struct stat s;
	int err;
	static int fd = -1;

	if(fd < 0)
	{
		    errno = 0;
		    err = stat("/dev/cpu_dma_latency", &s);
		    if(err == -1)
		    {
		            perror("stat");
		            return;
		    }

		    errno = 0;
		    fd = open("/dev/cpu_dma_latency", O_RDWR);
		    if(fd == -1)
		    {
		            perror("open");
		            return;
		    }
	}

	if(target >= 0)
	{
		    errno = 0;
		    uint32_t target_ = target;
		    err = write(fd, &target_, sizeof(target_));
		    if(err < 1)
		    {
		            perror("write");
		            close(fd);
		            fd = -1;
		            return;
		    }
	}
	else
	{
		close(fd);
		fd = -1;
		    return;
		
	}
}

static inline void set_cpu_affinity(uint32_t mask)
{
	cpu_set_t set;	                   
	CPU_ZERO(&set);
	for(uint32_t b = 1, i = 0; i < 32; i++, b <<= 1)
	{
		if(b & mask)
		{
			CPU_SET(i, &set);
		}
	}

	if(sched_setaffinity(getpid(), sizeof(set), &set) == -1)
	{
		printf("!!! WARNING: can't set CPU affinity\n");
	}
}

static inline void set_nic_irq_affinity(const char *nic, uint32_t mask)
{
	char *path;
	DIR *dir;
	struct dirent *entry;

	if(asprintf(&path, "/sys/class/net/%s/device/msi_irqs", nic) < 0)
	{
		printf("!!! ERROR: allocation error\n");
		return;
	}

	if(!(dir = opendir(path)))
	{
		printf("!!! ERROR: No MSI IRQs found for specified NIC\n");
		free(path);
		return;
	}

	while((entry = readdir(dir)) != NULL)
	{
		char *irq;
		char *name;
		struct stat s;
		
		if(asprintf(&name, "%s/%s", path, entry->d_name) < 0)
		{
			printf("!!! ERROR: allocation error\n");
			break;
		}

		stat(name, &s);
		free(name);
		if(S_ISDIR(s.st_mode))
		{
			continue;
		}
		
		if(asprintf(&irq, "/proc/irq/%s/smp_affinity", entry->d_name) < 0)
		{
			printf("!!! ERROR: allocation error\n");
			break;
		}

		FILE *fa = fopen(irq, "wb");
		free(irq);
		
		if(!fa)
		{
			printf("!!! ERROR: Can't open MSI IRQ affinity file\n");
			break;
		}
		
		fprintf(fa, "%x", mask);
		fclose(fa);
	}

	free(path);
	closedir(dir);
}

#endif


//sleep function to produce exact mean long term cycle time
#ifdef LINUX_RT_FEATURES
/*
    Make init. step if (ns == 0).
    It uses absolute sleep feature of linux kernel to
    produce stable matched cycle time.
*/
static inline void interval_sleep(uint64_t ns)
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

#ifdef WIN32
#include "windows.h"
#endif

#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>

static inline void interval_sleep(uint64_t ns)
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
    }
    t0 = t1;
#ifdef WIN32
	Sleep(ns / 1000000 + a / 1000);
#else
	usleep(ns / 1000 + a);
#endif
	first = false;
}
#endif

#endif
