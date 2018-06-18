FILES = $(wildcard *.c)

%.static: %.c 
	gcc $< -g2 -I../include -o $@ ../build/libservo_api.a -lpthread -no-pie

%.run: %.static 
	./$<

run: all $(FILES:.c=.run)

clean:
	rm -rf *.static

all: $(FILES:.c=.static)