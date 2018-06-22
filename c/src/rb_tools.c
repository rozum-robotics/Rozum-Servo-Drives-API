#include "rb_tools.h"

int rb_to_rb(uint8_t *dst, int dst_p, int dst_sz, uint8_t *src, int src_p, int src_sz, int n)
{

	dst_p = (dst_p + dst_sz) % dst_sz;
	src_p = (src_p + src_sz) % src_sz;

	for(; n--;)
	{
		dst[dst_p] = src[src_p];
		dst_p = (dst_p + 1) % dst_sz;
		src_p = (src_p + 1) % src_sz;

	}
	
	return dst_p;
}

int rb_dist(int h, int t, int sz)
{
	return (h - t + sz) % sz;
}
