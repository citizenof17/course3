#include "operations.h"

void query(int *s, int *rc, struct protocol *prot, struct protocol *response) {
    if ((*rc = send(*s, (void *)prot, sizeof(struct protocol), 0)) <= 0) {
        perror("ошибка вызова send");
        exit(1);
    }

    if ((*rc = recv(*s, (void *)response, sizeof(struct protocol), 0)) <= 0) {
        perror("ошибка вызова recv");
        exit(1);
    }
}

int tryInsert(int *s, int *rc, int key, int value) {
    struct protocol que = {1, key, value};
    struct protocol resp = {-1, -1, -1};
    query(s, rc, &que, &resp);
    return resp.operation == 10;
}

int tryErase(int *s, int *rc, int key) {
    struct protocol que = {2, key, -1};
    struct protocol resp = {-1, -1, -1};
    query(s, rc, &que, &resp);
    return resp.operation == 10;
}

struct Pair tryContains(int *s, int *rc, int key) {
    struct protocol que = {3, key, -1};
    struct protocol resp = {-1, -1, -1};
    query(s, rc, &que, &resp);
    struct Pair result = {resp.operation == 10, resp.key};
    return result;
}

struct Pair tryGetValue(int *s, int *rc, int key) {
    struct protocol que = {4, key, -1};
    struct protocol resp = {-1, -1, -1};
    query(s, rc, &que, &resp);
    struct Pair result = {resp.operation == 10, resp.key};
    return result;
}

