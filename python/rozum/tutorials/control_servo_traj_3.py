import logging
import os
from rozum.servo import ServoApi
from rozum.tutorials import *

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
    servo1 = api.init_servo(SERVO_1_ID)

    logger.info("Initializing servo id {}".format(SERVO_2_ID))
    servo2 = api.init_servo(SERVO_2_ID)

    logger.info("Initializing servo id {}".format(SERVO_3_ID))
    servo3 = api.init_servo(SERVO_3_ID)

    logger.info("Appending points")
    servo1.add_motion_point(100., 0., 6000)
    servo1.add_motion_point(-100., 0., 6000)
    servo1.add_motion_point(0, 0, 6000)
    servo2.add_motion_point(100., 0., 6000)
    servo2.add_motion_point(-100., 0., 6000)
    servo2.add_motion_point(0, 0, 6000)
    servo3.add_motion_point(100., 0., 6000)
    servo3.add_motion_point(-100., 0., 6000)
    servo3.add_motion_point(0, 0, 6000)
    logger.info("Starting motion")
    interface.start_motion(0)

    logger.info("Waiting till motion ends")
    api.sleep_ms(20000)

