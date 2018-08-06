from ctypes import *
import time
import os
from enum import IntEnum
from rozum.util import Singleton
# TODO Switching servo working states: rr_setup_nmt_callback, rr_describe_nmt
# TODO Check statuses and logging
# TODO add invoke_time_calculation
# TODO write doc strings
# TODO tests
# TODO constants as enums and modify _api functions so that they return enums

(RET_OK,                                    # < Status OK
 RET_ERROR,                                 # < Generic error
 RET_BAD_INSTANCE,                          # < Bad interface or servo instance (null)
 RET_BUSY,                                  # < Device is busy
 RET_WRONG_TRAJ,                            # < Wrong trajectory
 RET_LOCKED,                                # < Device is locked
 RET_STOPPED,                               # < Device is in STOPPED state
 RET_TIMEOUT,                               # < Communication timeout
 RET_ZERO_SIZE,                             # < Zero size
 RET_SIZE_MISMATCH,                         # < Received & target size mismatch
 RET_WRONG_ARG                              # < Wrong function argurent
 ) = map(c_int, range(11))

(APP_PARAM_NULL,                            # < Not used
 APP_PARAM_POSITION,                        # < Actual multi-turn position of the output shaft (degrees)
 APP_PARAM_VELOCITY,                        # < Actual velocity of the output shaft (degrees per second)
 APP_PARAM_POSITION_ROTOR,                  # < Actual position of the motor shaft (degrees)
 APP_PARAM_VELOCITY_ROTOR,                  # < Actual velocity of the motor shaft (degrees per second)
 APP_PARAM_POSITION_GEAR_360,               # < Actual single-turn position of the output shaft (from 0 to 360 degrees)
 APP_PARAM_POSITION_GEAR_EMULATED,          # < Actual multi-turn position of the motor shaft multiplied by gear ratio (degrees)
 APP_PARAM_CURRENT_INPUT,                   # < Actual DC current (amperes)
 APP_PARAM_CURRENT_OUTPUT,                  # < Not used
 APP_PARAM_VOLTAGE_INPUT,                   # < Actual DC voltage (volts)
 APP_PARAM_VOLTAGE_OUTPUT,                  # < Not used
 APP_PARAM_CURRENT_PHASE,                   # < Actual magnitude of AC current (amperes)
 APP_PARAM_TEMPERATURE_ACTUATOR,            # < Not used
 APP_PARAM_TEMPERATURE_ELECTRONICS,         # < Actual temperature of the motor controller
 APP_PARAM_TORQUE,                          # < Not used
 APP_PARAM_ACCELERATION,                    # < Not used
 APP_PARAM_ACCELERATION_ROTOR,              # < Not used
 APP_PARAM_CURRENT_PHASE_1,                 # < Actual phase 1 current
 APP_PARAM_CURRENT_PHASE_2,                 # < Actual phase 2 current
 APP_PARAM_CURRENT_PHASE_3,                 # < Actual phase 3 current
 APP_PARAM_CURRENT_RAW,                     # < Not used
 APP_PARAM_CURRENT_RAW_2,                   # < Not used
 APP_PARAM_CURRENT_RAW_3,                   # < Not used
 APP_PARAM_ENCODER_MASTER_TRACK,            # < Internal use only
 APP_PARAM_ENCODER_NONIUS_TRACK,            # < Internal use only
 APP_PARAM_ENCODER_MOTOR_MASTER_TRACK,      # < Internal use only
 APP_PARAM_ENCODER_MOTOR_NONIUS_TRACK,      # < Internal use only
 APP_PARAM_TORQUE_ELECTRIC_CALC,            # < Internal use only
 APP_PARAM_CONTROLLER_VELOCITY_ERROR,       # < Velocity following error
 APP_PARAM_CONTROLLER_VELOCITY_SETPOINT,    # < Velocity target
 APP_PARAM_CONTROLLER_VELOCITY_FEEDBACK,    # < Actual velocity (degrees per second)
 APP_PARAM_CONTROLLER_VELOCITY_OUTPUT,      # < Not used
 APP_PARAM_CONTROLLER_POSITION_ERROR,       # < Position following error
 APP_PARAM_CONTROLLER_POSITION_SETPOINT,    # < Position target
 APP_PARAM_CONTROLLER_POSITION_FEEDBACK,    # < Actual position (degrees)
 APP_PARAM_CONTROLLER_POSITION_OUTPUT,      # < Not used
 APP_PARAM_CONTROL_MODE,                    # < Internal use only
 APP_PARAM_FOC_ANGLE,                       # < Internal use only
 APP_PARAM_FOC_IA,                          # < Internal use only
 APP_PARAM_FOC_IB,                          # < Internal use only
 APP_PARAM_FOC_IQ_SET,                      # < Internal use only
 APP_PARAM_FOC_ID_SET,                      # < Internal use only
 APP_PARAM_FOC_IQ,                          # < Internal use only
 APP_PARAM_FOC_ID,                          # < Internal use only
 APP_PARAM_FOC_IQ_ERROR,                    # < Internal use only
 APP_PARAM_FOC_ID_ERROR,                    # < Internal use only
 APP_PARAM_FOC_UQ,                          # < Internal use only
 APP_PARAM_FOC_UD,                          # < Internal use only
 APP_PARAM_FOC_UA,                          # < Internal use only
 APP_PARAM_FOC_UB,                          # < Internal use only
 APP_PARAM_FOC_U1,                          # < Internal use only
 APP_PARAM_FOC_U2,                          # < Internal use only
 APP_PARAM_FOC_U3,                          # < Internal use only
 APP_PARAM_FOC_PWM1,                        # < Internal use only
 APP_PARAM_FOC_PWM2,                        # < Internal use only
 APP_PARAM_FOC_PWM3,                        # < Internal use only
 APP_PARAM_FOC_TIMER_TOP,                   # < Internal use only
 APP_PARAM_DUTY,                            # < Internal use only
 APP_PARAM_SIZE                             # < Use when you need to define the total param arrray size
 ) = map(c_int, range(59))


