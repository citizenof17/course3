#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "protocol.h"
#include "operations.h"

int main(void) {
    srand(time(NULL));

    struct sockaddr_in peer;
    int s;
    int rc;
    char buf[1];

    peer.sin_family = AF_INET;
    peer.sin_port = htons(7500);
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");

    s = socket(AF_INET, SOCK_STREAM, 0);

    if (s < 0) {
        perror("ошибка вызова socket");
        exit(1);
    }

    rc = connect(s, (struct sockaddr *)&peer, sizeof(peer));
    if (rc) {
        perror("ошибка вызова connect");
        exit(1);
    }

    int n = 10;
    scanf("%d", &n);

    rc = send(s, (void *)&n, sizeof(int), 0);
    if (rc <= 0) {
        perror("ошибка вызова send");
        exit(1);
    }

    rc = recv(s, buf, 1, 0);
    if (rc <= 0) {
        perror("ошибка вызова recv");
        exit(1);
    }

    //////////////////////

    printf("Sending %d operations\n", n);

    //insert

    int keys[]   = {1, 2, 3, 10, 15, -5, 5, 5, 5};
    int values[] = {1, 2, 3, 10, 15, 100, 5, 6, 7};
    
    int k = sizeof(keys) / sizeof(int);

    int i;
    for (i = 0; i < k; i++) {
        printf("Inserting key %d with value %d\n", keys[i], values[i]);
        tryInsert(&s, &rc, keys[i], values[i]);
    }

    printf("Contains 10? %d\n", tryContains(&s, &rc, 10).second);
    printf("Contains 15? %d\n", tryContains(&s, &rc, 15).second);
    printf("Contains 0? %d\n",  tryContains(&s, &rc, 0) .second);
    printf("Contains -4? %d\n", tryContains(&s, &rc, -4).second);

    printf("Contains 3? %d\n", tryContains(&s, &rc, 3).second);
    printf("Erasing 3 -> %d\n", tryErase(&s, &rc, 3));
    printf("Contains 3? %d\n", tryContains(&s, &rc, 3).second);

    printf("Value of 2 -> %d\n", tryGetValue(&s, &rc, 2).second);
    printf("Value of -5 -> %d\n", tryGetValue(&s, &rc, -5).second);
    printf("Value of -4 -> %d\n", tryGetValue(&s, &rc, -4).second);

    printf("Contains -4? %d\n", tryContains(&s, &rc, -4).second);
    
    close(s);
    exit(0);
}
