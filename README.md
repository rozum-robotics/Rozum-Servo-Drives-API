# Rozum-Servo-Drives-API

### About
The repository contains libraries and utilities intended to enable control of [Rozum Robotics RDrive-series servo motors]((https://rozum.com/servomotors/)).
For detailed technical specifications of RDrive servo motors, download the datasheet for the series at [documentation](https://rozum.com/assets/files/rdrive/servomotors_rozum_robotics.pdf).

### Project structure
1. The `c` folder includes a library in the `c` language and usefull utilities for updating, calibrating, and resolving issues.  
Online documentation about the user API is available at [here](https://rozum.com/support/servo-api/c/docs).
Also, you can download [offline PDF documentation](https://rozum.com/support/servo-api/c/docs/C-Servo-API.pdf).
2. The `python` folder is a Python wrapper for the `c` library.  
For details of the implementation, see the [Readme file](https://github.com/rozum-robotics/Rozum-Servo-Drives-API/tree/master/python) in the respective directory. 
Online documentation is available [here](https://rozum.com/support/servo-api/python/docs).
3. The `debug-tools` folder contains Octave/MATLAB scripts — an interpolation algorithm implemented for Rozum Robotics servo drives.
4. The `debian` and `bin` contain internal scripts.
