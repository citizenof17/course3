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
    pthread_mutex_t mutex;
} tree_impl_t;

#define PREPARE_IMPL(map) \
    assert(map); assert(map->impl); \
    tree_impl_t* impl = (tree_impl_t *)map->impl;


static response_t set(map_t * map, char * key, char * value){
    PREPARE_IMPL(map)

    pthread_mutex_lock(&impl->mutex);
    insert(impl->tree, key, value);
    pthread_mutex_unlock(&impl->mutex);
    response_t response = {"STORED", ""};
    return response;
}

static response_t get(map_t * map, char * key){
    PREPARE_IMPL(map)

    response_t response;
    pthread_mutex_lock(&impl->mutex);
    getValue(impl->tree, key, response.value);
    pthread_mutex_unlock(&impl->mutex);

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

    pthread_mutex_lock(&impl->mutex);
    erase(impl->tree, key);
    pthread_mutex_unlock(&impl->mutex);
    
    response_t response = {"DELETED", ""};
    return response;
}

void tree_map_init(map_t * map){
    assert(map);

    map->impl = malloc(sizeof(tree_impl_t));
    ((tree_impl_t *)map->impl)->tree = createTree();
    pthread_mutex_init(&((tree_impl_t *)map->impl)->mutex, NULL);

    map->get = &get;
    map->set = &set;
    map->rem = &rem;
}

void tree_map_free(map_t *map){
    PREPARE_IMPL(map);
    deleteTree(impl->tree);
    free(map->impl);
}

void tree_map_print(map_t *map){
    PREPARE_IMPL(map);
    printTree(impl->tree->root, 3);
}