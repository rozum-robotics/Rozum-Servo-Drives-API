all:
	mkdir -p build
	gcc test.c -g2 -I../include -o build/test.shared -L../build -lservo_api -lpthread -Wl,-rpath=. -Wl,-rpath=../build -lm
	gcc test.c -g2 -I../include -o build/test.static ../build/libservo_api.a -lpthread -lm
	gcc test1.c -g2 -I../include -o build/test1.static ../build/libservo_api.a -lpthread -lm
	#gcc pvt1.c -g2 -I../include -o build/pvt1 ../build/libservo_api.a -lpthread -lm
clean:
	rm -fr build
run:
	./build/pvt1
