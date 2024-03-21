all: libcalc.so clean

libcalc.so:
	ar cr libcalc.a source/*.o

clean: libcalc.so
	rm -rf source/*.o

	