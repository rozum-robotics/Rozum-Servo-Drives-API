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

    logger.info("Setting cache")
    servo.param_cache_setup_entry(rr.APP_PARAM_POSITION_ROTOR, True)
    servo.param_cache_setup_entry(rr.APP_PARAM_VELOCITY_ROTOR, True)
    servo.param_cache_setup_entry(rr.APP_PARAM_VOLTAGE_INPUT, True)
    servo.param_cache_setup_entry(rr.APP_PARAM_CURRENT_INPUT, True)

    logger.info("Updating cache")
    servo.param_cache_update()

    logger.info("Reading cache")
    position_rotor = servo.read_cached_parameter(rr.APP_PARAM_POSITION_ROTOR)
    velocity_rotor = servo.read_cached_parameter(rr.APP_PARAM_VELOCITY_ROTOR)
    voltage_input = servo.read_cached_parameter(rr.APP_PARAM_VOLTAGE_INPUT)
    current_input = servo.read_cached_parameter(rr.APP_PARAM_CURRENT_INPUT)

    logger.info("\nposition_rotor = {}\nvelocity_rotor = {}\nvoltage_input = {}\ncurrent_input = {}"
                .format(position_rotor, velocity_rotor, voltage_input, current_input))
