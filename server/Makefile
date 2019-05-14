CC=gcc
CFLAGS=-I. -pthread -Wall
DEPS = constants.h sope.h threadControl.h
OBJ = server.o threadControl.o log.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)