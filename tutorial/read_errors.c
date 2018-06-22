#include "api.h"
#include "tutorial.h"

int main(int argc, char *argv[])
{
    rr_can_interface_t *iface = rr_init_interface(/*TUTORIAL_DEVICE*/"/dev/ttyS3");
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading servo error count");
    uint32_t _size;
    rr_read_error_status(servo, &_size, 0);
    API_DEBUG("\tError count: %d %s\n", _size, _size ? "" : "(No errors)");

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading servo error list");
    uint8_t array[100];
    uint32_t size;
    rr_read_error_status(servo, &size, array);
    API_DEBUG("\tError count: %d %s\n", size, size ? "" : "(No errors)");
    for(int i = 0; i < size; i++)
    {
        API_DEBUG("\t\tError: %s\n", rr_describe_emcy_bit(array[i]));
    }
}
