import logging
import os
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

    logger.info("Appending points")
    servo.add_motion_point(100., 0., 6000)
    servo.add_motion_point(-100., 0., 6000)
    servo.add_motion_point(0, 0, 6000)
    logger.info("Starting motion")
    interface.start_motion(0)

    logger.info("Waiting till motion ends")
    api.sleep_ms(20000)


