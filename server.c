#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include "protocol.h"
#include "rb_tree.h"
#include "map_interface.h"
#include "tree_map.h"
#include "hash_map2.h"
#include "tree_hash_map.h"

#define MAX_CLIENTS (1010)
#define DEFAULT_PORT (7500)
#define BAD_PORT (2)
#define MAX_PORT (65535)
#define DEFAULT_SIZE (10)
#define DEFAULT_MULTIPLIER (2)
#define TREE_HASH_MAP_SIZE (10)  // debugging value. Must be changed to 1024
#define IP_ADDR ("2.92.55.120")

// Debug output
char *OPERS[] = {"Erase", "Set", "Get", "Success", "Fail"};

typedef struct status_t {
    int val;
} status_t;

typedef struct config_t {
    int port;
    map_t map;
} config_t; 

typedef struct client_params_t {
    config_t *config;
    int fd;
    pthread_mutex_t mutex;
} client_params_t;

int handle_query(int *rc, int *sock, map_t *map) {
    command_t query;
    response_t response;
    *rc = recv(*sock, &query, sizeof(query), 0);

    printf("operation: %s, key: %s, value: %s\n", OPERS[query.operation], query.key, query.value);
    if (*rc < 0) {
        perror("ошибка вызова recv");
        return (EXIT_FAILURE);
    } 
    else {
        switch (query.operation) {
            case OP_ERASE:
                response = map->rem(map, query.key);
                break;
            case OP_SET:
                response = map->set(map, query.key, query.value);
                break;
            case OP_GET:
                response = map->get(map, query.key);
                break;
        }
    }

    *rc = send(*sock, &response, sizeof(response), 0);
    if (*rc <= 0) {
        perror("ошибка вызова send");
    }
}

void * client_handler(void * arg) {
    client_params_t * _client_params = arg;
    client_params_t client_params = *_client_params;
    pthread_mutex_unlock (&_client_params->mutex);

    int32_t n;  //number of operations
    int rc = recv(client_params.fd, &n, sizeof (n), 0);
    printf("RECV %d\n", rc);
    if (rc <= 0) {
        printf("Failed\n");
        return ((void *)EXIT_FAILURE);
    }
    else {
        printf("Handling %d operations\n", n);
    }

    rc = send(client_params.fd, "0", 1, 0);
    if (rc <= 0) {
        perror("ошибка вызова send");
    }  

    int i;
    for (i = 0; i < n; i++) {
        handle_query(&rc, &client_params.fd, &client_params.config->map);
    }

    // hash_map_print(&client_params.config->map);
    tree_map_print(&client_params.config->map);
    // tree_hash_map_print(&client_params.config->map);
}

int run_server(config_t *config) {
    printf("Running\n");
    struct sockaddr_in local;
    int rc;
    char buf[1];
    local.sin_family = AF_INET;
    local.sin_port = htons(config->port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    map_t *map = &config->map;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
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
    client_params.config = config;

    for (;;){
        struct sockaddr_in client_name;
        socklen_t client_name_len;
        int fd = accept (sock, &client_name, &client_name_len);
        // int fd = accept(sock, NULL, NULL);

        if (fd < 0) {
            perror("ошибка вызова accept");
            break;
        }

        pthread_t thread;
        client_params.fd = fd;
        int rv = pthread_create (&thread, NULL, client_handler, &client_params);
        if (0 == rv){
            pthread_mutex_lock (&client_params.mutex);
        }
    }
}

int parse_str(int *num, char *str){
    *num = 0;

    int len = strlen(str);
    int i;
    for(i = 0; i < len; i++){
        if (isdigit(str[i])){
            *num *= 10;
            *num += str[i] - '0';
        }
        else{
            return BAD_PORT;
        }
        if (*num > MAX_PORT){
            *num = 0;
            return BAD_PORT;
        }
    }
    return 0;
}

int parse_config(config_t *config, int argc, char **argv){

    if (argc > 1){
        int rv = parse_str(&config->port, argv[1]);
        if (rv != 0){
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

int main (int argc, char * argv[]) {
    map_t map;
    // hash_map_init(&map, DEFAULT_SIZE, DEFAULT_MULTIPLIER);
    tree_map_init(&map);
    // tree_hash_map_init(&map, TREE_HASH_MAP_SIZE);

    config_t config = {
        .port = DEFAULT_PORT,
        .map = map,
    };

    //парсинг аргументов из вызова программы
    int rv = parse_config (&config, argc, argv);
    if (rv != EXIT_SUCCESS)
        return (rv);

    rv = run_server (&config);

    if (rv != 0){
        printf("Fail");
        return (rv);
    }

    return (EXIT_SUCCESS);
}

