#ifndef __USB_CAN_TYPES_H__
#define __USB_CAN_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

typedef float float24;

#define USB_CAN_PUT_UX_DECL(b, bp, d, n_memb, type)\
int usb_can_put_ ## type(uint8_t *b, int bp, const type *d, int n_memb)

#define USB_CAN_PUT_UX(b, bp, d, n_memb, type)\
USB_CAN_PUT_UX_DECL(b, bp, d, n_memb, type)\
{\
	for(; n_memb--;)\
	{\
		_usb_can_memcpy(b + bp, d++, sizeof(type));\
		bp += sizeof(type);\
	}\
	return bp;\
}\

#define USB_CAN_GET_UX_DECL(b, bp, d, n_memb, type)\
int usb_can_get_ ## type(uint8_t *b, int bp, type *d, int n_memb)

#define USB_CAN_GET_UX(b, bp, d, n_memb, type)\
USB_CAN_GET_UX_DECL(b, bp, d, n_memb, type)\
{\
	for(; n_memb--;)\
	{\
		_usb_can_memcpy(d++, b + bp, sizeof(type));\
		bp += sizeof(type);\
	}\
	return bp;\
}\

USB_CAN_GET_UX_DECL(b, bp, d, n_memb, float);
USB_CAN_GET_UX_DECL(b, bp, d, n_memb, double);
USB_CAN_GET_UX_DECL(b, bp, d, n_memb, uint8_t);
USB_CAN_GET_UX_DECL(b, bp, d, n_memb, uint16_t);
USB_CAN_GET_UX_DECL(b, bp, d, n_memb, uint32_t);
USB_CAN_GET_UX_DECL(b, bp, d, n_memb, uint64_t);


USB_CAN_PUT_UX_DECL(b, bp, d, n_memb, float);
USB_CAN_PUT_UX_DECL(b, bp, d, n_memb, double);
USB_CAN_PUT_UX_DECL(b, bp, d, n_memb, uint8_t);
USB_CAN_PUT_UX_DECL(b, bp, d, n_memb, uint16_t);
USB_CAN_PUT_UX_DECL(b, bp, d, n_memb, uint32_t);
USB_CAN_PUT_UX_DECL(b, bp, d, n_memb, uint64_t);

int usb_can_get_float24(uint8_t *b, int bp, float24 *d, int n_memb);
int usb_can_put_float24(uint8_t *b, int bp, const float24 *d, int n_memb);

#ifdef __cplusplus
}
#endif

#endif


