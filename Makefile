all: server user

server: server.c log.c types.h sope.h accountarray.h
	gcc server.c log.c accountarray.c -o server -lpthread -Wall -Wextra -pedantic -g

user: user.c log.c types.h sope.h
	gcc user.c log.c -o user -lpthread -Wall -Wextra -pedantic -g

clean:
	rm -f server server.o
	rm -f user user.o	
	