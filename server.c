#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>
#include "protocol.h"
#include "rb_tree.h"

#define DEFAULT_PORT (7500)
#define MAX_CLIENTS (1010)
#define KEY_SIZE (16)
#define VALUE_SIZE (16)

Tree *tree;

typedef struct status_t {
    int val;
} status_t;

typedef struct map_t {
    status_t * (*set) (struct map_t * map, char * key, char * value);
    char * (*get) (struct map_t * map, char * key);
    Tree *tree;
} map_t;

typedef struct config_t {
    int port;
    map_t map;
} config_t; 

typedef struct client_params_t {
    config_t * config;
    int fd;
    pthread_mutex_t mutex;
} client_params_t;

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

int handle_query(int *rc, int *sock, Tree *T) {
    protocol_t query;
    protocol_t response;
    *rc = recv(*sock, &query, sizeof(query), 0);
    response = query;

    if (*rc < 0) {
        perror("ошибка вызова recv");
        return (EXIT_FAILURE);
    } 
    else {
        switch (query.operation) {
            case OP_ERASE:
                erase(T, query.key);
                response.operation = SUCCESS;
                break;
            case OP_SET:
                insert(T, query.key, query.value);
                response.operation = SUCCESS;          
                break;
            case OP_GET:
                response.value = getValue(T, query.key);
                response.operation = SUCCESS;
                break;
        }
    }

    *rc = send(*sock, &response, sizeof(response), 0);
    if (*rc <= 0) {
        perror("ошибка вызова send");
    }
}

void * client_handler (void * arg) {
    client_params_t * _client_params = arg;
    client_params_t client_params = *_client_params;
    pthread_mutex_unlock (&_client_params->mutex);

    puts("Handling");
    ///////////////////////////////

    int32_t n;
    //getting number of operations;
    int rc = recv(client_params.fd, &n, sizeof (n), 0);

    if (rc <= 0) {
        printf("Failed\n");
        return (EXIT_FAILURE);
    }
    else {
        printf("%d\n", n);
    }

    rc = send(client_params.fd, "0", 1, 0);
    if (rc <= 0) {
        perror("ошибка вызова send");
    }  

    int i;
    for (i = 0; i < n; i++) {
        handle_query(&rc, &client_params.fd, tree);
    }

    printTree(tree->root, 3);
}




int run_server(config_t *config) {
    struct sockaddr_in local;
    int rc;
    char buf[1];
    local.sin_family = AF_INET;
    // local.sin_port = htons (config->port);
    local.sin_port = htons (DEFAULT_PORT);
    local.sin_addr.s_addr = htonl (INADDR_ANY);

    map_t *map = &config->map;

    int sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror ("ошибка вызова socket");
        return (EXIT_FAILURE);
    }

    rc = bind(sock, (struct sockaddr *)&local, sizeof(local));
    if (rc < 0) {
        perror("ошибка вызова bind");
        return (EXIT_FAILURE);
    }

    if (rc = listen(sock, MAX_CLIENTS)) {
        perror("ошибка вызова listen");
        return (EXIT_FAILURE);
    }

    client_params_t client_params;
    pthread_mutex_init (&client_params.mutex, NULL);
    pthread_mutex_lock (&client_params.mutex);

    for (;;){
        int fd = accept(sock, NULL, NULL);
        puts("Accepted");

        if (fd < 0) {
            perror("ошибка вызова accept");
            break;
        }

        pthread_t thread;
        client_params.fd = fd;
        int rv = pthread_create (&thread, NULL, client_handler, &client_params);
        puts("Created");
        if (0 == rv)
        pthread_mutex_lock (&client_params.mutex);
    }

    printTree(tree->root, 3);
    deleteTree(tree);
}

map_t map_init(){
    tree = createTree();
    map_t map;
    map.tree = createTree();
    return map;
}

int parse_config(config_t *config, int argc, char **argv){
    return 0;
}

int main (int argc, char * argv[]) {
    tree_map_t tree_map;
    tree_map.map = map_init ();

    config_t config = {
        .port = DEFAULT_PORT,
        .map = tree_map.map,
    };

    //парсинг аргументов из вызова программы
    int rv = parse_config (&config, argc, argv);
    if (rv != EXIT_SUCCESS)
        return (rv);

    rv = run_server (&config);

    if (rv != 0){
        return (rv);
    }

    return (EXIT_SUCCESS);
}

