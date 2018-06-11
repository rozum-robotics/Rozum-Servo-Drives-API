#include "usbcan_types.h"
#include "usbcan_util.h"

static void *_usb_can_memcpy(void *dest, const void *src, size_t n)
{
#ifndef USB_CAN_BIG_ENDIAN
	return memcpy(dest, src, n);
#else
#error "No big endian support yet"
#endif
}

USB_CAN_GET_UX(b, bp, d, n_memb, float)
USB_CAN_GET_UX(b, bp, d, n_memb, double)
USB_CAN_GET_UX(b, bp, d, n_memb, uint8_t)
USB_CAN_GET_UX(b, bp, d, n_memb, uint16_t)
USB_CAN_GET_UX(b, bp, d, n_memb, uint32_t)
USB_CAN_GET_UX(b, bp, d, n_memb, uint64_t)


USB_CAN_PUT_UX(b, bp, d, n_memb, float)
USB_CAN_PUT_UX(b, bp, d, n_memb, double)
USB_CAN_PUT_UX(b, bp, d, n_memb, uint8_t)
USB_CAN_PUT_UX(b, bp, d, n_memb, uint16_t)
USB_CAN_PUT_UX(b, bp, d, n_memb, uint32_t)
USB_CAN_PUT_UX(b, bp, d, n_memb, uint64_t)


int usb_can_get_float24(uint8_t *b, int bp, float *d, int n_memb)
{
	for(; n_memb--;)
	{
		*d = 0;
#ifndef USB_CAN_BIG_ENDIAN
		_usb_can_memcpy(((uint8_t *)d++) + 1, b + bp, sizeof(float) - 1);
#else
		_usb_can_memcpy(d++, b + bp, sizeof(float) - 1);
#endif
		bp += sizeof(float) - 1;
	}
	return bp;
}

int usb_can_put_float24(uint8_t *b, int bp, float *d, int n_memb)
{
	for(; n_memb--;)
	{
#ifndef USB_CAN_BIG_ENDIAN
		_usb_can_memcpy(b + bp, ((uint8_t *)d++) + 1, sizeof(float) - 1);
#else
		_usb_can_memcpy(b + bp, d++, sizeof(float) - 1);
#endif
		bp += sizeof(float) - 1;
	}
	return bp;
}




