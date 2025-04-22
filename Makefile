CC = gcc
CFLAGS = -Wall -g -fpic -Wno-deprecated-declarations

intel-all: lib/liblwp.so lib64/liblwp.so

lib/liblwp.so: lib liblwp32.o
	$(CC) $(CFLAGS) -m32 -shared -o $@ liblwp32.o

lib64/liblwp.so: lib64 liblwp64.o
	$(CC) $(CFLAGS) -shared -o $@ liblwp64.o

lib:
	mkdir lib

lib64:
	mkdir lib64

liblwp32.o: lwp.c lwp.h
	$(CC) $(CFLAGS) -m32 -c -o liblwp32.o lwp.c lwp.h

liblwp64.o: lwp.c lwp.h
	$(CC) $(CFLAGS) -m64 -c -o liblwp64.o lwp.c lwp.h