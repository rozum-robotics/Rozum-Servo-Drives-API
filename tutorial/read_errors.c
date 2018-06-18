#include "api.h"
#include "tutorial.h"

int main(int argc, char *argv[])
{
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading servo error array");
    uint8_t array[100];
    uint32_t size;
    rr_read_error_status(servo, array, &size);
    API_DEBUG("\tArray size: %d %s\n", size, size ? "" : "(No errors)");
    for(int i = 0; i < size; i++)
    {
        API_DEBUG("\t\tError: %d\n", array[i]);
    }
}
