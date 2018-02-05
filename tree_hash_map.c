#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "tree_hash_map.h"
#include "rb_tree.h"
#include "protocol.h"

#define tree_t Tree
#define MIN_SIZE (5)

#define PREPARE_IMPL(map) \
    assert(map); assert(map->impl); \
    tree_hash_impl_t* impl = (tree_hash_impl_t *)map->impl;

typedef struct {
    int size;
    tree_t **trees;
} tree_hash_impl_t;

int64_t A = 1e4 + 7;
int64_t M = 1e9 + 7;

int64_t get_hash(char *str){
    int64_t hash = 0;

    int i = 0;
    while(str[i] != '\0'){
        hash = (hash * A + str[i]) % M;
        i++;
    }

    return hash;
}

static response_t set(map_t * map, char * key, char * value){
    PREPARE_IMPL(map)

    int hash = get_hash(key) % impl->size;
    
    insert(impl->trees[hash], key, value);
    response_t response = {"STORED", ""};
    return response;
}

static response_t get(map_t * map, char * key){
    PREPARE_IMPL(map)

    response_t response;

    int hash = get_hash(key) % impl->size;
    getValue(impl->trees[hash], key, response.value);

    if (strcmp(response.value, "nil") == 0){  ///???? handle empty node
        strcpy(response.answer, "NOT_FOUND");
    }
    else{
        strcpy(response.answer, "FOUND");
    }
    return response;
}

static response_t rem(map_t * map, char * key){
    PREPARE_IMPL(map)

    int hash = get_hash(key) % impl->size;

    erase(impl->trees[hash], key);
    response_t response = {"DELETED", ""};
    return response;
}

void tree_hash_map_init(map_t * map, int size){
    assert(map);

    map->impl = malloc(sizeof(tree_hash_impl_t));
    tree_hash_impl_t * impl = (tree_hash_impl_t *)map->impl;

    if (size < MIN_SIZE){ size = MIN_SIZE; }

    impl->size = size;
    impl->trees = (tree_t **)malloc(size * sizeof(tree_t *));

    int i;
    for(i = 0; i < size; i++){
        impl->trees[i] = createTree();
    }

    map->get = &get;
    map->set = &set;
    map->rem = &rem;
}

void tree_hash_map_free(map_t *map){
    PREPARE_IMPL(map);
    
    int i;
    for(i = 0; i < impl->size; i++){
        deleteTree(impl->trees[i]);
    }

    free(impl->trees);
    free(map->impl);
}

void tree_hash_map_print(map_t *map){
    PREPARE_IMPL(map);

    int i;
    for(i = 0; i < impl->size; i++){
        printTree(impl->trees[i]->root, 3);
        puts("\n\n");
    }
}