# C Servo library

## Introduction

The `c` folder includes a C-language library intended to enable control of Rozum Robotics RDrive-series servo motors.
   
For more information about the RDrive series, go to: https://rozum.com/servomotors/.
    
For detailed technical specifications of RDrive servo motors, download the datasheet for the series at https://rozum.com/assets/files/rdrive/servomotors_rozum_robotics.pdf.
    
Online documentation about the Servo API is available at https://rozum.com/support/servo-api/c/docs/.

Offline PDF documentation about the Servo API is available at https://rozum.com/support/docs/servo-api/RR-UserAPI.pdf.
    
## System and software requirements

Operating systems: Linux, Windows (Cygwin), Mac OS.
   
For compiling, use the gcc compiler.
   
Additionally, make sure to install `libpthread` library and `make`.

## Compiling

Run:
    
`make clean; make`
        
As a result, two files should be created: `build/libservo_api.so` and `build/libservo_api.a`.

**Note: You may skip further steps in case you are going to use the library within a higher-level API application— i.e., Python**

## Building the documentation

1. Make sure `Doxygen` is installed (for generating documentation).

2. run:
    
`doxygen`
        
The `html` folder should be created. Open html/index.html in a browser to view the documentation.

## Compiling tutorials

1. Go to the tutorial folder:
    
`cd tutorial`
        
2. Open `tutorial.h`. Replace the `TUTORIAL_DEVICE` value with the identifications of your device:
    
name (e.g.,`/dev/ttyACM0`)
        
3. Run:
    
`make`

## Running tutorials

Go to the `build` folder created during compilation. Run the executable files from this folder, one by one, to check operation of the servos.

**Note: Before running the executable files, make sure the hardware (servos, a servo box, a power supply) is connected as appropriate.**

## Static linking

To link statically, pass `libservo_api.a` to the linker together with other object files:
    
`gcc -o app obj1.o obj2.o path_to_library/libservo_api.a -lpthread`
        
## Dynamic linking

To link against the shared library, specify the path to the library using the `-L` linker option and the library name using the `-l` option:
    
`gcc -o app obj1.o obj2.o -Lpath-to-library -lservo_api -lpthread`
        
## File structure

`build` - folder containing the compiled library (dynamic and static) and intermediate compiler files
        
`html` - Doxygen-generated HTML documentation

`latex` - Doxygen-generated documentation (Latex & PDF versions)

`doc-src` - servobox hardware documentation sources
    
`include` - header files of the library
    
`src` - library sources
    
`test` - tests
    
`tutorial` - examples for quick start
    
`.vimrc` - vim config (tab width - 4 spaces, F8 - clean, F9 - make)
    
`Doxyfile` - Doxygen project configuration
    
`Makefile` - make rules (configuration)
    
`README.md` - this file
    
`core.mk` - make rules (compilation logic itself)
