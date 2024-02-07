all: sample main

sample:	sample.cpp
	g++ $(DEBUG) -I../src -c -fPIC sample.cpp -o sample.o
	g++ $(DEBUG) -I../src -shared -Wl,-soname,sample.so -o sample.so sample.o
	test -d ../debug && cp *.so ../debug
	test -d ../build && cp *.so ../build

debug: DEBUG=-g

debug: sample main

main:		main.cpp
	g++ $(DEBUG) main.cpp -o main -ldl -I../src

test: sample main
	LD_LIBRARY_PATH=. ./main

clean:
	rm *.o
	rm *.so
	rm *.gch

