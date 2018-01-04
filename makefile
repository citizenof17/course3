CC=gcc

FLAGS=-c -std=c99

all: server client

server: rb_tree.o server.o 
	gcc -o server server.o rb_tree.o 

client: rb_tree.o client.o 
	gcc -o client client.o rb_tree.o 	

rb_tree.o: rb_tree.c
	gcc -c rb_tree.c

server.o: server.c
	gcc -c server.c

client.o: client.c
	gcc -c client.c

clear:
	rm -vf *.o