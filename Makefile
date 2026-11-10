CC = gcc
CFLAGS = -Wall -g

memsimulator: memsim.o pra.o
	$(CC) $(CFLAGS) -o memsimulator memsim.o pra.o

memsim.o: memsim.c pra.h
	$(CC) $(CFLAGS) -c memsim.c

pra.o: pra.c pra.h
	$(CC) $(CFLAGS) -c pra.c

clean:
	rm -f *.o memsimulator
