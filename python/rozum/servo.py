import time
import os
import logging
from functools import wraps
from enum import IntEnum
from rozum.util import Singleton
from rozum.constants import *

log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())
# TODO Switching servo working states: rr_setup_nmt_callback, rr_describe_nmt
# TODO add invoke_time_calculation
# TODO write doc strings
# TODO tests
# TODO constants as enums and modify _api functions so that they return enums


class CtypesEnum(IntEnum):  # for future use
    @classmethod
    def from_param(cls, obj):
        return int(obj)


def _log_func_status(func, status):
    message = "Call {} returned {}".format(func.__name__, RET_STATUS_MESSAGE[status])
    if status == RET_OK.value:
        log.info(message)
    else:
        log.error(message)


def ret_status_t(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        status = func(*args, **kwargs)
        _log_func_status(func, status)
    return wrapper


class Servo(object):
    def __init__(self, library_api, servo_interface, identifier):
        self._api = library_api
        self._servo = servo_interface
        self._identifier = identifier

    @ret_status_t
    def param_cache_update(self):
        return self._api.rr_param_cache_update(self._servo)

    @ret_status_t
    def param_cache_setup_entry(self, param: c_int, enabled: bool):
        return self._api.rr_param_cache_setup_entry(self._servo, param, c_bool(enabled))

    def read_parameter(self, param: c_int):
        value = c_float()
        status = self._api.rr_read_parameter(self._servo, param, byref(value))
        _log_func_status(self.read_parameter, status)
        return value.value

    def read_cached_parameter(self, param: c_int):
        value = c_float()
        status = self._api.rr_read_cached_parameter(self._servo, param, byref(value))
        _log_func_status(self.read_cached_parameter, status)
        return value.value

    @ret_status_t
    def set_zero_position(self, position_deg: float):
        return self._api.rr_set_zero_position(self._api, c_float(position_deg))

    @ret_status_t
    def set_zero_position_and_save(self, position_deg: float):
        return self._api.rr_set_zero_position_and_save(self._servo, c_float(position_deg))

    def get_max_velocity(self):
        velocity = c_float()
        status = self._api.rr_get_max_velocity(self._servo, byref(velocity))
        _log_func_status(self.get_max_velocity, status)
        return velocity.value

    @ret_status_t
    def set_max_velocity(self, max_velocity_deg_per_sec: float):
        return self._api.rr_set_max_velocity(self._servo, c_float(max_velocity_deg_per_sec))

    @ret_status_t
    def add_motion_point(self, position_deg: float, velocity_deg_per_sec: float, time_ms: int):
        return self._api.rr_add_motion_point(self._servo,
                                      c_float(position_deg), c_float(velocity_deg_per_sec), c_uint32(time_ms))

    @ret_status_t
    def clear_points_all(self):
        return self._api.rr_clear_points_all(self._servo)

    @ret_status_t
    def clear_points(self, num_to_clear: int):
        return self._api.rr_clear_points(self._servo, c_uint32(num_to_clear))

    def get_points_size(self):
        size = c_uint32(0)
        status = self._api.rr_get_points_size(self._servo, byref(size))
        _log_func_status(self.get_points_size, status)
        return size.value

    def get_points_free_space(self):
        size = c_uint32(0)
        status = self._api.rr_get_points_free_space(self._servo, byref(size))
        _log_func_status(self.get_points_free_space, status)
        return size.value

    @ret_status_t
    def stop_and_release(self):
        return self._api.rr_stop_and_release(self._servo)

    @ret_status_t
    def stop_and_freeze(self):
        return self._api.rr_stop_and_freeze(self._servo)

    @ret_status_t
    def set_current(self, current_a: float):
        return self._api.rr_set_current(self._servo, c_float(current_a))

    @ret_status_t
    def set_velocity(self, velocity_deg_per_sec: float):
        return self._api.rr_set_velocity(self._servo, c_float(velocity_deg_per_sec))

    @ret_status_t
    def set_position(self, position_deg: float):
        return self._api.rr_set_position(self._servo, c_float(position_deg))

    @ret_status_t
    def set_velocity_with_limits(self, velocity_deg_per_sec: float, current_a: float):
        return self._api.rr_set_velocity_with_limits(self._servo, c_float(velocity_deg_per_sec), c_float(current_a))

    @ret_status_t
    def set_position_with_limits(self, position_deg: float, velocity_deg_per_sec: float, current_a: float):
        return self._api.rr_set_position_with_limits(self._servo,
                                                     c_float(position_deg),
                                                     c_float(velocity_deg_per_sec),
                                                     c_float(current_a))

    @ret_status_t
    def set_duty(self, duty_percent: float):
        return self._api.rr_set_duty(self._servo, c_float(duty_percent))

    @ret_status_t
    def reboot(self):
        return self._api.rr_servo_reboot(self._servo)

    @ret_status_t
    def reset_communication(self):
        return self._api.rr_servo_reset_communication(self._servo)

    @ret_status_t
    def set_state_operational(self):
        return self._api.rr_servo_set_state_operational(self._servo)

    @ret_status_t
    def set_state_pre_operational(self):
        return self._api.rr_servo_set_state_pre_operational(self._servo)

    @ret_status_t
    def set_state_stopped(self):
        return self._api.rr_servo_set_state_stopped(self._servo)

    @ret_status_t
    def __del__(self):
        return self._api.rr_deinit_servo(byref(c_void_p(self._servo)))


class Interface(object):

    def __init__(self, library_api, interface_name):
        self._api = library_api
        self._interface = self._api.rr_init_interface(bytes(interface_name, encoding="utf-8"))
        if self._interface is None:
            message = "Failed to initialize interface by name: {}".format(interface_name)
            log.error(message)
            raise AttributeError(message)
        self._servos = {}
        time.sleep(0.5)  # for interface initialization

    @ret_status_t
    def start_motion(self, timestamp_ms: int):
        return self._api.rr_start_motion(self._interface, c_uint32(timestamp_ms))

    def init_servo(self, identifier) -> Servo:
        if identifier not in self._servos:
            servo_interface = self._api.rr_init_servo(self._interface, c_uint8(identifier))
            if servo_interface is None:
                message = "Failed to initialize servo by id: {}".format(identifier)
                log.error(message)
                raise AttributeError(message)
            self._servos[identifier] = Servo(self._api, servo_interface, identifier)
        return self._servos[identifier]

    @ret_status_t
    def net_reboot(self):
        return self._api.rr_net_reboot(self._interface)

    @ret_status_t
    def net_reset_communication(self):
        return self._api.rr_net_reset_communication(self._interface)

    @ret_status_t
    def net_set_state_operational(self):
        return self._api.rr_net_set_state_operational(self._interface)

    @ret_status_t
    def net_set_state_pre_operational(self):
        return self._api.rr_net_set_state_pre_operational(self._interface)

    @ret_status_t
    def net_set_state_stopped(self):
        return self._api.rr_net_set_state_stopped(self._interface)

    @ret_status_t
    def __del__(self):
        for servo in self._servos.values():
            del servo
        return self._api.rr_deinit_interface(byref(c_void_p(self._interface)))


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

    def load_library(self, library_path: str = None):
        if library_path is None:
            self._api = CDLL(os.path.join(os.path.dirname(__file__), "libservo_api.so"))
        else:
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
    time.sleep(1)
    t_servo.set_current(1)
    time.sleep(2)
    print(t_servo.read_parameter(APP_PARAM_DUTY))
    print(t_servo.get_points_free_space())
    time.sleep(2)
    t_servo.set_current(0)
    time.sleep(1)
