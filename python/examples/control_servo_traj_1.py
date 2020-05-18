""" @page tutor_py_servomove1 PVT trajectory for one servo 
Complete source code: <br>
\snippet control_servo_traj_1.py control_servo_traj_1_py
"""

"""! [control_servo_traj_1_py] """
import logging
import os
import argparse

import rdrive as rr

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument(
    "--servo_1_id", type=int, help="first servo ID that you want control"
)
parser.add_argument("--interface", type=str, help="interface name")

args = parser.parse_args()

INTERFACE_NAME = args.interface
SERVO_1_ID = args.servo_1_id

if __name__ == "__main__":
    logger.info("Initializing ServoApi")
    api = rr.ServoApi()

    logger.info("Initializing interface {}".format(INTERFACE_NAME))
    interface = api.init_interface(INTERFACE_NAME)

    logger.info("Initializing servo id {}".format(SERVO_1_ID))
    servo = interface.init_servo(SERVO_1_ID)

    logger.info("Setting servo to operational state")
    servo.set_state_operational()

    logger.info("Appending points")
    servo.add_motion_point(100.0, 0.0, 6000)
    servo.add_motion_point(-100.0, 0.0, 6000)
    logger.info("Appending point with acceleration")
    servo.add_motion_point(0, 0, 6000, accel_deg_per_sec2=0)
    logger.info("Starting motion")
    interface.start_motion(0)

    logger.info("Waiting till motion ends")
    api.sleep_ms(20000)
"""! [control_servo_traj_1_py] """

