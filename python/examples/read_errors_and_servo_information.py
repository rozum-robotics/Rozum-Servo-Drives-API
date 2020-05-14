""" @page tutor_py_error_read Reading error information
Complete source code: <br>
\snippet read_errors_and_servo_information.py read_errors_and_servo_information_py
"""

"""! [read_errors_and_servo_information_py] """
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

    logger.info("Initializing servo id {}".format(SERVO_1_ID))
    servo = interface.init_servo(SERVO_1_ID)

    logger.info("Setting servo to operational state")
    servo.set_state_operational()

    logger.info("Reading hardware and software info.")
    version = servo.get_version()
    logger.info("Got info: {}".format(version))

    logger.info("Reading error status")
    num, arr = servo.read_error_status(100)
    logger.info("Got {} errors".format(num))
    logger.info("Describing errors")
    for i in range(num):
        api.describe_emcy_bit(arr[i])
"""! [read_errors_and_servo_information_py] """
