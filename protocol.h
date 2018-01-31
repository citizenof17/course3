#pragma once

#define KEY_SIZE 250
#define VALUE_SIZE 500

typedef enum {
    OP_ERASE,
    OP_SET,
    OP_GET,
    SUCCESS,
    FAIL,
} operation_t;

typedef struct protocol_t {
    operation_t operation;
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
} protocol_t;