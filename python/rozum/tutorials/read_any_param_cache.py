import logging
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "../.."))

from rozum.servo import ServoApi
from rozum.tutorials import INTERFACE_NAME, SERVO_1_ID
from rozum.constants import *

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

    logger.info("Setting cache")
    servo.param_cache_setup_entry(APP_PARAM_POSITION_ROTOR, True)
    servo.param_cache_setup_entry(APP_PARAM_VELOCITY_ROTOR, True)
    servo.param_cache_setup_entry(APP_PARAM_VOLTAGE_INPUT, True)
    servo.param_cache_setup_entry(APP_PARAM_CURRENT_INPUT, True)

    logger.info("Updating cache")
    servo.param_cache_update()

    logger.info("Reading cache")
    position_rotor = servo.read_cached_parameter(APP_PARAM_POSITION_ROTOR)
    velocity_rotor = servo.read_cached_parameter(APP_PARAM_VELOCITY_ROTOR)
    voltage_input = servo.read_cached_parameter(APP_PARAM_VOLTAGE_INPUT)
    current_input = servo.read_cached_parameter(APP_PARAM_CURRENT_INPUT)

    logger.info("\nposition_rotor = {}\nvelocity_rotor = {}\nvoltage_input = {}\ncurrent_input = {}"
                .format(position_rotor, velocity_rotor, voltage_input, current_input))
