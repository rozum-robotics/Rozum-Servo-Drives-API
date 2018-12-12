OS:=$(strip$(OS))

APP_NAME=servo_api

ifeq ($(OS),win32)
	SHARED_LIB_EXT=dll
	STATIC_LIB_EXT=a
	EXE_EXT=exe
	BUILDDIR = build-win-32bit
	EXE_NAME = $(APP_NAME)-32bit
	TCHAIN=i686-w64-mingw32-
	LDFLAGS += -static
else ifeq ($(OS),win64)
	SHARED_LIB_EXT=dll
	STATIC_LIB_EXT=a
	EXE_EXT=exe
	BUILDDIR = build-win-64bit
	EXE_NAME = $(APP_NAME)-64bit
	TCHAIN=x86_64-w64-mingw32-
	LDFLAGS += -static
else
	SHARED_LIB_EXT=
	STATIC_LIB_EXT=
	EXE_EXT=
	BUILDDIR = build
	EXE_NAME = $(APP_NAME)
	PPDEFS += COLOR_TERM
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
MAKE_EXECUTABLE=no

#
#Enable shared library creation
#
MAKE_SHARED_LIB=yes

#
#Enable static library creation
#
MAKE_STATIC_LIB=yes

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
#MCPU += -m32

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
#Additional static libraries
#
EXT_LIBS += pthread

#
#Additional non-project object files
#


#
#Preprocessor definitions
#
#PPDEFS += 



#
#Include directories
#
INCDIR += .
INCDIR += include
INCDIR += src

#
#C sources
#
C_SOURCES += $(wildcard src/*.c)

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
