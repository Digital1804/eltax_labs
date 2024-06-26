all: main.o compile
	gcc -o files main.o -L./library -lcalc -Wl,-rpath,./library -lncurses

compile: 
	$(MAKE) -C library/source
	$(MAKE) -C library

main.o: main.c
	gcc -c main.c

