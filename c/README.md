# C Servo library

1. Introduction
2. 
2. File structure
    buid - folder containing compiled library (dynamic and static) and 
        intermidiate compiler files
    html - Doxygen generated documentation
    include - library header files
    src - library sources
    test - tests
    tutorial - examples for quick starting
    .vimrc - vim config (tabwidth - 4 spaces, F8 - clean, F9 - compile)
    Doxyfile - Doxygen project configuration
    Makefile - make rules (configuration)
    README.md - this file
    core.mk - make rules (compile logic itself)

3. Compiling
    Run:
        make clean; make
    Two files `build/libservo_api.so` & `build/libservo_api.a` should be created

4. Building documentation
    Run:
        doxygen
    `html` folder should be created. Open html/index.html using browser to view
    documentation.

5. Running tutorial
    Go to tutorial folder:
        cd tutorial
    Open & edit `tutorial.h`. Replace `TUTORIAL_DEVICE` value with actual device
        name (`/dev/ttyACM0` for example).
    Run:
        make

6. Building new project
6.1. Static linking
    To link statically pass `libservo_api.a` to linker together with other 
    object files:
        gcc -o app obj1.o obj2.o path_to_library/libservo_api.a -lpthread
6.2. Dynamic linkinkg
    To link against shared library supply path to the libraray using `-L` linker
    option & library name using `-l option:
        gcc -o app obj1.o obj2.o -Lpath-to-library -lservo_api -lpthread
    To run application without installing library into system search path
    `rpath` linker option should be added:
        gcc -o app obj1.o obj2.o -Lpath-to-library -lservo_api -lpthread \
        -Wl,-rpath=relative-path-to-library