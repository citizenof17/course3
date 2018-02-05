#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

int query(int *sock, int *rc, command_t *protocol, response_t *response);
int op_erase(int *sock, int *rc, char *key, response_t *response);
int op_set(int *sock, int *rc, char *key, char *value, response_t *response);
int op_get(int *sock, int *rc, char *key, response_t *response);