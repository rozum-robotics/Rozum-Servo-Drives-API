/**
 * @brief Changing CAN ID of a single servo
 * @file change_servo_id.c
 * @author Rozum
 * @date 2018-07-11
 */

#include "api.h"
#include "stdlib.h"
#include "tutorial.h"

/**
 * \defgroup tutor_c_changeID1 Changing CAN ID of a single servo
 * The tutorial describes how to ....
 *
 * 
 * <b> Complete tutorial code: </b>
 * \snippet change_servo_id.c change_id_code_full
 */

int main(int argc, char *argv[])
{
    //! [change_id_code_full]
    uint8_t id_old;
    uint8_t id_new;

    if(argc == 3)
    {
        id_old = strtol(argv[1], NULL, 0);
        id_new = strtol(argv[2], NULL, 0);
    }
    else
    {
        API_DEBUG("Wrong format!\nUsage: %s old_id new_id\n", argv[0]);
        return 1;
    }

    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);

    rr_servo_t *servo = rr_init_servo(iface, id_old);
    if(servo == NULL)
    {
        API_DEBUG("Failed to init servo with ID: %d\n", id_old);
        return 1;
    }

    API_DEBUG("========== Tutorial of the %s ==========\n", "changing CAN ID of one servo and saving it to the EEPROM");
    rr_ret_status_t status = rr_change_id_and_save(iface, servo, id_new);
    if(status != RET_OK)
    {
        API_DEBUG("Failed to change servo CAN ID and save it to the EEPROM: %d\n", status);
        return 1;
    }
    //! [change_id_code_full]
}
