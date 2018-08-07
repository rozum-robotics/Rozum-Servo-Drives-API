import logging
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "../.."))

from rozum.servo import ServoApi
from rozum.tutorial import *
from rozum.constants import *

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

if __name__ == '__main__':
    logger.info("Initializing ServoApi")
    api = ServoApi()

    logger.info("Loading library")
    api.load_library(LIBRARY_PATH)

    logger.info("Initializing interface {}".format(INTERFACE_NAME))
    interface = api.init_interface(INTERFACE_NAME)

    logger.info("Initializing servo id {}".format(SERVO_1_ID))
    servo = api.init_servo(SERVO_1_ID)

    position_rotor = servo.read_parameter(APP_PARAM_POSITION_ROTOR)
    logger.info("Position rotor {}".format(position_rotor))

    velocity_rotor = servo.read_parameter(APP_PARAM_VELOCITY_ROTOR)
    logger.info("Velocity rotor {}".format(velocity_rotor))

    voltage_input = servo.read_parameter(APP_PARAM_VOLTAGE_INPUT)
    logger.info("Voltage input {}".format(voltage_input))

    current_input = servo.read_parameter(APP_PARAM_CURRENT_INPUT)
    logger.info("Current input {}".format(current_input))
