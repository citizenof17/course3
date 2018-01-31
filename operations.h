#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

int query(int *sock, int *rc, protocol_t *protocol, protocol_t *response);
int op_erase(int *sock, int *rc, char *key, protocol_t *response);
int op_set(int *sock, int *rc, char *key, char *value, protocol_t *response);
int op_get(int *sock, int *rc, char *key, protocol_t *response);