""" @page tutor_py_nmt_state Reading CanOpen network state 
Complete source code: <br>
\snippet read_servo_nmt_state.py read_servo_nmt_state_py
"""

"""! [read_servo_nmt_state_py] """
import logging
import os
import argparse

import rdrive as rr

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument(
    "--servo_1_id", type=int, help="first servo ID that you want control"
)
parser.add_argument("--interface", type=str, help="interface name")

args = parser.parse_args()

INTERFACE_NAME = args.interface
SERVO_1_ID = args.servo_1_id

if __name__ == "__main__":
    logger.info("Initializing ServoApi")
    api = rr.ServoApi()

    logger.info("Initializing interface {}".format(INTERFACE_NAME))
    interface = api.init_interface(INTERFACE_NAME)

    logger.info("Reading servomotor state.")

    nmt_code = interface.net_get_state(SERVO_1_ID)
    logger.info("Got code: {}".format(nmt_code))

    human_readable_result = api.describe_nmt_state(nmt_code)
    logger.info("Got state: {}".format(human_readable_result))
"""! [read_servo_nmt_state_py] """
