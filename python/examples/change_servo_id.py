##
# \defgroup tutor_py_changeID1 Changing CAN ID of a single servo
"""The tutorial describes how to change the CAN identifier of a single servo and save it to the EEPROM memory.
<b>Important!</b> RDrive servos are all supplied with the same range of <b>default CAN IDs—from 32 to 37</b>.  
To avoid collisions, you need to assign a unique CAN ID to each servo on the same CAN bus.

1. Create two variables
# \snippet change_servo_id.py add variables
"""

import argparse
import logging
import time
import os

import rdrive as rr

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument(
    "--current_id", type=int, help="current servo ID that you want to change"
)
parser.add_argument(
    "--new_id", type=int, help="new servo ID that you want to set"
)
parser.add_argument("--interface", type=str, help="interface name")

args = parser.parse_args()

INTERFACE_NAME = args.interface

if __name__ == "__main__":
    logger.info("Initializing ServoApi")
    api = rr.ServoApi()
    # //! [add variables]
    logger.info("Initializing interface {}".format(INTERFACE_NAME))
    interface = api.init_interface(INTERFACE_NAME)
    # //! [add variables]
    logger.info(
        "Changing current servo id {} to {}".format(
            args.current_id, args.new_id
        )
    )
    interface.change_id_and_save(args.current_id, args.new_id)
    time.sleep(5)
    logger.info("Returning current id value {}".format(args.current_id))
    interface.change_id_and_save(args.new_id, args.current_id)
    time.sleep(5)
