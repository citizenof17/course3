#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "tree_map.h"
#include "rb_tree.h"
#include "protocol.h"

#define tree_t Tree

typedef struct {
    tree_t *tree;
} tree_impl_t;

#define PREPARE_IMPL(map) \
    assert(map); assert(map->impl); \
    tree_impl_t* impl = (tree_impl_t *)map->impl;


static response_t set(map_t * map, char * key, char * value){
    PREPARE_IMPL(map)

    insert(impl->tree, key, value);
    response_t response = {"STORED", ""};
    return response;
}

static response_t get(map_t * map, char * key){
    PREPARE_IMPL(map)

    response_t response;
    getValue(impl->tree, key, response.value);

    if (strcmp(response.value, "nil") == 0){  ///???? handle empty node
        strcpy(response.answer, "NOT_FOUND");
    }
    else{
        strcpy(response.answer, "FOUND");
    }
    return response;
}

static response_t remove(map_t * map, char * key){
    PREPARE_IMPL(map)

    erase(impl->tree, key);
    response_t response = {"DELETED", ""};
    return response;
}

void tree_map_init(map_t * map){
    assert(map);

    map->impl = malloc(sizeof(tree_impl_t));
    ((tree_impl_t *)map->impl)->tree = createTree();
    
    map->get = &get;
    map->set = &set;
    map->remove = &remove;
}

void tree_map_free(map_t *map){
    PREPARE_IMPL(map);
    deleteTree(impl->tree);
}

void tree_map_print(map_t *map){
    PREPARE_IMPL(map);

    printTree(impl->tree->root, 3);
}