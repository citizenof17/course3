#pragma once

struct protocol {
    int operation;
    int key;
    int value;
};

//operations:
//1 - insert, 2 - erase, 3 - contains
//10 for answer - success
//11 for answer - fail

// typedef enum {False, True} bool; 