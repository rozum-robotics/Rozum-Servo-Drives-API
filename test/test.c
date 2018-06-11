#include "api.h"

#define SERVO_ID	99

int main(int argc, char *argv[])
{
	CanInterface_t iface;
	CanDevice_t d;

	api_initInterface(&iface, "/dev/ttyACM0");
	api_initServo(&iface, &d, SERVO_ID);

	return 0;
}
