# C Servo library

1. Introduction

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
        
    Two files `build/libservo_api.so` and `build/libservo_api.a` should be created

4. Building documentation

    Run:
    
        doxygen
        
    `html` folder should be created. Open html/index.html in a browser to view
    documentation.

5. Running tutorial

    Go to the tutorial folder:
    
        cd tutorial
        
    Open and edit `tutorial.h`. Replace `TUTORIAL_DEVICE` value with your device
    
        name (`/dev/ttyACM0`, for example).
        
    Run:
    
        make

6. Static linking

    To link statically, pass `libservo_api.a` to linker together with other 
    object files:
    
        gcc -o app obj1.o obj2.o path_to_library/libservo_api.a -lpthread
        
7. Dynamic linkinkg

    To link against the shared library supply path to the library using the `-L` linker
    option and the library name using `-l option:
    
        gcc -o app obj1.o obj2.o -Lpath-to-library -lservo_api -lpthread
        
    To run application without installing the library into the system search path
    `rpath`, add the linker option:
    
        gcc -o app obj1.o obj2.o -Lpath-to-library -lservo_api -lpthread \
        -Wl,-rpath=relative-path-to-library