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
 * The tutorial describes how to change the CAN identifier of a single servo and save it to the EEPROM memory.
 * <p><b>Important!</b> RDrive servos are all supplied with <b>the same default CAN ID—32</b>.
 * To avoid collisions, you need to assign <b> a unique CAN ID</b> to each servo on the same CAN bus.</p>
 * 
 * 1. Create two variables—one for the default (current) CAN ID and one for the new CAN ID.
 * \snippet change_servo_id.c Create 2 variables
 * 
 * 2. Set the default (current) ID and a new one to replace it.
 * In the example below, we get the current and new ID values from the console return.
 * \snippet change_servo_id.c Check arguments
 * 
 * 3. Initialize the interface.
 * \snippet change_servo_id.c Add interface10
 * 
 * 4. Initialize the servo.
 * \snippet change_servo_id.c Add servo10
 * 
 * 5. Change the default (current) servo ID to a new one. You use any value from 1 to 127, only make sure no other servo on the CAN bus has the same ID.
 * \snippet change_servo_id.c Change ID 
 * 
 * <b> Complete tutorial code: </b>
 * \snippet change_servo_id.c change_id_code_full
 */

int main(int argc, char *argv[])
{
    //! [change_id_code_full]
    //! [Create 2 variables]
    uint8_t id_old;
    uint8_t id_new;
    //! [Create 2 variables]

    //! [Check arguments]
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
    //! [Check arguments]
    //! [Add interface10]
    rr_can_interface_t *iface = rr_init_interface(TUTORIAL_DEVICE);
    //! [Add interface10]
    //! [Add servo10]
    rr_servo_t *servo = rr_init_servo(iface, id_old);
    if(servo == NULL)
    {
        API_DEBUG("Failed to init servo with ID: %d\n", id_old);
        return 1;
    }

    //! [Add servo10]
    API_DEBUG("========== Tutorial of the %s ==========\n", "changing CAN ID of one servo and saving it to the EEPROM");
    //! [Change ID]
    rr_ret_status_t status = rr_change_id_and_save(iface, servo, id_new);
    if(status != RET_OK)
    {
        API_DEBUG("Failed to change servo CAN ID and save it to the EEPROM: %d\n", status);
        return 1;
    }
    //! [Change ID]
    //! [change_id_code_full]
}
