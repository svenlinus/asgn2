CC = gcc
CFLAGS = -Wall -g -Wno-deprecated-declarations

liblwp.so: liblwp.o
	$(CC) $(CFLAGS) -shared -o $@ liblwp.o

liblwp.o: lwp.c lwp.h scheduler.c scheduler.h
	$(CC) $(CFLAGS) -c -o liblwp.o lwp.c lwp.h scheduler.c scheduler.h