import logging
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "../.."))

import rozum as rr
from rozum.servo.tutorial import *

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

if __name__ == '__main__':
    logger.info("Initializing ServoApi")
    api = rr.ServoApi()

    logger.info("Initializing interface {}".format(INTERFACE_NAME))
    interface = api.init_interface(INTERFACE_NAME)

    logger.info("Reading servomotor state.")

    nmt_code = interface.net_get_state(SERVO_1_ID)
    logger.info("Got code: {}".format(nmt_code))

    human_readable_result = api.describe_nmt_state(nmt_code)
    logger.info("Got state: {}".format(human_readable_result))
