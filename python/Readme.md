# Python-API
This folder contains `Python 3` wrapper for the `C` library. 
Implementation is based on the `ctypes` module.

### Requirements
1. Operating systems: Linux, Mac OS, Windows (Cygwin)
2. Python version 3.5 or later.
3. No extra Python packages required.
4. libservo_api shared library. For building instructions, refer [here](../c/README.md).

### File structure
   `rozum` - package root
   
   `tutorial` - examples for quick start
   
   `contants.py` - useful and utility constants
   
   `servo.py` - wrapper for the `C` library

   `util.py` - utility functions
   
### Installation
Currently, installation via **pip** is not supported. 
So, copy the `rozum` module into your project root manually.

### Getting started
1. Build the libservo_api shared library. Make sure to copy it into the `rozum` folder.
2. Find out **_can_** interface name. 
_Hint_: For Linux, type in `ls /dev/serial/by-id/` in the terminal. The response should be something like `usb-Rozum_Robotics_USB-CAN_Interface_301-if00`.
3. Navigate to the `tutorials` folder and replace the constants in `__init__.py` with your relevant values. The constants are as follows:
  * `LIBRARY_PATH` - full path to the library (string)
  * `INTERFACE_NAME` - name of the interface (response from Step 2), e.g., "/dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00" 
   _Note:_ You can leave `LIBRARY_PATH = None` if you copied library into `rozum` folder in Step 1.
4. Run `python path_to_tutorials/control_servo_traj_1.py` or any other tutorial in the terminal.

### Usage
Below is the usual sequence of working with servos. For detailed instructions, refer to `tutorials`.
```python
# importing modules
from rozum.constants import *
from rozum.servo import ServoApi

# api initialization
api = ServoApi()

# library loading
api.load_library()  # or api.load_library("path_to_library") if library file is not in rozum folder

# interface initialization
interface = api.init_interface("/dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00")

# servo initialization
servo = api.init_servo(64)

# servo usage
# preparing specific realtime parameters for extraction
servo.param_cache_setup_entry(APP_PARAM_CURRENT_INPUT, True)
servo.param_cache_setup_entry(APP_PARAM_VOLTAGE_INPUT, True)

# adding motion points
servo.add_motion_point(100., 0., 6000)
servo.add_motion_point(-100., 0., 6000)
servo.add_motion_point(0, 0, 6000)

# starting motion
interface.start_motion(0)

# reading realtime parameters from cache
servo.param_cache_update()
current_input = servo.read_cached_parameter(APP_PARAM_CURRENT_INPUT)
voltage_input = servo.read_cached_parameter(APP_PARAM_VOLTAGE_INPUT)
print("current_input = {}, voltage_input = {}".format(current_input, voltage_input)) # printing them

# direct reading of realtime parameters
velocity_rotor = servo.read_parameter(APP_PARAM_VELOCITY_ROTOR)
print("velocity_rotor = {}".format(velocity_rotor))

# awaiting motion
api.sleep_ms(20000)

# simple motion control
servo.set_current(5)
api.sleep_ms(1000)
servo.set_current(0)

# reading errors
num, arr = servo.read_error_status(100)
for i in range(num):
    api.describe_emcy_bit(arr[i])

```

