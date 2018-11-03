FILES = $(wildcard *.c)
BUILDDIR=build

ifneq (,$(findstring mingw,$(TCHAIN)))
	EXT=.exe
endif

API_LIB=../build/libservo_api.a

$(BUILDDIR)/%$(EXT): %.c $(API_LIB)
	$(TCHAIN)gcc $< -g2 -I../include -o $@ $(API_LIB) -static -lpthread
	$(TCHAIN)strip $@

all: prepare $(addprefix $(BUILDDIR)/,$(FILES:.c=$(EXT)))

clean:
	rm -rf build

prepare:
	mkdir -p $(BUILDDIR)


