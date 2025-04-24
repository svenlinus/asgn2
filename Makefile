CC = gcc
CFLAGS = -Wall -g -Wno-deprecated-declarations

all: test
	./test

test: test.o magic64.o lwp.o scheduler.o
	$(CC) $(CFLAGS) -o test test.o magic64.o lwp.o scheduler.o

test.o: test.c
	$(CC) $(CFLAGS) -c test.c

lwp.o: lwp.c
	$(CC) $(CFLAGS) -c lwp.c
	
scheduler.o: scheduler.c
	$(CC) $(CFLAGS) -c scheduler.c

magic64.o: magic64.S
	$(CC) -o magic64.o -c magic64.S

clean:
	rm *.o

liblwp.so: liblwp.o
	$(CC) $(CFLAGS) -shared -o $@ liblwp.o
	
liblwp.o: lwp.c lwp.h scheduler.c scheduler.h
	$(CC) $(CFLAGS) -c -o liblwp.o lwp.c lwp.h scheduler.c scheduler.h