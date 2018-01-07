#pragma once

struct protocol {
    int operation;
    int key;
    int value;
};

//operations:
//1 - insert, 2 - erase, 3 - contains, 4 - find
//10 for answer - successu
//11 for answer - fail
