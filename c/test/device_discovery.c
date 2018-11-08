/**
 * @brief Tutorial example of reading device parameters
 * 
 * @file read_any_param.c
 * @author your name
 * @date 2018-06-25
 */

#include "api.h"
#include "../src/usbcan_proto.h"
#include "../src/usbcan_util.h"
#include "../src/logging.h"

#include <stdlib.h>
 
int main(int argc, char *argv[])
{
	debug_log = stdout;
		
	if(argc < 3)
	{
		printf("Usage: %s port out_file\n", argv[0]);
		exit(1);
	}
    
	usbcan_instance_t *inst = usbcan_instance_init(argv[1]);
	
	if(!inst)
	{
		exit(1);
		LOG_ERROR(debug_log, "Can't create usbcan instance");
	}
	
	FILE *f = fopen(argv[2], "w");
	
	if(!f)
	{
		LOG_ERROR(debug_log, "Can't open output file");
	}
	
	LOG_INFO(debug_log, "Discovering devices ...");
	msleep(5000);
	
	for(int i = 0; i < USB_CAN_MAX_DEV; i++)
	{
		if(inst->dev_alive[i] >= 0)
		{
			LOG_INFO(debug_log, "Device %d with stae %d('%s') found", i, inst->dev_state[i], rr_describe_nmt(inst->dev_state[i]));
			fprintf(f, "%d ", i);
		}
	}
	
	fprintf(f, "\n");
		
	usbcan_instance_deinit(&inst);
	fclose(f);
	return 0;
}
