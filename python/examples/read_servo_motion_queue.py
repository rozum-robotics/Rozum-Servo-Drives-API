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

    logger.info("Clearing points")
    servo.clear_points_all()

    points_size = servo.get_points_size()
    logger.info("Points in queue before: {}".format(points_size))

    free_points = servo.get_points_free_space()
    logger.info("Points queue free space before: {}".format(free_points))

    logger.info("Appending points")
    servo.add_motion_point(0.0, 0.0, 10000000)
    servo.add_motion_point(0.0, 0.0, 10000000)

    points_size = servo.get_points_size()
    logger.info("Points in queue after: {}".format(points_size))

    free_points = servo.get_points_free_space()
    logger.info("Points queue free space after: {}".format(free_points))

    logger.info("Clearing points")
    servo.clear_points_all()
