all: main.o compile
	gcc -o calc main.o -L./library -lcalc

compile: 
	$(MAKE) -C library/source
	$(MAKE) -C library

main.o: main.c
	gcc -c main.c

