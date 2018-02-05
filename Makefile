all: server client
server: tree_map.o tree_hash_map.o rb_tree.o server.o 
	gcc -o server rb_tree.o tree_map.o tree_hash_map.o server.o -pthread 
client: client.o operations.o 
	gcc -o client client.o operations.o -pthread
rb_tree.o: rb_tree.c
server.o: server.c
client.o: client.c
operations.o: operations.c
tree_map.o: tree_map.c
tree_hash_map.o: tree_hash_map.c

clear:
	rm -v *.o

# clear:
# 	${RM} *.o
