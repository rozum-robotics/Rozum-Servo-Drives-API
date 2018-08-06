import logging
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "../.."))

from rozum.servo import ServoApi
from rozum.tutorials import INTERFACE_NAME, SERVO_1_ID

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

if __name__ == '__main__':
    logger.info("Initializing ServoApi")
    api = ServoApi()

    logger.info("Loading library")
    api.load_library()

    logger.info("Initializing interface {}".format(INTERFACE_NAME))
    interface = api.init_interface(INTERFACE_NAME)

    logger.info("Initializing servo id {}".format(SERVO_1_ID))
    servo = api.init_servo(SERVO_1_ID)

    max_velocity = servo.get_max_velocity()
    logger.info("Max velocity {}".format(max_velocity))
