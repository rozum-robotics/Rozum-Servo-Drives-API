import time
import os
import logging
from functools import wraps
from rozum.util import Singleton
from rozum.constants import *

logger = logging.getLogger(__name__)
logger.addHandler(logging.NullHandler())
# TODO Switching servo working states: rr_setup_nmt_callback, rr_describe_nmt
# TODO normal logging
# TODO write doc strings
# TODO tests


class Servo(object):
    def __init__(self, library_api, servo_interface, identifier):
        self._api = library_api
        self._servo = servo_interface
        self._identifier = identifier

    @property
    def interface(self):
        return self._servo

    def param_cache_update(self):
        return self._api.rr_param_cache_update(self._servo)

    def param_cache_setup_entry(self, param: c_int, enabled: bool):
        return self._api.rr_param_cache_setup_entry(self._servo, param, c_bool(enabled))

    def read_parameter(self, param: c_int):
        value = c_float()
        status = self._api.rr_read_parameter(self._servo, param, byref(value))
        return value.value

    def read_cached_parameter(self, param: c_int):
        value = c_float()
        status = self._api.rr_read_cached_parameter(self._servo, param, byref(value))
        return value.value

    def set_zero_position(self, position_deg: float):
        return self._api.rr_set_zero_position(self._api, c_float(position_deg))

    def set_zero_position_and_save(self, position_deg: float):
        return self._api.rr_set_zero_position_and_save(self._servo, c_float(position_deg))

    def get_max_velocity(self):
        velocity = c_float()
        status = self._api.rr_get_max_velocity(self._servo, byref(velocity))
        return velocity.value

    def set_max_velocity(self, max_velocity_deg_per_sec: float):
        return self._api.rr_set_max_velocity(self._servo, c_float(max_velocity_deg_per_sec))

    def add_motion_point(self, position_deg: float, velocity_deg_per_sec: float, time_ms: int):
        return self._api.rr_add_motion_point(self._servo,
                                             c_float(position_deg), c_float(velocity_deg_per_sec), c_uint32(time_ms))

    def clear_points_all(self):
        return self._api.rr_clear_points_all(self._servo)

    def clear_points(self, num_to_clear: int):
        return self._api.rr_clear_points(self._servo, c_uint32(num_to_clear))

    def get_points_size(self):
        size = c_uint32(0)
        status = self._api.rr_get_points_size(self._servo, byref(size))
        return size.value

    def get_points_free_space(self):
        size = c_uint32(0)
        status = self._api.rr_get_points_free_space(self._servo, byref(size))
        return size.value

    def invoke_time_calculation(self,
                                start_position: float, start_velocity_deg_per_sec: float,
                                start_acceleration_deg_per_sec2: float, start_time_ms: int,
                                end_position: float, end_velocity_deg_per_sec: float,
                                end_acceleration_deg_per_sec2: float, end_time_ms: int):
        calculated_time = c_uint32(0)
        status = self._api.rr_invoke_time_calculation(self._servo,
                                                      c_float(start_position), c_float(start_velocity_deg_per_sec),
                                                      c_float(start_acceleration_deg_per_sec2), c_uint32(start_time_ms),
                                                      c_float(end_position), c_float(end_velocity_deg_per_sec),
                                                      c_float(end_acceleration_deg_per_sec2), c_uint32(end_time_ms),
                                                      byref(calculated_time))
        return calculated_time.value

    def release(self):
        return self._api.rr_release(self._servo)

    def freeze(self):
        return self._api.rr_freeze(self._servo)

    def set_current(self, current_a: float):
        return self._api.rr_set_current(self._servo, c_float(current_a))

    def brake_engage(self, en: bool):
        return self._api.rr_brake_engage(self._servo, c_bool(en))

    def set_velocity(self, velocity_deg_per_sec: float):
        return self._api.rr_set_velocity(self._servo, c_float(velocity_deg_per_sec))

    def set_position(self, position_deg: float):
        return self._api.rr_set_position(self._servo, c_float(position_deg))

    def set_velocity_with_limits(self, velocity_deg_per_sec: float, current_a: float):
        return self._api.rr_set_velocity_with_limits(self._servo, c_float(velocity_deg_per_sec), c_float(current_a))

    def set_position_with_limits(self, position_deg: float, velocity_deg_per_sec: float, current_a: float):
        return self._api.rr_set_position_with_limits(self._servo,
                                                     c_float(position_deg),
                                                     c_float(velocity_deg_per_sec),
                                                     c_float(current_a))

    def set_duty(self, duty_percent: float):
        return self._api.rr_set_duty(self._servo, c_float(duty_percent))

    def reboot(self):
        return self._api.rr_servo_reboot(self._servo)

    def reset_communication(self):
        return self._api.rr_servo_reset_communication(self._servo)

    def set_state_operational(self):
        return self._api.rr_servo_set_state_operational(self._servo)

    def set_state_pre_operational(self):
        return self._api.rr_servo_set_state_pre_operational(self._servo)

    def set_state_stopped(self):
        return self._api.rr_servo_set_state_stopped(self._servo)

    def read_error_status(self, array_size: int):
        error_count = c_uint32(0)
        error_array = (c_uint8 * array_size)()
        status = self._api.rr_read_error_status(self._servo, byref(error_count), byref(error_array))
        return error_count.value, error_array

    def deinit_servo(self):
        """The function deinitializes the servo, clearing all data associated with the servo descriptor.

        Automatically called on interface deinitialization.

        :return: Status code: int
        """
        return self._api.rr_deinit_servo(byref(self._servo))

    def _write_raw_sdo(self, idx: c_uint16, sidx: c_uint8, data: c_void_p, sz: c_int, retry: c_int, tout: c_int):
        """The function performs an arbitrary SDO write request to servo.

        Use on your own risk.

        :param idx: c_uint16:
            Index of SDO object
        :param sidx: c_uint8:
            Subindex
        :param data: c_void_p:
            Data to write to
        :param sz: c_int:
            Size of data in bytes
        :param retry: c_int:
            Number of reties (if communication error occured during request)
        :param tout: c_int:
            Request timeout in milliseconds
        :return: status: int
        """
        return self._api.rr_write_raw_sdo(self._servo, idx, sidx, data, sz, retry, tout)


