#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

struct Pair {
    int first;
    int second;
};

void query(int *s, int *rc, struct protocol *prot, struct protocol *response);
int tryInsert(int *s, int *rc, int key, int value);
int tryErase(int *s, int *rc, int key);
struct Pair tryContains(int *s, int *rc, int key);
struct Pair tryGetValue(int *s, int *rc, int key);