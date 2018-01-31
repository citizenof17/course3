#include <string.h>
#include "operations.h"

int query(int *s, int *rc, protocol_t *protocol, protocol_t *response) {
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

int op_erase(int *sock, int *rc, char *key, protocol_t *response) {
    protocol_t protocol = {OP_ERASE, "", ""};
    strcpy(protocol.key, key);
    response->operation = OP_ERASE;
    return query(sock, rc, &protocol, response);
}

int op_set(int *sock, int *rc, char *key, char *value, protocol_t *response) {
    protocol_t protocol = {OP_SET, "", ""};
    strcpy(protocol.key, key);
    strcpy(protocol.value, value);
    response->operation = OP_SET;
    return query(sock, rc, &protocol, response);
}

int op_get(int *sock, int *rc, char *key, protocol_t *response) {
    protocol_t protocol = {OP_GET, "", ""};
    strcpy(protocol.key, key);
    response->operation = OP_GET;
    return query(sock, rc, &protocol, response);
}
