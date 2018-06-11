all:
	gcc test.c -I../include -o test.shared -L../build -lservo_api -Wl,-rpath=../build -Wl,-rpath=. -no-pie
	gcc -static test.c -I../include -o test.static -L../build -lservo_api -no-pie
