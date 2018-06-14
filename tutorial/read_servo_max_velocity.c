#include "api.h"
#include "math.h"
#include <unistd.h>

#define SERVO_ID 123

int main(int argc, char *argv[])
{
    rr_can_interface_t *iface = rr_init_interface("/dev/ttyS3");
    rr_servo_t *servo = rr_init_servo(iface, SERVO_ID);

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading servo max velocity");
    float velocity;
    rr_get_max_velocity(servo, &velocity);

    API_DEBUG("\tMax velocity: %.3f Deg/sec\n", velocity);
}