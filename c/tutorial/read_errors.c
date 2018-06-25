/**
 * @brief Tutorial example of reading device errors
 * 
 * @file read_errors.c
 * @author Rozum
 * @date 2018-06-25
 */
#include "api.h"
#include "tutorial.h"

/**
 * \defgroup tutor_c_error_read Device errors reading
 * 
 *  Initialize the interface
 * \snippet read_errors.c Adding the interface 1
 * 
 *  Initialize the servo
 * \snippet read_errors.c Adding the servo
 * 
 * <b> Reading error count </b>
 * 
 * Create variable to read the current error count in it
 * \snippet read_errors.c Error count var
 * 
 * Reading error count. Notice: the "array" argument is zero (we don't need to read error bits)
 * \snippet read_errors.c Error count read
 * 
 * <b> Reading error count and error bits </b>
 * 
 * Create array to read the current error bits in it
 * \snippet read_errors.c Error array 2
 * 
 * Create variable to read the current error count in it
 * \snippet read_errors.c Error count var 2
 * 
 * Reading error count and error bits (if are present)
 * \snippet read_errors.c Error count read
 * 
 * Cycle print of error bits (described by ::rr_describe_emcy_bit function)
 * \snippet read_errors.c Cyclic read
 * 
 * <b> Complete tutorial code: </b>
 * \snippet read_errors.c cccode 1
 */

int main(int argc, char *argv[])
{
    //! [cccode 1]
    //! [Adding the interface 1]
    rr_can_interface_t *iface = rr_init_interface(/*TUTORIAL_DEVICE*/ "/dev/ttyS3");
    //! [Adding the interface 1]

    //! [Adding the servo]
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    //! [Adding the servo]

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading servo error count");

    //! [Error count var]
    uint32_t _size;
    //! [Error count var]
    //! [Error count read]
    rr_read_error_status(servo, &_size, 0);
    //! [Error count read]
    API_DEBUG("\tError count: %d %s\n", _size, _size ? "" : "(No errors)");

    API_DEBUG("========== Tutorial of the %s ==========\n", "reading servo error list");
    //! [Error array 2]
    uint8_t array[100];
    //! [Error array 2]
    //! [Error count var 2]
    uint32_t size;
    //! [Error count var 2]
    //! [Error count and array read]
    rr_read_error_status(servo, &size, array);
    //! [Error count and array read]
    API_DEBUG("\tError count: %d %s\n", size, size ? "" : "(No errors)");
    //! [Cyclic read]
    for(int i = 0; i < size; i++)
    {
        API_DEBUG("\t\tError: %s\n", rr_describe_emcy_bit(array[i]));
    }
    //! [Cyclic read]
    //! [cccode 1]
}
