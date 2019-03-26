# Rozum-Servo-Drives-API

### About
The repository contains code of libraries and utilities, which is intended to enable control of [Rozum Robotics RDrive-series servo motors]((https://rozum.com/servomotors/)). For detailed technical specifications of RDrive servo motors, download the datasheet for the series at [documentation](https://rozum.com/tpl/pdf/Servo/servomotors_Rozum_Robotics_quality.pdf).

### Project structure
1. The `c` folder includes a library in the C language and some usfull utilities for update, calibrate and solve problems.  
Online documentation on the user API is available [here](https://rozum.com/support/servo-api/c/docs) also, you can download [offline PDF documentation](https://rozum.com/support/servo-api/c/docs/C-Servo-API.pdf)
2. The `python` folder is wrapper for the `c` library in the python programing language.  
For details of implementation see the [Readme file](https://github.com/rozum-robotics/Rozum-Servo-Drives-API/tree/master/python) in the respective directory. 
Online documentation is available [here](https://rozum.com/support/servo-api/python/docs).
3. The `debug-tools` is Octave/MATLAB scripts which represents interpolation algorithm implemented inside RozumRobotics servo drives.
4. `debian` and `bin` contains internal scripts.
