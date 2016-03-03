
all: client server protocol 

client: client.c 
	gcc -c interface.c
	gcc -o client interface.o client.c

server: server.c
	gcc -o server server.c

protocol: protocol.c
	gcc -o protocol protocol.c

clean:
	rm -f *~ client server protocol interface
