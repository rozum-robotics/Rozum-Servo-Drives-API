#ifndef __RB_TOOLS_H__
#define __RB_TOOLS_H__

#include <stdint.h>

int rb_to_rb(uint8_t *dst, int dst_p, int dst_sz, uint8_t *src, int src_p, int src_sz, int n);
int rb_dist(int h, int t, int sz);


#endif
