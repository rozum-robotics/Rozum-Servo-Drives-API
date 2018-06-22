#include "usbcan_util.h"

uint32_t hexstr_to_int(uint8_t *src, int l)
{
	uint32_t res = 0;

	for(; l--;)
	{
		res <<= 4;
		res |= HEXTOBIN(*src++);
	}
	return res;
}

uint64_t get_ux_(uint8_t *d, int *p, int x)
{
	uint64_t v = 0;
	for(; x--;)
	{
		v <<= 8;
		v |= d[*p];
		(*p)++;
	}
	return v;
}

uint64_t get_ix_(uint8_t *d, int *p, int x)
{
	uint64_t v = d[*p] & 0x80u ? -1 : 0;
	for(; x--;)
	{
		v <<= 8;
		v |= d[*p];
		(*p)++;
	}
	return v;
}

void set_ux_(uint8_t *d, int *p, int x, uint64_t v)
{
	for(; x--;)
	{
		d[*p] = (v >> (8 * x)) & 0xffu;
		(*p)++;
	}
}

void msleep(uint32_t ms)
{
    usleep(ms * 1000);
}

