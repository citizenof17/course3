#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "hash_map2.h"
#include "hash.h"
#include "protocol.h"

#define LOAD_FACTOR (0.75)
#define MULTIPLIER (2)

typedef struct entry_t {
    char *key;
    char *value;
    char deleted;
} entry_t;

typedef struct hash_map_t {
    int size; //number of elements
    int arr_size; //size of array
    double load_factor;
    int multiplier; 
    // int semaphore;

    entry_t *entries;
    pthread_rwlock_t rwlock;
} hash_map_t;

hash_map_t *create_hash_map(int arr_size, double load_factor, int multiplier){
    hash_map_t *map = (hash_map_t *)malloc(sizeof(hash_map_t));
    map->size = 0;
    map->arr_size = arr_size;
    map->multiplier = multiplier;
    map->load_factor = load_factor;
    map->entries = (entry_t *)malloc(arr_size * sizeof(entry_t));
    pthread_rwlock_init(&map->rwlock, NULL);

    memset(map->entries, NULL, arr_size * sizeof(entry_t));

    return map;
}

void delete_hash_map(hash_map_t *map){
    int i;
    for(i = 0; i < map->arr_size; i++){
        free(map->entries[i].value);
        free(map->entries[i].key);
    }

    free(map->entries);
    free(map);
}

void hash_map_insert_light(hash_map_t *map, char *key, char *value){
    int index = get_hash(key) % map->arr_size;

    while (map->entries[index].key != NULL &&
           strcmp(map->entries[index].key, key) != 0){
        index = (index + 1) % map->arr_size;
    }

    map->entries[index].key = key;
    map->entries[index].value = value;        
}

void hash_map_rebuild(hash_map_t *map){
    // store all entries in map
    entry_t *aux_entries = (entry_t*)malloc(map->arr_size * sizeof(entry_t));
    int i;

    // get all entries
    for (i = 0; i < map->arr_size; i++){
        aux_entries[i].key = map->entries[i].key;
        aux_entries[i].value = map->entries[i].value;
    }

    // free old map
    free(map->entries);
    int old_size = map->arr_size;
    map->arr_size = map->arr_size * map->multiplier;
    map->entries = (entry_t *)malloc(map->arr_size * sizeof(entry_t));

    // add all entries to new hash map
    for(i = 0; i < map->arr_size; i++){
        map->entries[i].value = NULL;
        map->entries[i].key = NULL;
    }

    for(i = 0; i < old_size; i++){
        if (aux_entries[i].key != NULL){
            hash_map_insert_light(map, aux_entries[i].key, aux_entries[i].value);
        }
    }
}

void hash_map_insert(hash_map_t *map, char *key, char *value){
    // if map size is bigger than size limit then hash map 
    // should be rebuilded

    pthread_rwlock_wrlock(&map->rwlock);

    if (map->size >= map->arr_size * map->load_factor){
        hash_map_rebuild(map);
    }

    int index = get_hash(key) % map->arr_size;
    int first_deleted = -1;

    // fixind problem:
    // [a, b, c] -> delete b -> [a, _, c] -> add c -> [a, c, c]

    // if number of iterations > map->arr-size then it should be rebuilt
    while (map->entries[index].deleted || 
           (map->entries[index].key != NULL &&
           strcmp(map->entries[index].key, key)) != 0){

        if (map->entries[index].deleted && first_deleted == -1){
            first_deleted = index;
        }
        index = (index + 1) % map->arr_size;
    }

    // found entry with same key
    if (map->entries[index].key != NULL && 
        strcmp(map->entries[index].key, key) == 0){
        free(map->entries[index].value);
        map->entries[index].value = (char *)malloc(strlen(value) + 1);
        strcpy(map->entries[index].value, value);
    }
    else {
        // no entry with same key
        if (first_deleted != -1){
            index = first_deleted;
        }
        free(map->entries[index].key);
        map->entries[index].key = (char *)malloc(strlen(key) + 1);
        strcpy(map->entries[index].key, key);

        free(map->entries[index].value);
        map->entries[index].value = (char *)malloc(strlen(value) + 1);        
        strcpy(map->entries[index].value, value);
        map->size++;
    }

    map->entries[index].deleted = 0;

    pthread_rwlock_unlock(&map->rwlock);
}

