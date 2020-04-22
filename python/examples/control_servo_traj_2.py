import argparse
import logging
import os

import rdrive as rr

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument(
    "--servo_1_id", type=int, help="first servo ID that you want control"
)
parser.add_argument(
    "--servo_2_id", type=int, help="second servo ID that you want control"
)
parser.add_argument("--interface", type=str, help="interface name")

args = parser.parse_args()

INTERFACE_NAME = args.interface
SERVO_1_ID = args.servo_1_id
SERVO_2_ID = args.servo_2_id

if __name__ == "__main__":
    logger.info("Initializing ServoApi")
    api = rr.ServoApi()

    logger.info("Initializing interface {}".format(INTERFACE_NAME))
    interface = api.init_interface(INTERFACE_NAME)

    logger.info("Initializing servo id {}".format(SERVO_1_ID))
    servo1 = interface.init_servo(SERVO_1_ID)

    logger.info("Setting servo1 to operational state")
    servo1.set_state_operational()

    logger.info("Initializing servo id {}".format(SERVO_2_ID))
    servo2 = interface.init_servo(SERVO_2_ID)

    logger.info("Setting servo2 to operational state")
    servo2.set_state_operational()

    logger.info("Appending points")
    servo1.add_motion_point(100.0, 0.0, 6000)
    servo1.add_motion_point(-100.0, 0.0, 6000)
    servo1.add_motion_point(0, 0, 6000)
    servo2.add_motion_point(100.0, 0.0, 6000)
    servo2.add_motion_point(-100.0, 0.0, 6000)
    servo2.add_motion_point(0, 0, 6000)
    logger.info("Starting motion")
    interface.start_motion(0)

    logger.info("Waiting till motion ends")
    api.sleep_ms(20000)
