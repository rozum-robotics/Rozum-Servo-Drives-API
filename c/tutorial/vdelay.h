#ifndef __VDELAY_H__
#define __VDELAY_H__

#include "math_macro.h"

#define VDELAY_MAX_LEN	16

typedef struct
{
	//internals
	int ptr;
	double d[VDELAY_MAX_LEN];
	double z;
	//output variables
	double y;
} vdelay_t;


static inline void vdelay_init(vdelay_t *inst, double u, double z)
{
	inst->ptr = 0;
	inst->y = u;
	inst->z = z;
		
	for(int i = 0; i < VDELAY_MAX_LEN; i++)
	{
		inst->d[i] = u;
	}
}

static inline double vdelay_process(vdelay_t *inst, double u)
{
	inst->d[inst->ptr] = u;
	double z = inst->z;

	z = CLIP(z, 0.0, VDELAY_MAX_LEN - 1);	
	double k = z - floor(z);
	int p0 = (inst->ptr - ((int)z) + VDELAY_MAX_LEN) % VDELAY_MAX_LEN;
	int p1 = (p0 - 1 + VDELAY_MAX_LEN) % VDELAY_MAX_LEN;
	
	inst->y = inst->d[p0] * (1.0 - k) + inst->d[p1] * k;
	inst->ptr = (inst->ptr + 1) % VDELAY_MAX_LEN;

	return inst->y;
}

#endif
