#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "tree_map.h"
#include "rb_tree.h"
#include "protocol.h"

#define tree_t Tree

typedef struct {
    tree_t *tree;
    pthread_rwlock_t rwlock;
} tree_impl_t;

#define PREPARE_IMPL(map) \
    assert(map); assert(map->impl); \
    tree_impl_t* impl = (tree_impl_t *)map->impl;

static response_t set(map_t * map, char * key, char * value){
    PREPARE_IMPL(map)

    pthread_rwlock_wrlock(&impl->rwlock);
    insert(impl->tree, key, value);
    pthread_rwlock_unlock(&impl->rwlock);
    response_t response = {"STORED", ""};
    return response;
}

static response_t get(map_t * map, char * key){
    PREPARE_IMPL(map)

    response_t response;
    pthread_rwlock_rdlock(&impl->rwlock);
    getValue(impl->tree, key, response.value);
    pthread_rwlock_unlock(&impl->rwlock);

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

    pthread_rwlock_wrlock(&impl->rwlock);
    erase(impl->tree, key);
    pthread_rwlock_unlock(&impl->rwlock);
    
    response_t response = {"DELETED", ""};
    return response;
}

// creating a tree_map
void tree_map_init(map_t * map){
    assert(map);

    map->impl = malloc(sizeof(tree_impl_t));
    ((tree_impl_t *)map->impl)->tree = createTree();
    pthread_rwlock_init(&((tree_impl_t *)map->impl)->rwlock, NULL);

    map->get = &get;
    map->set = &set;
    map->rem = &rem;
}

// freeing a tree_map
void tree_map_free(map_t *map){
    PREPARE_IMPL(map);
    deleteTree(impl->tree);
    free(impl->tree);
    free(map->impl);
}

// debugging print
void tree_map_print(map_t *map){
    PREPARE_IMPL(map);
    printTree(impl->tree->root, 3);
}