class Interface(object):

    def __init__(self, library_api, interface_name):
        self._api = library_api
        self._api.rr_init_interface.restype = c_void_p
        self._api.rr_init_servo.restype = c_void_p
        self._interface = c_void_p(self._api.rr_init_interface(bytes(interface_name, encoding="utf-8")))
        if self._interface is None:
            message = "Failed to initialize interface by name: {}".format(interface_name)
            logger.error(message)
            raise AttributeError(message)
        self._servos = {}
        time.sleep(0.5)  # for interface initialization

    def start_motion(self, timestamp_ms: int):
        return self._api.rr_start_motion(self._interface, c_uint32(timestamp_ms))

    def init_servo(self, identifier) -> Servo:
        """See docstring of ServoApi.init_servo()"""
        if identifier not in self._servos:
            servo_interface = c_void_p(self._api.rr_init_servo(self._interface, c_byte(identifier)))
            if servo_interface is None:
                message = "Failed to initialize servo by id: {}".format(identifier)
                logger.error(message)
                raise AttributeError(message)
            self._servos[identifier] = Servo(self._api, servo_interface, identifier)
        return self._servos[identifier]

    def change_id_and_save(self, old_id: int, new_can_id: int):
        """The function enables changing the default CAN identifier (ID) of the specified servo to avoid collisions on
        a bus line. **Important!** Each servo connected to a CAN bus must have **a unique ID**.

        When called, the function resets CAN communication for the specified servo, checks that Heartbeats are generated
        for the new ID, and saves the new CAN ID to the EEPROM memory of the servo.

        **Note:** The EEPROM memory limit is 1,000 write cycles. Therefore, it is advisable to use the function with
        discretion.

        :param old_id: int:
            Old CAN ID.
        :param new_can_id: int:
            New CAN ID. You can set any value within the range from 1 to 127, only make sure no other servo has the same
            ID.
        :return: status : int
        """
        servo_interface = self.init_servo(old_id).interface
        status = self._api.rr_change_id_and_save(self._interface, servo_interface, c_uint8(new_can_id))
        del self._servos[old_id]
        return status

    def net_reboot(self):
        """The function reboots all servos connected to the current interface, resetting them back to the power-on state.

        :return: Status code: int
        """
        return self._api.rr_net_reboot(self._interface)

    def net_reset_communication(self):
        """The function resets communication on current interface.

        For instance, you may need to use the function when changing settings that require a reset after modification.

        :return: Status code: int
        """
        return self._api.rr_net_reset_communication(self._interface)

    def net_set_state_operational(self):
        """The function sets all servos connected to the current interface (CAN bus) to
        the operational state. In the state, the servos can both communicate with the user program and execute commands.

        For instance, you may need to call the function to switch all servos on a specific bus from the pre-operational
        state to the operational one after an error (e.g., due to overcurrent).

        :return: Status code: int
        """
        return self._api.rr_net_set_state_operational(self._interface)

    def net_set_state_pre_operational(self):
        """The function sets all servos connected to the current interface to the pre-operational state.
        In the state, the servos are available for communication, but cannot execute commands.

        For instance, you may need to call the function, if you want to force all servos on a specific bus to stop
        executing commands, e.g., in an emergency.

        :return: Status code: int
        """
        return self._api.rr_net_set_state_pre_operational(self._interface)

    def net_set_state_stopped(self):
        """The function sets all servos connected to the interface specified in the 'interface' parameter to the stopped state.
        In the state, the servos are neither available for communication nor can execute commands.

        For instance, you may need to call the fuction to stop all servos on a specific bus without deinitializing them.

        :return: Status code: int
        """
        return self._api.rr_net_set_state_stopped(self._interface)

    def deinit_interface(self):
        """The function closes the COM port where the corresponding CAN-USB dongle is connected, clearing all data
        associated with the interface descriptor.

        Automatically called when ServoApi is deinitializing. In addition performs deinitialization on all registered
        servos.

        :return: None
        """
        for servo in self._servos.values():
            if servo is not None:
                servo.deinit_servo()
        return self._api.rr_deinit_interface(byref(self._interface))


