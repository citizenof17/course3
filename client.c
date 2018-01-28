#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "protocol.h"
#include "operations.h"

#define DEFAULT_PORT (7500)

int glob = 5;

typedef struct client_params_t{
    // struct sockaddr_in *peer;
    int id;
} client_params_t;

void * run_client(void * arg){
    client_params_t client_params = *(client_params_t *)arg;

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(DEFAULT_PORT);
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sock;
    int rc;
    char buf[1];
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ошибка вызова socket");
        return (EXIT_FAILURE);
    }
    
    if ((rc = connect(sock, (struct sockaddr *)&peer, sizeof(peer))) > 0) {
        perror("ошибка вызова connect");
        return (EXIT_FAILURE);        
    }
    printf("Connected %d\n", client_params.id);

    int n = rand() % 100;

    if ((rc = send(sock, &n, sizeof(n), 0)) <= 0) {
        perror("ошибка вызова send");
        return (EXIT_FAILURE);
    }


    if ((rc = recv(sock, buf, 1, 0)) <= 0) {
        perror("ошибка вызова recv");
        return (EXIT_FAILURE);
    }

    //////////////////////

    printf("Client %d is sending %d operations\n", client_params.id, n);

    //insert
    protocol_t response;

    int i, res;
    for (i = 0; i < n; i++) {
        int key = rand() % 500;
        int value = rand() % 100;
        int oper = rand() % 3;

        switch(oper) {
            case 0:
                res = op_erase(&sock, &rc, key, &response);
                break;
            case 1:
                res = op_set(&sock, &rc, key, value, &response);
                break;
            case 2:
                res = op_get(&sock, &rc, key, &response);
                break;
        }

        if (res != 0){
            printf("Operation Error");
            return (EXIT_FAILURE);
        }
        printf("ID: %d -> oper: %d, resp: %d, key: %d, value: %d\n", 
            client_params.id, oper, response.operation, response.key, response.value);
    }

    close(sock);
    glob--;
    return (EXIT_SUCCESS);
}

int create_clients(){
    struct sockaddr_in peer;
    int sock;
    int rc;
    char buf[1];

    peer.sin_family = AF_INET;
    peer.sin_port = htons(DEFAULT_PORT);
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");

    int workers = glob;
    int i;

    for(i = 0; i < workers; i++){
        pthread_t thread;
        client_params_t params = {
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

int main(void) {
    srand(time(NULL));

    int rv = create_clients();

    if (rv != 0){
        printf("Failed %d\n", rv);
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}
