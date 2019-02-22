/**
 * @brief Tutorial example of reading emergency (EMY) log buffer
 * 
 * @file read_any_param.c
 * @author your name
 * @date 2018-06-25
 */

#include "api.h"
#include <stdlib.h>
#include <inttypes.h>

 
int main(int argc, char *argv[])
{
    uint8_t id;
    emcy_log_entry_t *emcy;

    if(argc == 3)
    {
        id = strtol(argv[2], NULL, 0);
    }
    else
    {
        API_DEBUG("Wrong format!\nUsage: %s interface id\n", argv[0]);
        return 1;
    }
    //! [cccode 4] 
    //! [Adding interface 4]
    rr_can_interface_t *iface = rr_init_interface(argv[1]);
	
    //! [Adding interface 4]
    //! [Adding servo 4]
    rr_servo_t *servo = rr_init_servo(iface, id);
    //! [Adding servo 4]
    
    while((emcy = rr_emcy_log_pop(iface)))
    {
    	printf("id: %d, code: 0x%." PRIx16 ", reg: 0x%.2" PRIx8 ", bits: 0x%.2" PRIx8 ", info: 0x%.8" PRIx32 "\n", 
			(int)emcy->id, emcy->err_code, emcy->err_reg, emcy->err_bits, emcy->err_info);
    }


}
