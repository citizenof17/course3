#pragma once

typedef enum {
    OP_ERASE,
    OP_SET,
    OP_GET,
    SUCCESS,
    FAIL,
} operation_t;

typedef struct protocol_t {
    operation_t operation;
    int key;
    int value;
    // char key[KEY_SIZE];
    // char value[VALUE_SIZE];
} protocol_t;


//operations:
//1 - insert, 2 - erase, 3 - contains, 4 - find
//10 for answer - successu
//11 for answer - fail
