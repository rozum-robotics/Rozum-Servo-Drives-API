all:
	gcc test.c -I../include -o test.shared -L../build -lservo_api -lpthread -Wl,-rpath=../build -Wl,-rpath=. -no-pie
	gcc test.c -I../include -o test.static ../build/libservo_api.a -lpthread -no-pie
