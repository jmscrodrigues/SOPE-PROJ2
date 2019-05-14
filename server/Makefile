CC=gcc
<<<<<<< HEAD
CFLAGS=-I. -pthread
DEPS = constants.h sope.h threadControl.h accountarray.h
OBJ = server.o threadControl.o log.o accountarray.o
=======
CFLAGS=-I. -pthread -Wall
DEPS = constants.h sope.h threadControl.h
OBJ = server.o threadControl.o log.o
>>>>>>> user_dev

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
