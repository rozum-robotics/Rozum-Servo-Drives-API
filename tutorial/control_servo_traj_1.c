#include "api.h"
#include "math.h"
#include <unistd.h>

#define SERVO_0_ID 123

int main(int argc, char *argv[])
{
    rr_can_interface_t *iface = rr_init_interface("/dev/ttyS3");
    rr_servo_t *servo = rr_init_servo(iface, SERVO_0_ID);

    API_DEBUG("========== Tutorial of the %s ==========\n", "controlling one servo");

    rr_clear_points_all(servo);
    API_DEBUG("Appending points\n", 0);
    int status = rr_add_motion_point(servo, 100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    status = rr_add_motion_point(servo, -100.0, 0.0, 6000);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation: %d\n", status);
        return 1;
    }
    rr_start_motion(iface, 0);

    rr_sleep_ms(14000);
}