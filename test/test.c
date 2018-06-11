#include <unistd.h>
#include "api.h"

#define SERVO_ID	111

int main(int argc, char *argv[])
{
	CanInterface_t iface;
	CanDevice_t d;

	printf("init interface\n");
	api_initInterface(&iface, "192.168.0.123");
	printf("init device\n");
	api_initDevice(&iface, &d, SERVO_ID);

	
	sleep(1);

	printf("set current\n");
	api_setCurrent(&d, 3.0);

	sleep(30);
	api_setCurrent(&d, 0.0);
	sleep(1);

	return 0;
}
