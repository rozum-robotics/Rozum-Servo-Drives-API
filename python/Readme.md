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
   
   `rozum/servo` - package containing api
   
   `rozum/servo/tutorial` - examples for quick start
   
### Installation
After building the library, copy `libservo_api.so` file into `rozum/servo` directory. 
Optionally you can run `make python` in console after building. It will perform copying operation.

Currently, installation via **pip** is not supported. 
So, copy the `rozum` module into your project root manually.

##### For Windows OS:
1. Install Cygwin.
2. Install `make`, `gcc`, `python3` packages in Cygwin.
3. Follow [the building guide](../c/README.md) using the Cygwin console.
4. Use the `python3` interpreter in Cygwin to run your programs.


### Getting started
1. Build the libservo_api shared library. Make sure to copy it into the `rozum` folder.
2. Find out the **_can_** interface name. 
_Hint:_ For Linux, you can find it out by executing `ls /dev/serial/by-id/` command in the console.
The output should contain something like: `usb-Rozum_Robotics_USB-CAN_Interface_301-if00`.
Alternatively, you can execute `ls /dev/` in the console. In this case, the output should be of the following type: `ttyACM1` (`ttyS1` in Cygwin). 
_Note:_ The last number may differ on your machine.
3. Navigate to the `tutorials` folder and replace the constants in `__init__.py` with relevant values. The constants are as follows:
  * `LIBRARY_PATH` - full path to the library (string)
  * `INTERFACE_NAME` - name of the interface (output from Step 2), e.g., "/dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00"
  * `SERVO_N_ID` - your servo ID  
_Note:_ You can leave `LIBRARY_PATH = None` if you copied the library into the `rozum` folder in Step 1.
4. Run `python path_to_tutorials/control_servo_traj_1.py` or any other tutorial in the console.

### Usage
Below is the usual sequence of working with servos. For detailed instructions, refer to `tutorials`.
```python
# importing modules
import rozum as rr

# api initialization and library loading
api = rr.ServoApi()

# interface initialization
interface = api.init_interface("/dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00")

# servo initialization
servo = interface.init_servo(64)

# servo usage
# preparing specific realtime parameters for extraction
servo.param_cache_setup_entry(rr.APP_PARAM_CURRENT_INPUT, True)
servo.param_cache_setup_entry(rr.APP_PARAM_VOLTAGE_INPUT, True)

# adding motion points
servo.add_motion_point(100., 0., 6000)
servo.add_motion_point(-100., 0., 6000)
servo.add_motion_point(0, 0, 6000)

# starting motion
interface.start_motion(0)

# reading realtime parameters from cache
servo.param_cache_update()
current_input = servo.read_cached_parameter(rr.APP_PARAM_CURRENT_INPUT)
voltage_input = servo.read_cached_parameter(rr.APP_PARAM_VOLTAGE_INPUT)
print("current_input = {}, voltage_input = {}".format(current_input, voltage_input)) # printing them

# direct reading of realtime parameters
velocity_rotor = servo.read_parameter(rr.APP_PARAM_VELOCITY_ROTOR)
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

