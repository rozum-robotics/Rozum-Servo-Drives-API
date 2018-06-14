#include "api.h"
#include "math.h"
#include <unistd.h>

#define SERVO_ID 123

int main(int argc, char *argv[])
{
    rr_can_interface_t *iface = rr_init_interface("/dev/ttyS3");
    rr_servo_t *servo = rr_init_servo(iface, SERVO_ID);

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading motion queue parameters");

    API_DEBUG("Clearing points\n", 0);
    rr_clear_points_all(servo);

    uint32_t num;
    rr_get_points_size(servo, &num);
    API_DEBUG("\tPoints in the queue before: %d\n", num);

    rr_get_points_free_space(servo, &num);
    API_DEBUG("\tPoints queue free size before: %d\n", num);

    API_DEBUG("Appending points\n", 0);
    rr_add_motion_point(servo, 0.0, 0.0, 10000000);
    rr_add_motion_point(servo, 0.0, 0.0, 10000000);

    rr_get_points_size(servo, &num);
    API_DEBUG("\tPoints in the queue after: %d\n", num);

    rr_get_points_free_space(servo, &num);
    API_DEBUG("\tPoints queue free size after: %d\n", num);
}