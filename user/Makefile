CC=gcc
CFLAGS=-I. -pthread
DEPS = constants.h sope.h 
OBJ = user.o log.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

user: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)