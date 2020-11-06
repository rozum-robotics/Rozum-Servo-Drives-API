FILES = $(wildcard *.c)

OS:=$(strip$(OS))

ifeq ($(OS),win32)
	EXE_EXT=.exe
	BUILDDIR = build-win-32bit
	TCHAIN=i686-w64-mingw32-
	LDFLAGS += -static -lpthread -lm -lws2_32
	EXT_OBJECTS += ../build-win-32bit/libservo_api-32bit.a
else ifeq ($(OS),win64)
	EXE_EXT=.exe
	BUILDDIR = build-win-64bit
	TCHAIN=x86_64-w64-mingw32-
	LDFLAGS += -static -lpthread -lm -lws2_32
	EXT_OBJECTS += ../build-win-64bit/libservo_api-64bit.a
else
	EXE_EXT=
	BUILDDIR = build
	LDFLAGS += -static -lpthread -lm
	EXT_OBJECTS += ../build/libservo_api.a
endif


$(BUILDDIR)/%$(EXE_EXT): %.c $(API_LIB) $(EXT_OBJECTS)
	$(TCHAIN)gcc $< -g2 -I../include -o $@ $(EXT_OBJECTS) $(LDFLAGS)

all: prepare $(addprefix $(BUILDDIR)/,$(FILES:.c=$(EXE_EXT))) 

clean:
	rm -rf $(BUILDDIR)

prepare:
	make -C ..
	mkdir -p $(BUILDDIR)


