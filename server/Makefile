CC=gcc
CFLAGS=-I. -pthread
DEPS = constants.h sope.h threadControl.h accountarray.h
OBJ = server.o threadControl.o log.o accountarray.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
