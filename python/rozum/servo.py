from rozum.util import Singleton
from ctypes import *
import time
import os
# TODO Switching servo working states: rr_setup_nmt_callback, rr_describe_nmt
# TODO Check statuses and logging
# TODO add invoke_time_calculation


class Servo(object):
    def __init__(self, library_api, servo_interface, identifier):
        self._api = library_api
        self._servo = servo_interface
        self._identifier = identifier

    def add_motion_point(self, position_deg: float, velocity_deg_per_sec: float, time_ms: int):
        self._api.rr_add_motion_point(self._servo,
                                      c_float(position_deg), c_float(velocity_deg_per_sec), c_uint32(time_ms))

    def clear_points_all(self):
        self._api.rr_clear_points_all(self._servo)

    def clear_points(self, num_to_clear: int):
        self._api.rr_clear_points(self._servo, c_uint32(num_to_clear))

    def get_points_size(self):
        size = c_uint32(0)
        self._api.rr_get_points_size(self._servo, byref(size))
        return size

    def get_points_free_space(self):
        size = c_uint32(0)
        self._api.rr_get_points_free_space(self._servo, byref(size))
        return size

    def stop_and_release(self):
        self._api.rr_stop_and_release(self._servo)

    def stop_and_freeze(self):
        self._api.rr_stop_and_freeze(self._servo)

    def set_current(self, current_a: float):
        self._api.rr_set_current(self._servo, c_float(current_a))

    def set_velocity(self, velocity_deg_per_sec: float):
        self._api.rr_set_velocity(self._servo, c_float(velocity_deg_per_sec))

    def set_position(self, position_deg: float):
        self._api.rr_set_position(self._servo, c_float(position_deg))

    def set_velocity_with_limits(self, velocity_deg_per_sec: float, current_a: float):
        self._api.rr_set_velocity_with_limits(self._servo, c_float(velocity_deg_per_sec), c_float(current_a))

    def set_position_with_limits(self, position_deg: float, velocity_deg_per_sec: float, current_a: float):
        self._api.rr_set_position_with_limits(self._servo,
                                              c_float(position_deg), c_float(velocity_deg_per_sec), c_float(current_a))

    def set_duty(self, duty_percent: float):
        self._api.rr_set_duty(self._servo, c_float(duty_percent))

    def reboot(self):
        self._api.rr_servo_reboot(self._servo)

    def reset_communication(self):
        self._api.rr_servo_reset_communication(self._servo)

    def set_state_operational(self):
        self._api.rr_servo_set_state_operational(self._servo)

    def set_state_pre_operational(self):
        self._api.rr_servo_set_state_pre_operational(self._servo)

    def set_state_stopped(self):
        self._api.rr_servo_set_state_stopped(self._servo)

    def __del__(self):
        self._api.rr_deinit_servo(byref(c_void_p(self._servo)))


class Interface(object):

    def __init__(self, library_api, interface_name):
        self._api = library_api
        self._interface = self._api.rr_init_interface(bytes(interface_name, encoding="utf-8"))
        self._servos = {}
        time.sleep(0.5)  # for interface initialization

    def start_motion(self, timestamp_ms: int):
        self._api.rr_start_motion(self._interface, c_uint32(timestamp_ms))

    def init_servo(self, identifier) -> Servo:
        if identifier not in self._servos:
            servo_interface = self._api.rr_init_servo(self._interface, c_uint8(identifier))
            self._servos[identifier] = Servo(self._api, servo_interface, identifier)
        return self._servos[identifier]

    def net_reboot(self):
        self._api.rr_net_reboot(self._interface)

    def net_reset_communication(self):
        self._api.rr_net_reset_communication(self._interface)

    def net_set_state_operational(self):
        self._api.rr_net_set_state_operational(self._interface)

    def net_set_state_pre_operational(self):
        self._api.rr_net_set_state_pre_operational(self._interface)

    def net_set_state_stopped(self):
        self._api.rr_net_set_state_stopped(self._interface)

    def __del__(self):
        for servo in self._servos.values():
            del servo
        self._api.rr_deinit_interface(byref(c_void_p(self._interface)))


class ServoApi(object, metaclass=Singleton):
    __LIBRARY_NAME = "libservo_api.so"

    def __init__(self):
        print(str(self.__class__) + " Initialized")
        self._api = None
        self._interface = None

    def _check_library_loaded(self):
        if self._api is None:
            raise AttributeError("Library not loaded. Consider call api.load_library(path_to_library) first.")

    def _check_interface_initialized(self):
        if self._interface is None:
            raise AttributeError("Interface not initialized. Call api.init_interface(interface_name) first.")

    @property
    def api(self):
        self._check_library_loaded()
        return self._api

    @property
    def interface(self) -> Interface:
        self._check_library_loaded()
        self._check_interface_initialized()
        return self._interface

    def load_library(self, library_path: str):
        if ServoApi.__LIBRARY_NAME not in library_path:
            raise ArgumentError("Expected that path to library contains " + ServoApi.__LIBRARY_NAME)
        if self._api is None:
            self._api = CDLL(library_path)

    def init_interface(self, interface_name: str):
        if self._interface is None:
            self._interface = Interface(self._api, interface_name)

    def init_servo(self, identifier: int) -> Servo:
        return self.interface.init_servo(identifier)

    def __del__(self):
        if self._interface is not None:
            del self._interface


if __name__ == '__main__':
    #  "../../c/build/libservo_api.so"
    api = ServoApi()
    api.load_library(os.path.join(os.path.dirname(__file__), "libservo_api.so"))
    api.init_interface("/dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00")
    t_servo = api.init_servo(64)
    t_servo.set_state_operational()
    time.sleep(1)
    t_servo.set_velocity(20)
    time.sleep(10)
    t_servo.set_velocity(0)
    time.sleep(1)
