#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include "protocol.h"
#include "operations.h"

#define DEFAULT_PORT (7500)
#define DEFAULT_PORT (7500)
#define BAD_PORT (2)
#define MAX_PORT (65535)

int glob = 5;

typedef struct config_t {
    int port;
} config_t;

typedef struct client_params_t {
    // struct sockaddr_in *peer;
    int id;
    config_t *config;
} client_params_t;

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

char *gen_str(){
    int size = 5;
    char *res = malloc(sizeof(char) * size + 1);
    for(int i = 0; i < size; i++){
        res[i] = (char)(rand() % 26 + 'a');
    }
    res[size] = '\0';
    return res;
}

void * run_client(void * arg){
    client_params_t client_params = *(client_params_t *)arg;

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(client_params.config->port);
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sock;
    int rc;
    char buf[1];
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ошибка вызова socket");
        return ((void *)EXIT_FAILURE);
    }
    
    if ((rc = connect(sock, (struct sockaddr *)&peer, sizeof(peer))) > 0) {
        perror("ошибка вызова connect");
        return ((void *)EXIT_FAILURE);        
    }
    printf("Connected %d\n", client_params.id);

    int n = rand() % 15;

    if ((rc = send(sock, &n, sizeof(n), 0)) <= 0) {
        perror("ошибка вызова send");
        return ((void *)EXIT_FAILURE);
    }

    if ((rc = recv(sock, buf, 1, 0)) <= 0) {
        perror("ошибка вызова recv");
        return ((void *)EXIT_FAILURE);
    }

    //////////////////////

    printf("Client %d is sending %d operations\n", client_params.id, n);

    response_t response;

    int i, res;
    for (i = 0; i < n; i++) {
        char *key = gen_str();
        char *value = gen_str();
        int oper = rand() % 3;
        
        switch(oper) {
            case 0:
                res = op_erase(&sock, &rc, key, &response);
                printf("Rem %s\n", response.answer);
                break;
            case 1:
                res = op_set(&sock, &rc, key, value, &response);
                printf("Set %s\n", response.answer);
                break;
            case 2:
                res = op_get(&sock, &rc, key, &response);
                printf("Get %s -> %s\n", response.answer, response.value);
                break;
        }

        free(key);
        free(value);
        
        if (res != 0){
            printf("Operation Error");
            return ((void *)EXIT_FAILURE);
        }
    }

    close(sock);
    glob--;
    return ((void *)EXIT_SUCCESS);
}

int create_clients(config_t *config){
    struct sockaddr_in peer;
    int sock;
    int rc;
    char buf[1];

    peer.sin_family = AF_INET;
    peer.sin_port = htons(config->port);
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");

    int workers = glob;
    int i;

    for(i = 0; i < workers; i++){
        pthread_t thread;
        client_params_t params = {
            .config = config,
            .id = i,
        };

        printf("Cycle %d\n", i);

        int rv = pthread_create(&thread, NULL, run_client, &params);
        if (rv != 0){
            puts("RV fail");
            return (EXIT_FAILURE);
        }
        rv = pthread_join(thread, NULL);
        if (rv != 0){
            return (EXIT_FAILURE);
        }
    }

    while(glob){ ; }

    return (EXIT_SUCCESS);
}

int main(int argc, char * argv[]) {
    srand(time(NULL));

    config_t config = {
        .port = DEFAULT_PORT,
    };

    int rv = parse_config (&config, argc, argv);
    rv = create_clients(&config);

    if (rv != 0){
        printf("Failed %d\n", rv);
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}
