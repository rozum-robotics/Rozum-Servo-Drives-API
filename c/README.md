# C Servo library

## Introduction

The `c` folder includes a library in the C language, which is intended to enable control of Rozum Robotics RDrive-series servo motors.
   
For more information about the RDrive series, go to: https://rozum.com/servomotors/.
    
For detailed technical specifications of RDrive servo motors, download the data-sheet for the series at: https://rozum.com/tpl/pdf/RD_50-85.pdf.
    
Online documentation on the user API: https://rozum.com/support/docs/servo-api

Offline PDF documentation on the user API: https://rozum.com/support/docs/servo-api/RR-UserAPI.pdf
    
## System and software requirements

Operating systems: Linux, Mac OS, Windows (Cygwin)
   
For compiling, use the gcc compiler.
   
Additionally, make sure to install `libpthread` library and `make`

## Compiling

Run:
    
`make clean; make`
        
Two files should be created: `build/libservo_api.so` and `build/libservo_api.a`.

**Note: You may skip further steps in case you are going to use the library within the high-level API appllication(i.e. Python)**

## Building the documentation

Make sure `Doxygen` is installed (for generating the documentation)

Then run:
    
`doxygen`
        
The `html` folder should be created. Open html/index.html in a browser to view the documentation.

## Compiling tutorials

Go to the tutorial folder:
    
`cd tutorial`
        
Open and edit `tutorial.h`. Replace the `TUTORIAL_DEVICE` value with your device:
    
name (e.g.,`/dev/ttyACM0`)
        
Run:
    
`make`

## Running tutorials

Go to the `build` folder created during compilation. Run the executable files from this folder, one by one, to check operation of the servos.

**Note: Before running the executable files, make sure the hardware (servos, a servo box, a power supply) is connected as appropriate.**

## Static linking

To link statically, pass `libservo_api.a` to linker together with other object files:
    
`gcc -o app obj1.o obj2.o path_to_library/libservo_api.a -lpthread`
        
## Dynamic linking

To link against the shared library, specify the path to the library using the `-L` linker option and the library name using the `-l` option:
    
`gcc -o app obj1.o obj2.o -Lpath-to-library -lservo_api -lpthread`
        
## File structure

`build` - folder contains the compiled library (dynamic and static) and intermediate compiler files
        
`html` - Doxygen-generated documentation HTML version
    
`latex` - Doxygen-generated documentation latex & PDF versions

`doc-src` - Servobox hardware documentation sources
    
`include` - header files of the library
    
`src` - library sources
    
`test` - tests
    
`tutorial` - examples for quick start
    
`.vimrc` - vim config (tab width - 4 spaces, F8 - clean, F9 - make)
    
`Doxyfile` - Doxygen project configuration
    
`Makefile` - make rules (configuration)
    
`README.md` - this file
    
`core.mk` - make rules (compile logic itself)
