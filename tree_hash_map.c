#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "tree_hash_map.h"
#include "rb_tree.h"
#include "protocol.h"
#include "hash.h"

#define tree_t Tree
#define MIN_SIZE (5)

#define PREPARE_IMPL(map) \
    assert(map); assert(map->impl); \
    tree_hash_impl_t* impl = (tree_hash_impl_t *)map->impl;

typedef struct {
    int size;
    tree_t **trees;
    pthread_mutex_t *mutex;
} tree_hash_impl_t;

static response_t set(map_t * map, char * key, char * value){
    PREPARE_IMPL(map)

    int hash = get_hash(key) % impl->size;
    
    pthread_mutex_lock(&impl->mutex[hash]);
    insert(impl->trees[hash], key, value);
    pthread_mutex_unlock(&impl->mutex[hash]);
    response_t response = {"STORED", ""};
    return response;
}

static response_t get(map_t * map, char * key){
    PREPARE_IMPL(map)

    response_t response;

    int hash = get_hash(key) % impl->size;
    pthread_mutex_lock(&impl->mutex[hash]);    
    getValue(impl->trees[hash], key, response.value);
    pthread_mutex_unlock(&impl->mutex[hash]);

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

    pthread_mutex_lock(&impl->mutex[hash]);
    erase(impl->trees[hash], key);
    pthread_mutex_unlock(&impl->mutex[hash]);
    
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
    impl->mutex = (pthread_mutex_t *)malloc(size * sizeof(pthread_mutex_t));

    int i;
    for(i = 0; i < size; i++){
        impl->trees[i] = createTree();
        pthread_mutex_init(&impl->mutex[i], NULL);
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
    free(impl->mutex);    
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