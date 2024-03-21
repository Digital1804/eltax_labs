all: libcalc.so clean

libcalc.so:
	gcc -shared -o libcalc.so source/*.o

clean: libcalc.so
	rm -rf source/*.o

	