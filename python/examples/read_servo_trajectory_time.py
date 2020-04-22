import logging
import os
import argparse

import rdrive as rr

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument("--servo_1_id", type=int, help="first servo ID that you want control")
parser.add_argument("--interface", type=str, help="interface name")

args = parser.parse_args()

INTERFACE_NAME = args.interface
SERVO_1_ID = args.servo_1_id

if __name__ == '__main__':
    logger.info("Initializing ServoApi")
    api = rr.ServoApi()

    logger.info("Initializing interface {}".format(INTERFACE_NAME))
    interface = api.init_interface(INTERFACE_NAME)

    logger.info("Initializing servo id {}".format(SERVO_1_ID))
    servo = interface.init_servo(SERVO_1_ID)

    logger.info("Setting servo to operational state")
    servo.set_state_operational()

    travel_time = servo.invoke_time_calculation(0.0, 0.0, 0.0, 0, 100.0, 0.0, 0.0, 0)
    logger.info("Travel time: {}".format(travel_time))
