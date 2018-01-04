#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "protocol.h"
#include "rb_tree.h"

void handleQuery(int *rc, int *s1, struct protocol *query, Tree *T){
    *rc = recv(*s1, (void*)query, sizeof(struct protocol), 0);
    struct protocol response = {-1, -1, -1};

    if (*rc < 0){
        perror("ошибка вызова recv");
        exit(1);
    } 
    else {
        if (query->operation == 1){
            insert(T, query->key);
            response.operation = 10;
        }
        else if (query->operation == 2){
            erase(T, query->key);
            response.operation = 10;
        }
        else if (query->operation == 3){
            response.operation = 10;
            response.key = contains(T, T->root, query->key);
        }
    }

    *rc = send(*s1, (void*)&response, sizeof(struct protocol), 0);
    if (*rc <= 0){
        perror("ошибка вызова send");
    }    
}

int main(void)
{
    struct sockaddr_in local;
    int s;
    int s1;
    int rc;
    char buf[1];
    local.sin_family = AF_INET;
    local.sin_port = htons(7500);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0){
        perror("ошибка вызова socket");
        exit(1);
    }

    rc = bind(s, (struct sockaddr *)&local, sizeof(local));
    if (rc < 0){
        perror("ошибка вызова bind");
        exit(1);
    }

    if (rc = listen(s, 5)){
        perror("ошибка вызова listen");
        exit(1);
    }
    
    s1 = accept(s, NULL, NULL);
    if (s1 < 0){
        perror("ошибка вызова accept");
        exit(1);
    }


    ///////////////////////////////

    int n;
    //getting number of operations;
    rc = recv(s1, (void*)&n, sizeof(n), 0);

    if (rc <= 0){
        printf("Failed\n");
        exit(1);
    }
    else{
        printf("%d\n", n);
    }

    rc = send(s1, "0", 1, 0);
    if (rc <= 0){
        perror("ошибка вызова send");
    }  

    Tree *tr = (Tree*)malloc(sizeof(Tree));
    tr->nil = makeNewNode(tr, 0);
    tr->root = tr->nil;

    for (int i = 0; i < n; i++){
        struct protocol query;
        handleQuery(&rc, &s1, &query, tr);
    }

    //resulting tree;
    printTree(tr->root, 3);
    clearTree(tr, tr->root);

    close(s);
    close(s1);
    exit(0);
}