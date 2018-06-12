#include "api.h"

#define SERVO_ID	111

int main(int argc, char *argv[])
{
	CanInterface_t iface;
	CanDevice_t d;

	api_initInterface(&iface, "192.168.0.123");
	api_initDevice(&iface, &d, SERVO_ID);
	api_sleepMs(1000);
	api_setVelocity(&d, 10.0);
	api_sleepMs(30000);
	api_stopAndRelease(&d);

	return 0;
}
