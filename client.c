#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "protocol.h"
#include "operations.h"

#define DEFAULT_PORT (7500)

int run_client(){
    struct sockaddr_in peer;
    int sock;
    int rc;
    char buf[1];

    peer.sin_family = AF_INET;
    peer.sin_port = htons(DEFAULT_PORT);
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ошибка вызова socket");
        return (EXIT_FAILURE);
    }
    
    if ((rc = connect(sock, (struct sockaddr *)&peer, sizeof(peer))) > 0) {
        perror("ошибка вызова connect");
        return (EXIT_FAILURE);        
    }

    int n = rand() % 100;
    n = 4;

    if ((rc = send(sock, &n, sizeof(n), 0)) <= 0) {
        perror("ошибка вызова send");
        return (EXIT_FAILURE);
    }

    if ((rc = recv(sock, buf, 1, 0)) <= 0) {
        perror("ошибка вызова recv");
        return (EXIT_FAILURE);
    }
    puts("Received");

    //////////////////////

    printf("Sending %d operations\n", n);

    //insert

    int keys[]   = {1, 2, 3, 10};
    int values[] = {1, 2, 3, 10};
    
    int k = sizeof(keys) / sizeof(int);
    protocol_t response;

    int i, res;
    for (i = 0; i < k; i++) {
        printf("Inserting key %d with value %d\n", keys[i], values[i]);
        res = op_set(&sock, &rc, keys[i], values[i], &response);
        if (res != 0){
            printf("Operation Error");
            return (EXIT_FAILURE);
        }
        printf("%d\n", response.operation);
    }

    close(sock);
    return (EXIT_SUCCESS);
}

int main(void) {
    srand(time(NULL));

    int rv = run_client();

    if (rv != 0){
        printf("Failed");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}
