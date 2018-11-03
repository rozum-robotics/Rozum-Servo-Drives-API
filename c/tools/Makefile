#
#Verbose mode
#
VERBOSE=no

#
#Colorize ouput
#
COLORIZE=no

#
#Build directory
#
BUILDDIR = build

#
#Executable name
#
EXE_NAME = rr-fw-update.exe

#
#Enable binary creation
#
MAKE_BINARY=no

#
#Enable binary creation
#
MAKE_EXECUTABLE=yes

#
#Enable shared library creation
#
MAKE_SHARED_LIB=no

#
#Enable static library creation
#
MAKE_STATIC_LIB=no

#
#Enable MAP-file creation
#
CREATE_MAP=no

#
#Tool-chain prefix
#
#TCHAIN = 

#
#CPU specific options
#
#MCPU += -mthumb

#
#C language dialect
#
CDIALECT = gnu99

#
#C++ language dialect
#
CPPDIALECT = c++0x

#
#Optimization
#
OPT_LVL = 2

#
#Additional C flags
#
#CFLAGS += 


#
#Additional CPP flags
#
#CPPCFLAGS += -felide-constructors

#
#Additional linker flags
#
LDFLAGS += -static 


#
#Additional static libraries
#
EXT_LIBS += pthread

#
#Additional non-project object files
#
EXT_OBJECTS += ../build/libservo_api.a


#
#Preprocessor definitions
#
#PPDEFS += 

#
#Include directories
#
INCDIR += .
INCDIR += common
INCDIR += ../src

#
#C sources
#
C_SOURCES += $(wildcard common/*.c)
C_SOURCES += $(wildcard *.c)

#
#Assembler sources
#
#S_SOURCES += 

#
#CPP sources
#
#CPP_SOURCES += 

#
#Linker scripts
#
#LDSCRIPT += 

include core.mk
