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
#include "hash_map.h"
#include "tree_hash_map.h"

#define MAX_CLIENTS (1010)
#define DEFAULT_PORT (7500)
#define BAD_PORT (2)
#define MAX_PORT (65535)
#define DEFAULT_SIZE (256)
#define DEFAULT_MULTIPLIER (2)
#define TREE_HASH_MAP_SIZE (1024) 
#define THREADS_NUM (4)
// #define N (10000000)         // 1e6
#define N_ops (10000000)    // 1e7
#define SET_OPERATION_PROB (20)
#define REPEATS (10)
#define STR_SIZE (10)

int N;

typedef struct key_value_ {
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
} key_value_t;

// key_value_t STORAGE[N];
key_value_t *STORAGE;
// Debug output
char *OPERS[] = {"Erase", "Set", "Get", "Success", "Fail"};

FILE *fout;

typedef struct config_t {
    int port;
    map_t map;
} config_t; 

typedef struct client_params_t {
    config_t *config;
    int fd;
    pthread_mutex_t mutex;
} client_params_t;

int handle_query(map_t *map, command_t query) {
    // printf("operation: %s, key: %s, value: %s\n", OPERS[query.operation], query.key, query.value);
    switch (query.operation) {
        case OP_SET:
            map->set(map, query.key, query.value);
            break;
        case OP_GET:
            map->get(map, query.key);
            break;
    }
}

void * client_handler(void * arg) {
    client_params_t * _client_params = arg;
    client_params_t client_params = *_client_params;
    pthread_mutex_unlock (&_client_params->mutex);

    // printf("Thread number %d started working\n", client_params.fd);
    int i;
    for (i = 0; i < N_ops; i++) {
        command_t query;
        query.operation = OP_GET;

        // 20% for OP_SET
        if (rand() % 100 < SET_OPERATION_PROB){
            query.operation = OP_SET;
        }

        int ind = rand() % N;
        strcpy(query.key, STORAGE[ind].key);
        strcpy(query.value, STORAGE[ind].value);
        handle_query(&client_params.config->map, query);
    }
    // printf("Thread number %d ended working\n", client_params.fd);

    // hash_map_print(&client_params.config->map);
    // tree_map_print(&client_params.config->map);
    // tree_hash_map_print(&client_params.config->map);

}

char *gen_str(int size){
    char *res = malloc(sizeof(char) * size + 1);
    for(int i = 0; i < size; i++){
        res[i] = (char)(rand() % 26 + 'a');
    }
    res[size] = '\0';
    return res;
}

int run_server(config_t *config, double *timer) {
    map_t *map = &config->map;
    srand(time(NULL));

    STORAGE = (key_value_t*)malloc(sizeof(key_value_t) * N); 
    // storage initialization
    int i;
    for(i = 0; i < N; i++){
        char *key = gen_str(STR_SIZE);
        char *value = gen_str(STR_SIZE);

        strcpy(STORAGE[i].key, key);
        strcpy(STORAGE[i].value, value);
        free(key);
        free(value);
        command_t query;
        query.operation = OP_SET;
        strcpy(query.key, STORAGE[i].key);
        strcpy(query.value, STORAGE[i].value);
        handle_query(map, query);
    }
    // tree_map_print(map);
    // tree_hash_map_print(map);
    // hash_map_print(map);
    // list_hash_map_print(map);

    clock_t ti1 = clock();

    client_params_t client_params;
    pthread_mutex_init (&client_params.mutex, NULL);
    pthread_mutex_lock (&client_params.mutex);
    client_params.config = config;

    pthread_t threads[THREADS_NUM];
    for (i = 0; i < THREADS_NUM; i++){
        client_params.fd = i;
        int rv = pthread_create (&threads[i], NULL, client_handler, &client_params);
        if (rv == 0){
            pthread_mutex_lock (&client_params.mutex);
        }
    }

    for(i = 0; i < THREADS_NUM; i++){
        pthread_join(threads[i], NULL);
    }

    clock_t ti2 = clock();
    double cur_time = (double)(ti2 - ti1) / CLOCKS_PER_SEC;
    *timer = cur_time;
    fprintf(fout, "TIME: %f\n", cur_time);
    printf("TIME: %f\n", cur_time);
    free(STORAGE);
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

    fout = fopen("output.txt", "a");
    for (int mn = 1; mn <= 10; mn++){
        N = (int)1e6 * mn;
        double res_time = 0;
        fprintf(fout, "Size: %d\n", N);
        printf("Size: %d\n", N);   
        for (int t = 0; t < REPEATS; t++){
            map_t map;
            // tree_map_init(&map);
            // tree_hash_map_init(&map, TREE_HASH_MAP_SIZE);
            hash_map_init(&map, DEFAULT_SIZE, DEFAULT_MULTIPLIER);
            // list_hash_map_init(&map, DEFAULT_SIZE, DEFAULT_MULTIPLIER);
            config_t config = {
                .map = map,
            };

            int rv;
            double cur_time;
            rv = run_server (&config, &cur_time);
            res_time += cur_time;

            // tree_map_free(&config.map);
            // tree_hash_map_free(&config.map);
            hash_map_free(&config.map);
            // list_hash_map_free(&config.map);

            fflush(fout);
        }
        fprintf(fout, "AVERAGE TIME: %f\n", res_time / REPEATS);
        printf("AVERAGE TIME: %f\n", res_time / REPEATS);
        //ADD AVERAGE TIME HERE
        fflush(fout);
    }
    fclose(fout);
    return (EXIT_SUCCESS);
}

