import argparse
import logging
import time
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "../.."))

from rozum.servo import ServoApi
from rozum.tutorials import INTERFACE_NAME

logging.basicConfig()
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument("--current_id", type=int, help="current servo id that you want to change")
parser.add_argument("--new_id", type=int, help="new servo id that you want to set")

args = parser.parse_args()

if __name__ == '__main__':
    logger.info("Initializing ServoApi")
    api = ServoApi()

    logger.info("Loading library")
    api.load_library()

    logger.info("Initializing interface")
    interface = api.init_interface(INTERFACE_NAME)

    logger.info("Changing current servo id {} to {}".format(args.current_id, args.new_id))
    interface.change_id_and_save(args.current_id, args.new_id)
    time.sleep(5)
    logger.info("Returning current id value {}".format(args.current_id))
    interface.change_id_and_save(args.new_id, args.current_id)
    time.sleep(5)
