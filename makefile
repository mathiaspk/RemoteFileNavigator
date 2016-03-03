
CFLAGS = -Wall -g

all: client server

client: client.c 
	gcc $(CFLAGS) -c app_proto.c
	gcc $(CFLAGS) -o client app_proto.o client.c

server: server.c
	gcc $(CFLAGS) -o server app_proto.o server.c

debug: 
	gcc $(CFLAGS) -c app_proto.c -DDEBUG
	gcc $(CFLAGS) -o server app_proto.o server.c -DDEBUG
	gcc $(CFLAGS) -o client app_proto.o client.c -DDEBUG

clean:
	rm -f *~ client server app_proto
	