#include "api.h"
#include "math.h"
#include <unistd.h>

#define SERVO_ID 123

int main(int argc, char *argv[])
{
    rr_can_interface_t *iface = rr_init_interface("/dev/ttyS3");
    rr_servo_t *servo = rr_init_servo(iface, SERVO_ID);

    API_DEBUG("========== Tutorial of the %s ==========\n", "trajectory calculation");

    int status = rr_invoke_time_calculation(servo, 0.0, 0.0, 0.0, 0, 100.0, 0.0, 0.0, 0);
    if(status != RET_OK)
    {
        API_DEBUG("Error in the trjectory point calculation\n", 0);
        return 1;
    }

    uint32_t travel_time;
    rr_get_time_calculation_result(servo, &travel_time);
    API_DEBUG("\tCalculated travel time: %d ms.\n", travel_time);
}