# Python-API
This folder contains `Python 3` wrapper for the `C` library. 
Implementation is based on the `ctypes` module.

### Requirements
1. Operating systems: Linux, Mac OS, Windows
2. Python version 3.5 or later.
3. No extra Python packages required.

### File structure
   `rdrive` - package root
   
   `rdrive/servo` - package containing api
   
   `examples` - examples for quick start
   
### Installation
To get the latest version, use the following command:

`pip install rdrive -i https://pip.rozum.com/simple`

To install a specific version: 

`pip install rdrive==v1.v2.v3 -i https://pip.rozum.com/simple`
where **v1**, **v2**, and **v3** (e.g., rdrive==1.4.3) are version numbers.

**Note:** On MacOS and Linux make sure to install `gcc` compiler and `make`.

### Getting started
1. Find out the **_CAN_** interface name. 
    * On Linux, you can find it out by executing the `ls /dev/serial/by-id/` command in the console.
    The output should contain something like: `usb-Rozum_Robotics_USB-CAN_Interface_301-if00`.
    Alternatively, you can execute `ls /dev/` in the console. In this case, the output should be of the following type: `ttyACM1` (`ttyS1` in Cygwin). 
    _Note:_ The last number may differ on your machine.
    * On Windows - open **Device manager** and find USB devices. The name should be of the following type: `COM3`.
    * On MacOS, you can find it out by executing the `ls /dev/ | grep cu.usb` command in the console.
    The output should contain something like: `/dev/cu.usbmodem301`
2. Use one of the scripts in the `examples` folder as a reference.

### Basic usage
```python

import rdrive as rr

api = rr.ServoApi() # api initialization and library loading
interface = api.init_interface("/dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00") # interface initialization
servo = interface.init_servo(64) # servo initialization

# go to specific position(degrees)
servo.set_position(100.)

# set specific velocity(degrees/sec)
servo.set_velocity(204.5)


```

### Advanced usage
Below is the usual sequence of working with servos. For detailed instructions, refer to `examples`.
```python
# importing modules
import rdrive as rr

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

# set motor trajectory (adding motion points - position(deg), velocity(deg/sec), time(ms))
servo.add_motion_point(100., 0., 6000)
servo.add_motion_point(-100., 0., 6000)
servo.add_motion_point(0, 0, 6000)

# start motion with delay(ms)
interface.start_motion(0)

# reading realtime parameters from cache
servo.param_cache_update()
current_input = servo.read_cached_parameter(rr.APP_PARAM_CURRENT_INPUT)
voltage_input = servo.read_cached_parameter(rr.APP_PARAM_VOLTAGE_INPUT)
print("current_input = {}, voltage_input = {}".format(current_input, voltage_input)) # printing parameters

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

