#include "operations.h"

int query(int *s, int *rc, protocol_t *protocol, protocol_t *response) {
    // protocol
    if ((*rc = send(*s, protocol, sizeof(*protocol), 0)) <= 0) {
        perror("ошибка вызова send");
        return (EXIT_FAILURE);
    }

    if ((*rc = recv(*s, response, sizeof(*response), 0)) <= 0) {
        perror("ошибка вызова recv");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

int op_erase(int *sock, int *rc, int key, protocol_t *response) {
    protocol_t protocol = {OP_ERASE, key, 0};
    response->operation = OP_ERASE;
    // protocol_t response = {OP_ERASE, key, 0};
    return query(sock, rc, &protocol, response);
}

int op_set(int *sock, int *rc, int key, int value, protocol_t *response) {
    protocol_t protocol = {OP_SET, key, value};
    response->operation = OP_SET;
    // protocol_t response = {OP_SET, key, value};
    return query(sock, rc, &protocol, response);
}

int op_get(int *sock, int *rc, int key, protocol_t *response) {
    protocol_t protocol = {OP_GET, key, 0};
    response->operation = OP_GET;
    // protocol_t response = {OP_GET, key, 0};
    return query(sock, rc, &protocol, response);
}

// void query(int *s, int *rc, struct protocol *prot, struct protocol *response) {
//     if ((*rc = send(*s, (void *)prot, sizeof(struct protocol), 0)) <= 0) {
//         perror("ошибка вызова send");
//         exit(1);
//     }

//     if ((*rc = recv(*s, (void *)response, sizeof(struct protocol), 0)) <= 0) {
//         perror("ошибка вызова recv");
//         exit(1);
//     }
// }

// int tryInsert(int *s, int *rc, int key, int value) {
//     struct protocol que = {1, key, value};
//     struct protocol resp = {-1, -1, -1};
//     query(s, rc, &que, &resp);
//     return resp.operation == 10;
// }

// int tryErase(int *s, int *rc, int key) {
//     struct protocol que = {2, key, -1};
//     struct protocol resp = {-1, -1, -1};
//     query(s, rc, &que, &resp);
//     return resp.operation == 10;
// }

// struct Pair tryContains(int *s, int *rc, int key) {
//     struct protocol que = {3, key, -1};
//     struct protocol resp = {-1, -1, -1};
//     query(s, rc, &que, &resp);
//     struct Pair result = {resp.operation == 10, resp.key};
//     return result;
// }

// struct Pair tryGetValue(int *s, int *rc, int key) {
//     struct protocol que = {4, key, -1};
//     struct protocol resp = {-1, -1, -1};
//     query(s, rc, &que, &resp);
//     struct Pair result = {resp.operation == 10, resp.key};
//     return result;
// }

