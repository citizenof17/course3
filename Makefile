all: server client
server: rb_tree.o server.o 
	gcc -o server rb_tree.o server.o -pthread
client: rb_tree.o client.o operations.o
rb_tree.o: rb_tree.c
server.o: server.c
client.o: client.c
operations.o: operations.c

# clear:
# 	${RM} *.o
