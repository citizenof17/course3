#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

// struct Pair {
//     int first;
//     int second;
// };

int query(int *sock, int *rc, protocol_t *protocol, protocol_t *response);
int op_erase(int *sock, int *rc, int key, protocol_t *response);
int op_set(int *sock, int *rc, int key, int value, protocol_t *response);
int op_get(int *sock, int *rc, int key, protocol_t *response);

// void query(int *s, int *rc, struct protocol *prot, struct protocol *response);
// int tryInsert(int *s, int *rc, int key, int value);
// int tryErase(int *s, int *rc, int key);
// struct Pair tryContains(int *s, int *rc, int key);
// struct Pair tryGetValue(int *s, int *rc, int key);