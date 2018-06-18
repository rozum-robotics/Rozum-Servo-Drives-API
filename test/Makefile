all:
	mkdir -p build
	gcc test.c -g2 -I../include -o build/test.shared -L../build -lservo_api -lpthread -Wl,-rpath=. -Wl,-rpath=../build -no-pie
	gcc test.c -g2 -I../include -o build/test.static ../build/libservo_api.a -lpthread -no-pie
clean:
	rm -fr build
run:
	./build/test.shared
