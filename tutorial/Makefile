FILES = $(wildcard *.c)
BUILDDIR=build

%.static: %.c 
	gcc $< -g2 -I../include -o $(BUILDDIR)/$@ ../build/libservo_api.a -lpthread

%.run: %.static 
	./$(BUILDDIR)/$<

all: prepare $(FILES:.c=.static)

clean:
	rm -rf build

prepare:
	mkdir -p $(BUILDDIR)

run: $(FILES:.c=.run)
