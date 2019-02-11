#include "api.h"
#include <stdlib.h>


int main(int argc, char *argv[])
{
    uint8_t id;
    float ang, vel, acc;

    if(argc == 6)
    {
        id = strtol(argv[2], NULL, 0);
	ang = strtof(argv[3], NULL);
	vel = strtof(argv[4], NULL);
	acc = strtof(argv[5], NULL);
    }
    else
    {
        API_DEBUG("Wrong format!\nUsage: %s interface id angle max_vel max_acc\n", argv[0]);
        return 1;
    }

    rr_can_interface_t *iface = rr_init_interface(argv[1]);
    rr_servo_t *servo = rr_init_servo(iface, id);

    //! [Switching to operational state]
    rr_servo_set_state_operational(servo);
    //! [Switching to operational state]

    rr_clear_points_all(servo);


    API_DEBUG("Appending points\n");
    uint32_t time_to_get_ms = 0;
    int status = rr_set_position_with_limits(servo, ang, vel, acc, &time_to_get_ms);
    if(status != RET_OK)
    {
        API_DEBUG("Error in point appending: %d\n", status);
        return 1;
    }

    API_DEBUG("Time to get: %d ms\n", time_to_get_ms);
    rr_sleep_ms(time_to_get_ms + 100); // wait till the movement ends
}


