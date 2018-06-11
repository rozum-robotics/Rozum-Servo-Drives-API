#include "api.h"


int main(int argc, char *argv[])
{

	api_initInterface("/dev/ttyACM0");
	return 0;
}
