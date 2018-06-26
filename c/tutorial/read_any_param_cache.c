/**
 * @brief Tutorial example of reading servo parameters from the cache
 * 
 * @file read_any_param_cache.c
 * @author Rozum
 * @date 2018-06-25
 */
#include "api.h"
#include "tutorial.h"

/**
 * @ingroup tutor_c_param_cache Reading servo parameters from the cache
 * 
 * This tutorial describes how to read an array of servo parameters at a time and save them to the program cache where they will be stored and available for reading until you quit the program.
 * In this example, we will read an array of four parameters: rotor position, rotor velocity, input voltage, and input current.
 * <b> Note </b>: In general, it is advisable to use the function, when you need to read more than one parameter from the servo.
 * If you need to read a single parameter, use the ::rr_read_parameter function (refer to the Device parameters reading tutorial).
 * 
 * 1. Initialize the interface.
 * \snippet read_any_param_cache.c Adding the interface
 * 
 * 2. Initialize the servo.
 * \snippet read_any_param_cache.c Adding the servo
 * 
 * <b> Setting up and saving an array of parameters to the program cache</b>
 * 
 * 3. Add parameter 1 (rotor position) to the array of parameters you want to read from the servo.
 * \snippet read_any_param_cache.c Cache setup entry 1
 * 
 * 4. Add parameter 2 (rotor velocity) to the array of parameters you want to read from the servo.
 * \snippet read_any_param_cache.c Cache setup entry 2
 * 
 * 5. Add parameter 3 (input voltage) to the array of parameters you want to read from the servo.
 * \snippet read_any_param_cache.c Cache setup entry 3
 * 
 * 6. Add parameter 4 (input current) to the array of parameters you want to read from the servo.
 * \snippet read_any_param_cache.c Cache setup entry 4
 * 
 * 7. Save the parameters to the program cache.
 * \snippet read_any_param_cache.c Cache update
 * 
 * <b> Reading the preset array of parameters from the cache </b>
 * 
 * 8. Create a variable where the function will read the parameters from the cache.
 * \snippet read_any_param_cache.c Parameter array
 * 
 * 9. Read parameter 1 (rotor position) from the cache.
 * \snippet read_any_param_cache.c Read cached parameter 1
 * 
 * 10. Read parameter 2 (rotor velocity) from the cache.
 *\snippet read_any_param_cache.c Read cached parameter 2
 * 
 * 11. Read parameter 3 (input voltage) from the cache.
 * \snippet read_any_param_cache.c Read cached parameter 3
 * 
 * 12. Read parameter 4 (input current) from the cache.
 * \snippet read_any_param_cache.c Read cached parameter 4
 * 
 * <b> Complete tutorial code: </b>
 * \snippet read_errors.c cccode 1
 */
 
 
int main(int argc, char *argv[])
{
    //! [cccode 1] 
    //! [Adding the interface]
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    //! [Adding the interface]
    //! [Adding the servo]
    rr_servo_t *servo = rr_init_servo(iface, TUTORIAL_SERVO_0_ID);
    //! [Adding the servo]

    API_DEBUG("========== Tutorial of %s ==========\n", "programming and reading the device parameter cache");

    //! [Cache setup entry 1]
    rr_param_cache_setup_entry(servo, APP_PARAM_POSITION_ROTOR, true);
    //! [Cache setup entry 1]
    //! [Cache setup entry 2]
    rr_param_cache_setup_entry(servo, APP_PARAM_VELOCITY_ROTOR, true);
    //! [Cache setup entry 2]
    //! [Cache setup entry 3]
    rr_param_cache_setup_entry(servo, APP_PARAM_VOLTAGE_INPUT, true);
    //! [Cache setup entry 3]
    //! [Cache setup entry 4]
    rr_param_cache_setup_entry(servo, APP_PARAM_CURRENT_INPUT, true);
    //! [Cache setup entry 4]

    //! [Cache update]
    rr_param_cache_update(servo);
    //! [Cache update]

    //! [Parameter array]
    float value;
    //! [Parameter array]

    //! [Read cached parameter 1]
    rr_read_cached_parameter(servo, APP_PARAM_POSITION_ROTOR, &value);
    //! [Read cached parameter 1]
    API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_POSITION_ROTOR), value);

    //! [Read cached parameter 2]
    rr_read_cached_parameter(servo, APP_PARAM_VELOCITY_ROTOR, &value);
    //! [Read cached parameter 2]
    API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_VELOCITY_ROTOR), value);

    //! [Read cached parameter 3]
    rr_read_cached_parameter(servo, APP_PARAM_VOLTAGE_INPUT, &value);
    //! [Read cached parameter 3]
    API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_VOLTAGE_INPUT), value);

    //! [Read cached parameter 4]
    rr_read_cached_parameter(servo, APP_PARAM_CURRENT_INPUT, &value);
    //! [Read cached parameter 4]
    API_DEBUG("\t%s value: %.3f\n", STRFY(APP_PARAM_CURRENT_INPUT), value);

    //! [cccode 1]
}
