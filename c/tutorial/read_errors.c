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
 * \defgroup tutor_c_error_read Reading device errors
 * The tutorial describes how to read the total number of errors that occurred on the servo
 * and to display their description.
 * 
 * 1. Initialize the interface.
 * \snippet read_errors.c Adding interface 6
 *  
 * 2. Initialize the servo.
 * \snippet read_errors.c Adding servo 6
 * 
 * <b> Reading the current error count</b>
 * 
 * 3. Create a variable where the function will read the current error count.
 * \snippet read_errors.c Error count var
 * 
 * 4. Read the current error count. <b>Note</b>: The "array" argument is zero (we don't need to read error bits).
 * \snippet read_errors.c Error count read
 * 
 * <b> Reading the current error count and error bits</b>
 * 
 * 5. Create an array where the function will read the current error bits.
 *  \snippet read_errors.c Error array 2
 * 
 * 6. Create a variable where the function will read the current error count.
 * \snippet read_errors.c Error count var 2
 * 
 * 7. Read the current error count and error bits (if any).
 * \snippet read_errors.c Error count read
 * 
 * 8. Cycle print of error bits (described by ::rr_describe_emcy_bit function).
 * \snippet read_errors.c Cyclic read
 * 
 * <b> Complete tutorial code: </b>
 * \snippet read_errors.c cccode 6
 */

int main(int argc, char *argv[])
{
    //! [cccode 6]
    //! [Adding interface 6]
    rr_can_interface_t *iface = rr_init_interface(/*TUTORIAL_DEVICE*/ "/dev/ttyS3");
    //! [Adding interface 6]

    //! [Adding servo 6]
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    //! [Adding servo 6]

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
    //! [cccode 6]
}