class CtypesEnum(IntEnum):
    @classmethod
    def from_param(cls, obj):
        return int(obj)


class Servo(object):
    def __init__(self, library_api, servo_interface, identifier):
        self._api = library_api
        self._servo = servo_interface
        self._identifier = identifier

    def param_cache_update(self):
        self._api.rr_param_cache_update(self._servo)

    def param_cache_setup_entry(self, param: c_int, enabled: bool):
        self._api.rr_param_cache_setup_entry(self._servo, param, c_bool(enabled))

    def read_parameter(self, param: c_int):
        value = c_float()
        self._api.rr_read_parameter(self._servo, param, byref(value))
        return value.value

    def read_cached_parameter(self, param: c_int):
        value = c_float()
        self._api.rr_read_cached_parameter(self._servo, param, byref(value))
        return value.value

    def set_zero_position(self, position_deg: float):
        self._api.rr_set_zero_position(self._api, c_float(position_deg))

    def set_zero_position_and_save(self, position_deg: float):
        self._api.rr_set_zero_position_and_save(self._servo, c_float(position_deg))

    def get_max_velocity(self):
        velocity = c_float()
        self._api.rr_get_max_velocity(self._servo, byref(velocity))
        return velocity.value

    def set_max_velocity(self, max_velocity_deg_per_sec: float):
        self._api.rr_set_max_velocity(self._servo, c_float(max_velocity_deg_per_sec))

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
        return size.value

    def get_points_free_space(self):
        size = c_uint32(0)
        self._api.rr_get_points_free_space(self._servo, byref(size))
        return size.value

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
    time.sleep(1)
    t_servo.set_current(1)
    time.sleep(2)
    print(t_servo.read_parameter(APP_PARAM_DUTY))
    print(t_servo.get_points_free_space())
    time.sleep(2)
    t_servo.set_current(0)
    time.sleep(1)
