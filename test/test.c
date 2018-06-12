#include "api.h"

#define SERVO_ID	123

int main(int argc, char *argv[])
{
	FILE *log = fopen("comm.log", "w+");

	api_setDebugLogStream(stderr);
	CanInterface_t iface = api_initInterface("/dev/ttyS3");
	api_setCommLogStream(iface, log);

	CanDevice_t d[10];
   
	for(int i = 0; i < 10; i++)
	{
		d[i] = api_initDevice(iface, i + 1);
	}


	api_deinitDevice(&d[4]);
	api_deinitDevice(&d[1]);
	api_deinitDevice(&d[8]);

	api_sleepMs(1000);
	api_setVelocity(d[0], 10.0);
	api_sleepMs(30000);
	api_stopAndRelease(d);

	
	api_deinitInterface(&iface);

	fclose(log);

	return 0;
}
