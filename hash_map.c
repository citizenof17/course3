#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hash_map.h"
#include "hash.h"
#include "protocol.h"

typedef struct entry_t {
    char *key;
    char *value;
    struct entry_t *next;
} entry_t;

typedef struct hash_map_t {
    int size; //number of elements
    int arr_size; //size of array
    int size_limit; //max number of elements
    int multiplier; 

    entry_t **entries;
} hash_map_t;

hash_map_t *create_hash_map(int arr_size, int size_limit, int multiplier){
    hash_map_t *map = (hash_map_t *)malloc(sizeof(hash_map_t));
    map->size = 0;
    map->arr_size = arr_size;
    map->size_limit = size_limit;
    map->multiplier = multiplier;
    map->entries = (entry_t **)malloc(arr_size * sizeof(entry_t *));

    return map;
}

entry_t *make_new_entry(char *key, char *value){
    entry_t *entry = (entry_t *)malloc(sizeof(entry_t));

    entry->key = (char*)malloc(strlen(key) + 1);
    entry->value = (char*)malloc(strlen(value) + 1);
    strcpy(entry->key, key);
    strcpy(entry->value, value);
    entry->next = NULL;

    return entry;
}

void entry_recursive_free(entry_t *entry){
    if (entry == NULL){ return; }

    if (entry->next){
        entry_recursive_free(entry->next);
    }
    free(entry->key);
    free(entry->value);
    free(entry->next);
    free(entry);
}

void delete_hash_map(hash_map_t *map){
    int i;
    for(i = 0; i < map->arr_size; i++){
        entry_recursive_free(map->entries[i]);
    }

    free(map);
}

void hash_map_insert(hash_map_t *map, char *key, char *value){
    int index = get_hash(key) % map->arr_size;

    if (map->entries[index] == NULL){
        map->entries[index] = make_new_entry(key, value);
    }
    else{
        entry_t *curr_entry = map->entries[index];

        while(curr_entry){
            if (strcmp(curr_entry->key, key) == 0){
                free(curr_entry->value);
                curr_entry->value = (char *)malloc(strlen(value) + 1);
                strcpy(curr_entry->value, value);
                return;
            }
            else if (curr_entry->next == NULL){
                curr_entry->next = make_new_entry(key, value);
                return;                  
            }
            curr_entry = curr_entry->next;
        } 
    }
}

void hash_map_erase(hash_map_t *map, char *key){
    int index = get_hash(key) % map->arr_size;

    entry_t *curr_entry = map->entries[index];
    entry_t **last_entry_pointer = &map->entries[index];

    while(curr_entry){
        if (strcmp(curr_entry->key, key) == 0){
            *last_entry_pointer = curr_entry->next;
            free(curr_entry->value);
            free(curr_entry->key);
            free(curr_entry);
            return;
        }
        last_entry_pointer = &curr_entry->next;
        curr_entry = curr_entry->next;
    }
}

void hash_map_get(hash_map_t *map, char *key, char *value){
    int index = get_hash(key) % map->arr_size;

    entry_t *curr_entry = map->entries[index];

    while(curr_entry){
        if (strcmp(curr_entry->key, key) == 0){
            strcpy(value, curr_entry->value);
            return;
        }
        curr_entry = curr_entry->next;
    }

    strcpy(value, "nil");
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

void hash_map_init(map_t *map, int arr_size, int multiplier){
    map->impl = (hash_impl_t *)malloc(sizeof(hash_impl_t));
    ((hash_impl_t *)map->impl)->map = create_hash_map(arr_size, arr_size * 2, multiplier);

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
    int i;
    for(i = 0; i < hash_map.arr_size; i++){
        entry_t *entry = hash_map.entries[i];
        while(entry){
            printf("|%s %s| ", entry->key, entry->value);
            entry = entry->next;
        }
        printf("\n");
    }
}
