#include <string.h>
#include "operations.h"

// generating a query to a server for an operation
int query(int *s, int *rc, command_t *command, response_t *response) {
    if ((*rc = send(*s, command, sizeof(*command), 0)) <= 0) {
        perror("ошибка вызова send");
        return (EXIT_FAILURE);
    }

    if ((*rc = recv(*s, response, sizeof(*response), 0)) <= 0) {
        perror("ошибка вызова recv");
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}

// delete a pair from a structure
int op_erase(int *sock, int *rc, char *key, response_t *response) {
    command_t command = {OP_ERASE, "", ""};
    strcpy(command.key, key);
    return query(sock, rc, &command, response);
}

// add a pair or change it's value in a structure
int op_set(int *sock, int *rc, char *key, char *value, response_t *response) {
    command_t command = {OP_SET, "", ""};
    strcpy(command.key, key);
    strcpy(command.value, value);
    return query(sock, rc, &command, response);
}

// try to find a pair in a structure
int op_get(int *sock, int *rc, char *key, response_t *response) {
    command_t command = {OP_GET, "", ""};
    strcpy(command.key, key);
    return query(sock, rc, &command, response);
}
