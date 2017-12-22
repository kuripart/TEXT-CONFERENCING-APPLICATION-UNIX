all: client server

client: client.o
	gcc -o client client.o

server: server.o
	gcc -o server server.o

client.o: client.c
	gcc -Wall -g -c -o client.o client.c

server.o: server.c
	gcc -Wall -g -c -o server.o server.c

clean:
	rm -f client server client.o server.o
