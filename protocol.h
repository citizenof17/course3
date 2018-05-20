#pragma once

#define KEY_SIZE 20
#define VALUE_SIZE 20
#define ANSWER_SIZE 15

#define protocol_t command_t

typedef enum {
    OP_ERASE,
    OP_SET,
    OP_GET,
    SUCCESS, // not used
    FAIL,    // not used
} operation_t;

typedef struct command_t {
    operation_t operation;
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
} command_t;

typedef struct response_t {
    char answer[ANSWER_SIZE];
    char value[VALUE_SIZE];
} response_t;