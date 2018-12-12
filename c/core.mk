CC=$(TCHAIN)gcc
CPP=$(TCHAIN)g++
ifeq ($(strip $(CPP_SOURCES)),)
	LD=$(TCHAIN)gcc
else
	LD=$(TCHAIN)g++
endif
OC=$(TCHAIN)objcopy
AR=$(TCHAIN)ar
RL=$(TCHAIN)ranlib

ifeq ($(strip $(COLORIZE)),yes)
	CLRED=\e[31m
	CLGRN=\e[32m
	CLYEL=\e[33m
	CLRST=\e[0m
	VECHO_=printf
else
	CLRED=
	CLGRN=
	CLYEL=
	CLRST=
	VECHO_=printf
endif

ifneq ($(VERBOSE),yes)
	Q:=@
	VECHO=@$(VECHO_)
else
	Q:= 
	VECHO=@true
endif

ifeq ($(SHARED_LIB_EXT),)
	SHARED_LIB_EXT=.so
else
	SHARED_LIB_EXT:=.$(SHARED_LIB_EXT)
endif

ifeq ($(STATIC_LIB_EXT),)
	STATIC_LIB_EXT=.a
else
	STATIC_LIB_EXT:=.$(STATIC_LIB_EXT)
endif

ifneq ($(EXE_EXT),)
	EXE_EXT:=.$(EXE_EXT)
endif

FLAGS=-c -Wall -O$(OPT_LVL)
FLAGS+=-gdwarf-2
FLAGS+=-MMD -MP 
FLAGS+=$(MCPU) $(addprefix -I,$(INCDIR)) $(addprefix -D,$(PPDEFS)) 

ifneq ($(CDIALECT),)
CFLAGS+=-std=$(CDIALECT)
endif

ifneq ($(CPPDIALECT),)
CPPFLAGS+=-std=$(CPPDIALECT)
endif

LDFLAGS+=$(MCPU) 
ifeq ($(strip $(CREATE_MAP)),yes)
LDFLAGS+=-Wl,-Map=$(BUILDDIR)/map.map
endif
LDFLAGS+=$(addprefix -T,$(LDSCRIPT))

vpath %.c $(SRC_PATH)
vpath %.s $(SRC_PATH)
vpath %.cpp $(SRC_PATH)

EXECUTABLE=$(BUILDDIR)/$(EXE_NAME)$(EXE_EXT)
BINARY=$(BUILDDIR)/$(basename $(EXE_NAME)).bin
SHARED_LIB=$(BUILDDIR)/$(basename lib$(EXE_NAME))$(SHARED_LIB_EXT)
STATIC_LIB=$(BUILDDIR)/$(basename lib$(EXE_NAME))$(STATIC_LIB_EXT)

CPP_OBJECTS=$(addprefix $(BUILDDIR)/, $(CPP_SOURCES:.cpp=.o))
C_OBJECTS=$(addprefix $(BUILDDIR)/, $(C_SOURCES:.c=.o))
S_OBJECTS=$(addprefix $(BUILDDIR)/, $(S_SOURCES:.s=.o))

LINK_OBJECTS=$(S_OBJECTS) $(C_OBJECTS) $(CPP_OBJECTS)
BUILDTREE=$(BUILDDIR) $(dir $(LINK_OBJECTS))


ifeq ($(strip $(MAKE_BINARY)),yes)
	ARTEFACTS+=$(BINARY)
endif
ifeq ($(strip $(MAKE_EXECUTABLE)),yes)
	ARTEFACTS+=$(EXECUTABLE)
endif

ifeq ($(strip $(MAKE_SHARED_LIB)),yes)
	ARTEFACTS+=$(SHARED_LIB)
	CFLAGS+=-fpic
endif

ifeq ($(strip $(MAKE_STATIC_LIB)),yes)
	ARTEFACTS+=$(STATIC_LIB)
endif

all: prepare $(CPP_SOURCES) $(C_SOURCES) $(S_SOURCES) $(ARTEFACTS)

prepare:
	@mkdir -p $(BUILDTREE)


$(SHARED_LIB): $(LINK_OBJECTS)
	$(VECHO)  ' [$(CLRED)L$(CLRST)]   $(CLRED)$@$(CLRST) ...\n'
	$(Q)$(LD) -shared -o $@  $(LDFLAGS) $(LINK_OBJECTS) $(EXT_OBJECTS) $(addprefix -L,$(LIBDIR)) $(addprefix -l,$(EXT_LIBS)) 

$(STATIC_LIB): $(LINK_OBJECTS)
	$(VECHO)  ' [$(CLRED)AR$(CLRST)]  $(CLRED)$@$(CLRST) ...\n'
	$(Q)$(AR) -rc $@  $(LINK_OBJECTS) $(EXT_OBJECTS) 
	$(VECHO)  ' [$(CLRED)RL$(CLRST)]  $(CLRED)$@$(CLRST) ...\n'
	$(Q)$(RL) $@ 

$(EXECUTABLE): $(LINK_OBJECTS) 
	$(VECHO)  ' [$(CLRED)L$(CLRST)]   $(CLRED)$@$(CLRST) ...\n'
	$(Q)$(LD) -o $@  $(LDFLAGS) $(LINK_OBJECTS) $(EXT_OBJECTS) $(addprefix -L,$(LIBDIR)) $(addprefix -l,$(EXT_LIBS)) 

$(BINARY) : $(EXECUTABLE)
	$(VECHO) ' [$(CLRED)B$(CLRST)]   $(CLRED)$@$(CLRST) ...\n'
	$(Q)$(OC) -O binary $< $(@)

$(BUILDDIR)/%.o: %.s
	$(VECHO) ' [$(CLGRN)S$(CLRST)] $< ...\n'
	$(Q)$(CC) $(FLAGS) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.c
	$(VECHO) ' [$(CLGRN)C$(CLRST)]   $< ...\n'
	$(Q)$(CC) $(FLAGS) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.cpp
	$(VECHO) ' [$(CLYEL)C++$(CLRST)] $< ...\n'
	$(Q)$(CPP) $(FLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -fr $(BUILDDIR)

-include $(C_OBJECTS:.o=.d)
-include $(CPP_OBJECTS:.o=.d)



