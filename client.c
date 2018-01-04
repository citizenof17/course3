#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "protocol.h"

void query(int *s, int *rc, struct protocol *prot, struct protocol *response){
    *rc = send(*s, (void*)prot, sizeof(struct protocol), 0);

    if (*rc <= 0){
        perror("ошибка вызова send");
        exit(1);
    }

    *rc = recv(*s, (void*)response, sizeof(struct protocol), 0);

    if (*rc <= 0){
        perror("ошибка вызова recv");
        exit(1);
    }
}

int main(void)
{
    struct sockaddr_in peer;
    int s;
    int rc;
    char buf[1];

    peer.sin_family = AF_INET;
    peer.sin_port = htons(7500);
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");

    s = socket(AF_INET, SOCK_STREAM, 0);

    if (s < 0){
        perror("ошибка вызова socket");
        exit(1);
    }

    rc = connect(s, (struct sockaddr *)&peer, sizeof(peer));
    if (rc){
        perror("ошибка вызова connect");
        exit(1);
    }

    int n = 6;
    rc = send(s, (void*)&n, sizeof(int), 0);
    if (rc <= 0){
        perror("ошибка вызова send");
        exit(1);
    }
    
    rc = recv(s, buf, 1, 0);
    if (rc <= 0){
        perror("ошибка вызова recv");
        exit(1);
    }

    //////////////////////

    printf("Sending %d operations\n", n);

    //insert
    struct protocol prot = {1, 1, 100};
    struct protocol response = {-1, -1, -1};

    for (int i = 1; i <= 3; i++){
        prot.key = i;
        prot.value = i + 100;
        query(&s, &rc, &prot, &response);

        if (response.operation == 10){
            printf("Succes\n");
        }
        else{
            printf("Fail\n");
        }
    }

    //erase
    prot.operation = 2;
    prot.key = 1;

    query(&s, &rc, &prot, &response);
    
    if (response.operation == 10){
        printf("Erasing\n");
    }
    else{
        printf("Fail\n");
    }

    //contains
    prot.operation = 3;
    prot.key = 2;

    query(&s, &rc, &prot, &response);
    if (response.operation == 10){
        printf("Succes: ");
        if (response.key == 1){
            printf("Contains\n");
        }
        else{
            printf("Not contains\n");
        }
    }
    else{
        printf("Fail\n");
    }

    prot.key = 1;

    query(&s, &rc, &prot, &response);
        if (response.operation == 10){
        printf("Succes: ");
        if (response.key == 1){
            printf("Contains\n");
        }
        else{
            printf("Not contains\n");
        }
    }
    else{
        printf("Fail\n");
    }

    close(s);
    exit(0);
}