OS:=$(strip$(OS))

APP_NAME=rr-cfg-update

ifeq ($(OS),win32)
	SHARED_LIB_EXT=dll
	STATIC_LIB_EXT=a
	EXE_EXT=exe
	BUILDDIR = build-win-32bit
	EXE_NAME = $(APP_NAME)-32bit
	TCHAIN=i686-w64-mingw32-
	LDFLAGS += -static -Wl,-subsystem,console -lws2_32
	EXT_OBJECTS += ../../build-win-32bit/libservo_api-32bit.a
else ifeq ($(OS),win64)
	SHARED_LIB_EXT=dll
	STATIC_LIB_EXT=a
	EXE_EXT=exe
	BUILDDIR = build-win-64bit
	EXE_NAME = $(APP_NAME)-64bit
	TCHAIN=x86_64-w64-mingw32-
	LDFLAGS += -static -Wl,-subsystem,console -lws2_32
	EXT_OBJECTS += ../../build-win-64bit/libservo_api-64bit.a
else
	SHARED_LIB_EXT=
	STATIC_LIB_EXT=
	EXE_EXT=
	BUILDDIR = build
	EXE_NAME = $(APP_NAME)
	EXT_OBJECTS += ../../build/libservo_api.a
endif

#
#Verbose mode
#
VERBOSE=no

#
#Colorize ouput
#
COLORIZE=no

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
#Preprocessor definitions
#
#PPDEFS += 

#
#Include directories
#
INCDIR += .
INCDIR += ../common
INCDIR += ../../src

#
#C sources
#
SRC_PATH=../common
C_SOURCES += $(notdir $(wildcard ../common/*.c))
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

include ../../core.mk