void hash_map_erase(hash_map_t *map, char *key){
    pthread_rwlock_wrlock(&map->rwlock);

    int index = get_hash(key) % map->arr_size;

    int cnt = 0;
    // what about situation: [a, DEL, b, DEL, c, DEL, DEL] ?
    while(map->entries[index].deleted || 
          (map->entries[index].key != NULL &&
          strcmp(map->entries[index].key, key)) != 0 &&
          cnt++ < map->arr_size){
        index = (index + 1) % map->arr_size;
    }

    // found exact key
    if (map->entries[index].key != NULL &&
        strcmp(map->entries[index].key, key) == 0){
        free(map->entries[index].key);
        free(map->entries[index].value);
        map->entries[index].key = NULL;
        map->entries[index].value = NULL;
        map->entries[index].deleted = 1;
        map->size--;
    }
    pthread_rwlock_unlock(&map->rwlock);    
}

void hash_map_get(hash_map_t *map, char *key, char *value){
    pthread_rwlock_rdlock(&map->rwlock);
    int index = get_hash(key) % map->arr_size;

    int cnt = 0;
    while(map->entries[index].deleted || 
          (map->entries[index].key != NULL &&
          strcmp(map->entries[index].key, key)) != 0 &&
          cnt++ < map->arr_size){
        index = (index + 1) % map->arr_size;
    }

    // found 
    if (map->entries[index].key != NULL &&
          strcmp(map->entries[index].key, key) == 0){
        strcpy(value, map->entries[index].value);
    }
    else{
        strcpy(value, "nil");
    }

    // didn't find
    pthread_rwlock_unlock(&map->rwlock);
}

#define PREPARE_IMPL(map) \
    assert(map); assert(map->impl); \
    hash_impl_t* impl = (hash_impl_t *)map->impl;

typedef struct {
    hash_map_t *map;
} hash_impl_t;

static response_t get(map_t *map, char *key){
    PREPARE_IMPL(map)

    response_t response;
    hash_map_get(impl->map, key, response.value);
    if (strcmp(response.value, "nil") == 0){
        strcpy(response.answer, "NOT_FOUND");
    }
    else{
        strcpy(response.answer, "FOUND");
    }
    return response;
}

static response_t set(map_t *map, char *key, char *value){
    PREPARE_IMPL(map)

    hash_map_insert(impl->map, key, value);
    response_t response = {"STORED", ""};
    return response;
}

static response_t rem(map_t *map, char *key){
    PREPARE_IMPL(map)

    hash_map_erase(impl->map, key);
    response_t response = {"DELETED", ""};
    return response;
}

void hash_map_init(map_t *map, int arr_size, int multiplier){  // delete multiplier?
    map->impl = (hash_impl_t *)malloc(sizeof(hash_impl_t));
    ((hash_impl_t *)map->impl)->map = 
        create_hash_map(arr_size, LOAD_FACTOR, MULTIPLIER);

    map->get = &get;
    map->set = &set;
    map->rem = &rem;
}

void hash_map_free(map_t *map){
    PREPARE_IMPL(map)
    delete_hash_map(impl->map);
}

void hash_map_print(map_t *map){
    PREPARE_IMPL(map)

    hash_map_t hash_map = *(impl->map);
    printf("Size: %d %d\n", hash_map.size, hash_map.arr_size);
    int i;
    for(i = 0; i < hash_map.arr_size; i++){
        printf("%d %s %s\n", i, 
            (hash_map.entries[i].key == NULL ? "nil" : hash_map.entries[i].key), 
            (hash_map.entries[i].value == NULL ? "nil" : hash_map.entries[i].value));
    }
    printf("\n");
}
