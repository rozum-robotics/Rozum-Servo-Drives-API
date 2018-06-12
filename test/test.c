#include "api.h"

#define SERVO_ID	123

#define M() printf("%d\n", __LINE__)

int main(int argc, char *argv[])
{
	FILE *log = fopen("comm.log", "w+");

	api_setDebugLogStream(stderr);
	CanInterface_t iface = api_initInterface("/dev/ttyS3");
	api_setCommLogStream(iface, stderr);

	CanDevice_t d[10];
   
	for(int i = 0; i < 10; i++)
	{
		d[i] = api_initDevice(iface, i + 1);
	}


	api_deinitDevice(&d[4]);
	api_deinitDevice(&d[1]);
	api_deinitDevice(&d[8]);

	M();
	api_sleepMs(1000);
	M();
	api_setVelocity(d[0], 10.0);
	M();
	api_sleepMs(3000);
	M();
	api_stopAndRelease(d[0]);

	M();
	
	api_deinitInterface(&iface);

	M();
	fclose(log);

	return 0;
}
