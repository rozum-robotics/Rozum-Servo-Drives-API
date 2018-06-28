# C Servo library

1. Introduction

The `c` folder includes a library in the C language, which is intended to enable control of Rozum Robotics servo motors.

2. File structure

    `build` - folder containing the compiled library (dynamic and static) and
              intermediate compiler files
        
    `html` - Doxygen-generated documentation
    
    `include` - library header files
    
    `src` - library sources
    
    `test` - tests
    
    `tutorial` - examples for quick start
    
    `.vimrc` - vim config (tab width - 4 spaces, F8 - clean, F9 - make)
    
    `Doxyfile` - Doxygen project configuration
    
    `Makefile` - make rules (configuration)
    
    `README.md` - this file
    
    `core.mk` - make rules (compile logic itself)

3. Compiling

    Run:
    
        make clean; make
        
    Two files should be created: `build/libservo_api.so` and `build/libservo_api.a`.

4. Building the documentation

    Run:
    
        doxygen
        
    The `html` folder should be created. Open html/index.html in a browser to view the
    documentation.

5. Running a tutorial

    Go to the tutorial folder:
    
        cd tutorial
        
    Open and edit `tutorial.h`. Replace the `TUTORIAL_DEVICE` value with your device:
    
        name (e.g.,`/dev/ttyACM0`)
        
    Run:
    
        make

6. Static linking

    To link statically, pass `libservo_api.a` to linker together with other 
    object files:
    
        gcc -o app obj1.o obj2.o path_to_library/libservo_api.a -lpthread
        
7. Dynamic linkinkg

    To link against the shared library, specify the path to the library using the `-L` linker
    option and the library name using the `-l option:
    
        gcc -o app obj1.o obj2.o -Lpath-to-library -lservo_api -lpthread
        
    To run application without installing the library into the system, find the
    `rpath` path and add the linker option:
    
        gcc -o app obj1.o obj2.o -Lpath-to-library -lservo_api -lpthread \
        -Wl,-rpath=relative-path-to-library