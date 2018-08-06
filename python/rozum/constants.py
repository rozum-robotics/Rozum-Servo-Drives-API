from ctypes import *

RET_STATUS_T = (RET_OK,                                    # < Status OK
                RET_ERROR,                                 # < Generic error
                RET_BAD_INSTANCE,                          # < Bad interface or servo instance (null)
                RET_BUSY,                                  # < Device is busy
                RET_WRONG_TRAJ,                            # < Wrong trajectory
                RET_LOCKED,                                # < Device is locked
                RET_STOPPED,                               # < Device is in STOPPED state
                RET_TIMEOUT,                               # < Communication timeout
                RET_ZERO_SIZE,                             # < Zero size
                RET_SIZE_MISMATCH,                         # < Received & target size mismatch
                RET_WRONG_ARG                              # < Wrong function argument
                ) = list(map(c_int, range(11)))

RET_MESSAGES = (
                "Status OK",
                "Generic error",
                "Bad interface or servo instance (null)",
                "Device is busy",
                "Wrong trajectory",
                "Device is locked",
                "Device is in STOPPED state",
                "Communication timeout",
                "Zero size",
                "Received & target size mismatch",
                "Wrong function argument",
)

RET_STATUS_MESSAGE = {x[0].value: x[1] for x in zip(RET_STATUS_T, RET_MESSAGES)}

SERVO_PARAM_T = (
    APP_PARAM_NULL,                            # < Not used
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
 ) = list(map(c_int, range(59)))