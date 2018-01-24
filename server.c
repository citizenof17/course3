#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include "protocol.h"
#include "rb_tree.h"

#define DEFAULT_PORT (7500)
typedef enum {
  OP_ERASE,
  OP_SET,
  OP_GET,
} operation_t;

typedef struct protocol_t {
  operation_t operation;
  char key[KEY_SIZE];
  char value[VALUE_SIZE];
} protocol_t;

void handleQuery(int *rc, int *s1, struct protocol *query, Tree *T) {
    *rc = recv(*s1, query, sizeof (*query), 0);
    struct protocol response = {-1, -1, -1};

    if (*rc < 0) {
        perror("ошибка вызова recv");
        exit(1);
    } 
    else {
      switch (query->operation)
	{
	case ERASE:
	  erase(T, query->key);
	  response.operation = 10;
	  break;
	}
      
        if (query->operation == 1) { //insert
            insert(T, query->key, query->value); //NEED TO MODIFY
            response.operation = 10;
        }
        else if (query->operation == 2) { //erase
            erase(T, query->key);
            response.operation = 10;
        }
        else if (query->operation == 3) { //contains
            response.operation = 10;
            response.key = contains(T, T->root, query->key);
        }
        else if (query->operation == 4) { //getValue
            response.operation = 10;
            response.key = getValue(T, T->root, query->key);
        }
    }

    *rc = send(*s1, (void*)&response, sizeof(struct protocol), 0);
    if (*rc <= 0) {
        perror("ошибка вызова send");
    }    
}

typedef struct client_params_t {
  config_t * config;
  int fd;
  pthread_mutex_t mutex;
} client_params_t;

void * client_handler (void * arg) {
  client_params_t * _client_params = arg;
  client_params_t client_params = *_client_params;
  pthread_mutex_unlock (_client_params.mutex);

  ///////////////////////////////

  int32_t n;
  //getting number of operations;
  rc = recv(s1, &n, sizeof (n), 0);

  if (rc <= 0) {
    printf("Failed\n");
    exit(1);
  }
  else {
    printf("%d\n", n);
  }

  rc = send(s1, "0", 1, 0);
  if (rc <= 0) {
    perror("ошибка вызова send");
  }  

  Tree *tr = createTree();

  int i;
  for (i = 0; i < n; i++) {
    struct protocol query;
    handleQuery(&rc, &s1, &query, tr);
  }

  //resulting tree;
  printTree(tr->root, 3);
  clearTree(tr, tr->root);

  close(s);
}

typedef struct map_t {
  status_t (*set) (map_t * map, char * key, char * value);
  chat * (*get) (map_t * map, chat * key);
} map_t;

typedef struct tree_map_t {
  map_t map;
  void * root;
} tree_map_t;

typedef struct hash_map_t {
  map_t map;
  void * table;
  int size;
} hash_map_t;

tree_map_t * tree_map_init () {

}

int main (int argc, char * argv) {
  config_t config = {
    .port = DEFAULT_PORT,
  };
  map_t map;

  map_init ();

  int rv = parse_config (&config, argc, argv);
  if (rv != EXIT_SUCCESS)
    return (rv);
  rv = run_server (&config);
  return (rv);
    
    struct sockaddr_in local;
    int s;
    int s1;
    int rc;
    char buf[1];
    local.sin_family = AF_INET;
    local.sin_port = htons (DEFAULT_PORT);
    local.sin_addr.s_addr = htonl (INADDR_ANY);

    int sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror ("ошибка вызова socket");
        return (EXIT_FAILURE)
    }

    rc = bind(s, (struct sockaddr *)&local, sizeof(local));
    if (rc < 0) {
        perror("ошибка вызова bind");
        exit(1);
    }

    if (rc = listen(s, 5)) {
        perror("ошибка вызова listen");
        exit(1);
            }

    client_params_t client_params;
    pthread_mutex_init (&client_params.mutex, NULL);
    pthread_mutex_lock (&client_params.mutex);
    
    for (;;){
      int fd = accept(s, NULL, NULL);
      if (s1 < 0) {
        perror("ошибка вызова accept");
        break;
      }
      int rv = pthread_create (&id, NULL, client_handler, &client_params);
      if (0 == rv)
	pthread_mutex_lock (&client_params.mutex);
    }


    close(s1);
    exit(0);
}
