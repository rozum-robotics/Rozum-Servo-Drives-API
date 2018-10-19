#ifndef __USBCAN_UTIL_H__
#define __USBCAN_UTIL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef PI
#define PI 3.141592653589793f
#endif

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define SQ(a)			 	((a) * (a))
#define ABS(a) 				( ((a) < 0) ? -(a) : (a) )
#define DBAND(a, l, h) 		( (((a) <= (h)) && ((a) >= (l))) ? 0 : (a) )
#define CLIP(a, l, h) 		( MAX((MIN((a), (h))), (l)) )
#define CLIPS(a, m) 		( MAX((MIN((a), (m))), (-(m))) )
#define CLIPL(a, l) 		( ((a) < (l)) ? (l) : (a))
#define CLIPH(a, h) 		( ((a) > (h)) ? (h) : (a))
#define SIGN(a) 			( ((a) > 0) ? 1 : (((a) == 0) ? 0 : -1) )
#define DIRECTION(a) 		( ((a) >= 0) ? 1 : -1 )
#define INRANGE(a, l, h) 	( (((a) > (l)) && ((a) <= (h))) || ((a) == (l)) )
#define ROUND(a) 			( ABS((a) - ((int)(a))) >= 0.5 ? ((int)(a)) + SIGN(a) : (int)(a) )
#define DECAY(a, l, h)     	CLIP((((a) - (l)) / ((h) - (l))), 0.0, 1.0)
#define INV_DECAY(a, l, h) 	CLIP((((h) - (a)) / ((h) - (l))), 0.0, 1.0)
#define DEG_TO_RAD(a)		((a) * PI / 180.0f)
#define RAD_TO_DEG(a)		((a) * 180.0f / PI)
#define LOGIC_XOR(a, b)		(((a) || (b)) && !((a) && (b)))


 
#define U16_H8(u)		(((u) >> 8) & 0xffu)
#define U16_L8(u)		(((u) >> 0) & 0xffu)
#define U32_H8(u)		(((u) >> 24) & 0xffu)
#define U32_MH8(u)		(((u) >> 16) & 0xffu)
#define U32_ML8(u)		(((u) >> 8) & 0xffu)
#define U32_L8(u)		(((u) >> 0) & 0xffu)

#define HEXTOBIN(h) ((((h) & 0xdf) - 16) % 39)


uint32_t hexstr_to_int(uint8_t *src, int l);
uint64_t get_ux_(uint8_t *d, int *p, int x);
uint64_t get_ix_(uint8_t *d, int *p, int x);
void set_ux_(uint8_t *d, int *p, int x, uint64_t v);

void msleep(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif
