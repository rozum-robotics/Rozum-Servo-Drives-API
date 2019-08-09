# Python-API
This folder contains `Python 3` wrapper for the `C` library. 
Implementation is based on the `ctypes` module.

## Prerequisites
1. Operating systems (OS): Linux, macOS, Windows.<br>
2. Python 3.5 or a later version.<br>
   To install Python, proceed as below, depending on your OS:<br>
   **Windows**: Navigate to [Python](https://www.python.org/downloads/windows/). Download and complete the setup process.<br>
   **Important!** At the installation setup screen, make sure to check **Add Python v.Х to PATH** (where **v.Х.** is the downloaded Python version).
   
   **Linux**: To install Python and a corresponing pip package, run one of the following commands in the console.
   In case you have a Linux distribution other than specified below, proceed in the usual order of running the installation pocedure in your system.<br> 
   **Ubuntu/Debian**: Run the command `sudo apt install python3 python3-pip`.<br>
   **Fedora starting from version 22**: Run the command `sudo dnf install python3 python3-pip`.<br>
   **RedHat and Fedora before version 22**: Run the command `sudo yum install python3 python3-pip`.<br>
3. `pip 3` package<br>
   **Important!** The Python installation package for Windows already contains a `pip` package. For Ubuntu/Debian and other Linux systems, see Step 2 above.
4. On MacOS and Linux, make sure to install the `gcc` compiler and `make`.
   
## Installation Windows OS

**Note:** If your PC runs Windows 8 or earlier versions of the operating system, download and install a driver for the CAN-USB dongle from the [web page](https://www.st.com/en/development-tools/stsw-stm32102.html).

To install the API library on a PC running under Windows OS, run the pip install command as described below:

- To get the latest version, use the following command:
  `pip install rdrive -i https://pip.rozum.com/simple`

- To install a specific version:
  `pip install rdrive==v1.v2.v3 -i https://pip.rozum.com/simple`
   
where **v1**, **v2**, and **v3** (e.g., rdrive==1.0.31) are version numbers.

Now, the API library for Windows is installed, and you can proceed to working with the cmd console (for further steps, refer to **Getting started Windows OS**).

## Installation Linux

To install the API library on a PC running under Linux OS, run the pip install command as described below:

- To get the latest version, use the following command:
  `pip3 install rdrive -i https://pip.rozum.com/simple`

- To install a specific version: 
  `pip3 install rdrive==v1.v2.v3 -i https://pip.rozum.com/simple`
where **v1**, **v2**, and **v3** (e.g., rdrive==1.0.31) are version numbers.

## Getting started Windows OS

To run any of the program examples with Python code on Windows OS, complete the steps below.

**Note:** Before running a tutorial, RDrive servo should be wired to a power supply and the CAN-USB dongle.
as described in the User Manual or the Quick-Start Guide available for downloading at the [company website] (https://rozum.com/servomotors/#rdrive-documentation).

1. Open the Command Line Interface via the Start menu.

2. Find out the **_CAN_** interface name. To do this, complete the following steps:
   - Open the start **Menu** - **System Settings** - **Devices** - **Bluetooth and other devices**.<br>
   - On the **Other devices** list find a serial USB device with a COM port. The COM port number is the CAN ID interface you need—e.g., **COM3**.
   
3. Find out the **CAN ID of the connected servo motor**. To do this, complete the following steps:

   - In the command console, run a tutorial from the `examples` folder by executing the following command with a specified CAN Interface ID and CAN Servo ID.
   
     `python ..\userapi\python\examples\read_servo_max_velocity.py --interface COM3 --servo_1_id 32`

   where **read_servo_max_velocity.py** is the tutorial name;<br>
   **interface COM3** is the parameter specifying the CAN Interface ID;<br>
   **servo_1_id 37** is the parameter specifying an arbitrary CAN ID for the connected servo.
   
   - In the command output, go to the INFO lines (see the example below). Look for IDs within the range between 32 to 37 — default servo IDs (37 in the example below).
   
   **Example:**<br/>
`INFO:	ID: 50 Device is in operational mode`<br/>
`INFO:	ID: 37 Device is in pre-operational mode`   
 
 4. Now, you can run any of the tutorials from the `examples` folder to move your RDrive servo or read parameters from it.
 
 To do this, run the following command in the cmd console:
 
 `python ..\userapi\python\examples\read_servo_max_velocity.py --interface COM3 --servo_1_id 37`
 
   where **read_servo_max_velocity.py** is the tutorial name (replace the name from the current example with any other tutorial name as needed);<br>
   **interface COM3** is the parameter specifying the CAN Interface ID (the one we got at Step 2);<br>
   **servo_1_id 37** is the parameter specifying an arbitrary CAN ID for the connected servo (the one we got at Step 3).

If the command is successfully executed, the connected servo will behave as commanded—return parameters or move.

**Note:** Some of the tutorials may require specifing some other parameters in addition to the CAN Interface ID and a CAN Servo ID. 
 
## Getting started Linux OS

To run any of the program examples with Python code on Linux OS, complete the steps below.

1. Find out the **_CAN_** interface name. To do this, open the console and type in the following command: `ls /dev/serial/by-id/`.<br>
   In the output, look for something like: `usb-Rozum_Robotics_USB-CAN_Interface_301-if00`. It is the CAN interface name you need.

**Note:** On MacOS, you can find out the CAN Interface name by executing the `ls /dev/ | grep cu.usb` command in the console. The output will contain something like: `/dev/cu.usbmodem301`
   
2. Find out the **CAN ID of the connected servo motor**. To do this, complete the following steps:

 - In the console, run a tutorial from the `examples` folder by executing the following command with a specified CAN Interface ID and a CAN Servo ID.
   
     `python3 ..\userapi\python\examples\read_servo_max_velocity.py --interface /dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00 --servo_1_id 32`

   where **read_servo_max_velocity.py** is the tutorial name;
   **interface /dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00** is the parameter specifying the CAN Interface ID;<br>
   **servo_1_id 37** is the parameter specifying an arbitrary CAN ID for the connected servo.
   
 - In the command output, go to the INFO lines (see the example below). Look for IDs within the range between 32 to 37 — default servo IDs (37 in the example below).
  
   **Example:**<br/>
   `INFO:   ID: 50 Device is in operational mode`<br/>
   `INFO:	ID: 37 Device is in pre-operational mode`
       
3. Now, you can run any of the tutorials from the `examples` folder to move your RDrive servo or read parameters from it.

   To do this, run the following command in the console:
 
   `python3 ..\userapi\python\examples\read_servo_max_velocity.py --interface /dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00 --servo_1_id 37`
 
   where **read_servo_max_velocity.py** is the tutorial name (replace the name from the current example with any other tutorial name as needed);<br> 
   **interface /dev/serial/by-id/usb-Rozum_Robotics_USB-CAN_Interface_301-if00** is the parameter specifying the CAN Interface ID (the one we got at Step 1);<br>
   **servo_1_id 37** is the parameter specifying an arbitrary CAN ID for the connected servo (the one we got at Step 2).

If the command is successfully executed, the connected servo will behave as commanded—return parameters or move.

**Note:** Some of the tutorials may require specifing some other parameters in addition to the CAN Interface ID and a CAN Servo ID.

## Python script examples 

The two subsections below contain examples of Python scripts based on available Python API commands.

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

