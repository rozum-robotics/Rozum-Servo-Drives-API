import logging
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "../.."))

import rdrive as rr
from rdrive.servo.tutorial import *

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

if __name__ == '__main__':
    logger.info("Initializing ServoApi")
    api = rr.ServoApi()

    logger.info("Initializing interface {}".format(INTERFACE_NAME))
    interface = api.init_interface(INTERFACE_NAME)

    logger.info("Initializing servo id {}".format(SERVO_1_ID))
    servo = interface.init_servo(SERVO_1_ID)

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