class ServoApi(object, metaclass=Singleton):
    __LIBRARY_NAME = "libservo_api"

    def __init__(self):
        self._api = None
        self._interface = None

    def _check_library_loaded(self):
        if self._api is None:
            raise AttributeError("Library not loaded. Call api.load_library(path_to_library) first.")

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
        """The function is the first to call to be able to work with the user API.

        It tries to load library with respect to given path. If path is not provided, it tries to find library in
        current directory.

        :param library_path: str or None
        :return: None
        """
        if library_path is None and self._api is None:
            self._api = CDLL(os.path.join(os.path.dirname(__file__), "libservo_api.so"))
        else:
            if ServoApi.__LIBRARY_NAME not in library_path:
                raise ArgumentError("Expected that path to library contains " + ServoApi.__LIBRARY_NAME)
            if self._api is None:
                self._api = CDLL(library_path)

    def init_interface(self, interface_name: str) -> Interface:
        """The function is the second to call to be able to work with the user API.

        It opens the COM port where the corresponding CAN-USB dongle is connected, enabling communication between the
        user program and the servo motors on the respective CAN bus.

        Interface name string examples:
            * Linux: "/dev/ttyACM0" **or** "/dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00"
            * MacOS: "/dev/cu.modem301"
            * Windows (Cygwin): "/dev/ttyS0"
        *Note: last numbers in "/dev/.." strings may differ on your machine.*

        :param interface_name: str
        :return: Interface instance
        """
        self._check_library_loaded()
        if self._interface is None:
            self._interface = Interface(self._api, interface_name)
        return self._interface

    def init_servo(self, identifier: int) -> Servo:
        """_init_servo_label:
        The function determines whether the servo motor with the specified ID is connected to the specified interface.

         It waits for 2 seconds to receive a Heartbeat message from the servo. When the message arrives within the
         interval, the servo is identified as successfully connected.

        :param identifier: int:
            Unique identifier of the servo in the specified interface. The available value range is from 0 to 127.
        :return: Servo instance
        """
        return self.interface.init_servo(identifier)

    def sleep_ms(self, ms: int):
        """The function sets an idle period for the user program (e.g., to wait till a servo executes a motion trajectory).

        Until the period expires, the user program will not execute any further operations.
        However, the network management, CAN communication, emergency, and Heartbeat functions remain available.
        Note:The user can also call system-specific sleep functions directly.
        However, using this sleep function is preferable to ensure compatibility with subsequent API library versions.

        :param ms: int:
            Idle period (in milleseconds)
        :return: None
        """
        self._api.rr_sleep_ms(c_int(ms))

    def describe_emcy_bit(self, bit: c_uint8):
        return self._api.rr_describe_emcy_bit(bit)

    def describe_emcy_code(self, code: c_uint16):
        return self._api.rr_describe_emcy_code(code)

    def __del__(self):
        self.interface.deinit_interface()
