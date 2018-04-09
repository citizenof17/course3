#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "hash_map.h"
#include "hash.h"
#include "protocol.h"

#define LOAD_FACTOR (0.75)

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
    int semaphore;

    entry_t **entries;
    pthread_mutex_t *mutex;
} hash_map_t;

void semaphore_init(hash_map_t *map, int value){
    map->semaphore = value;
}

void semaphore_lock(hash_map_t *map){
    while(1){
        if (map->semaphore > 0){
            map->semaphore -= map->arr_size;
            break;
        }
    }
    // printf("SEMAPHORE %d %d\n", map->arr_size, map->semaphore);
    while(1){
        if (map->semaphore == -1){
            break;
        }
    }
}

void semaphore_enter(hash_map_t *map){
    while(1){
        if (map->semaphore > 0){
            map->semaphore -= 1;
            break;
        }
    }
}

void semaphore_leave(hash_map_t *map){
    map->semaphore += 1;
}

hash_map_t *create_hash_map(int arr_size, int size_limit, int multiplier){
    hash_map_t *map = (hash_map_t *)malloc(sizeof(hash_map_t));
    map->size = 0;
    map->arr_size = arr_size;
    map->size_limit = size_limit;
    map->multiplier = multiplier;
    map->entries = (entry_t **)malloc(arr_size * sizeof(entry_t *));
    map->mutex = (pthread_mutex_t *)malloc(arr_size * sizeof(pthread_mutex_t));
    semaphore_init(map, map->arr_size);
    memset(map->entries, NULL, sizeof(*map->entries));

    return map;
}

void create_hash_map_light(hash_map_t * map, int arr_size, 
                           int size_limit, int multiplier){
    map->size = 0;
    map->arr_size = arr_size;
    map->size_limit = size_limit;
    map->multiplier = multiplier;
    map->entries = (entry_t **)malloc(arr_size * sizeof(entry_t *));
    map->mutex = (pthread_mutex_t *)malloc(arr_size * sizeof(pthread_mutex_t));
    memset(map->entries, NULL, sizeof(*map->entries));
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

    free(map->mutex);
    free(map);
}

void get_all_entries(entry_t **aux_entries, hash_map_t *map){
    int i;
    int j = 0; 
    for (i = 0; i < map->arr_size; i++){
        // check each row of the array
        entry_t *entry = map->entries[i];  
        
        // check each column of the array and store entry
        while(entry != NULL){
            aux_entries[j++] = entry;
            entry = entry->next;
        }
    }
}

// free entries without deleting keys and values
void entry_recursive_free_light(entry_t *entry){
    if (entry == NULL){ return; }

    if (entry->next){
        entry_recursive_free_light(entry->next);
    }
    entry->next = NULL;
}

// free hash_map without deleting entries
void delete_hash_map_light(hash_map_t *map){
    int i;
    for(i = 0; i < map->arr_size; i++){
        entry_recursive_free_light(map->entries[i]);
    }

    free(map->mutex);
}

void hash_map_insert_light(hash_map_t *map, entry_t *entry){
    int index = get_hash(entry->key) % map->arr_size;
    if (map->entries[index] == NULL){
        map->entries[index] = entry;
    }
    else{
        entry_t *curr_entry = map->entries[index];

        while(curr_entry){
            if (curr_entry->next == NULL){
                curr_entry->next = entry;
                return;                  
            }
            curr_entry = curr_entry->next;
        } 
    }
}

void hash_map_rebuild(hash_map_t *map){
    // store all entries in map
    entry_t **aux_entries = (entry_t**)malloc(map->size * sizeof(entry_t*));
    int i;

    //semaphore
    semaphore_lock(map);
    
    for(i = 0; i < map->arr_size; i++){
        pthread_mutex_lock(&map->mutex[i]);
    }
    get_all_entries(aux_entries, map);
    for(i = 0; i < map->arr_size; i++){
        pthread_mutex_unlock(&map->mutex[i]);
    }

    // free old map
    int size = map->size;
    int size_limit = map->size_limit;
    int multiplier = map->multiplier;
    int arr_size = map->arr_size;
    delete_hash_map_light(map);

    int new_size = arr_size * multiplier;
    create_hash_map_light(map, new_size, new_size * LOAD_FACTOR, multiplier);
    map->size = size;

    // add all entries to new hash map
    for (i = 0; i < size; i++){
        hash_map_insert_light(map, aux_entries[i]);
    }

    semaphore_init(map, map->arr_size - 1);
}

void hash_map_insert(hash_map_t *map, char *key, char *value){
    // if map size is bigger than size limit then hash map 
    // should be rebuilded

    // printf("BEFORE ENTER %s %d\n", key, map->semaphore);
    semaphore_enter(map);
    // printf("AFTER ENTER %s %d\n", key, map->semaphore);
    if (map->size >= map->size_limit){
        printf("REBUILD\n");
        hash_map_rebuild(map);
    }

    int index = get_hash(key) % map->arr_size;

    pthread_mutex_lock(&map->mutex[index]);
    if (map->entries[index] == NULL){
        map->entries[index] = make_new_entry(key, value);
        map->size++;
    }
    else{
        entry_t *curr_entry = map->entries[index];

        while(curr_entry){
            if (strcmp(curr_entry->key, key) == 0){
                free(curr_entry->value);
                curr_entry->value = (char *)malloc(strlen(value) + 1);
                strcpy(curr_entry->value, value);
                break;
            }
            else if (curr_entry->next == NULL){
                curr_entry->next = make_new_entry(key, value);
                map->size++;
                break;                  
            }
            curr_entry = curr_entry->next;
        }
    }
    pthread_mutex_unlock(&map->mutex[index]);
    semaphore_leave(map);
    // printf("AFTER LEAVE %s %d\n", key, map->semaphore);
}

void hash_map_erase(hash_map_t *map, char *key){
    semaphore_enter(map);
    int index = get_hash(key) % map->arr_size;
    pthread_mutex_lock(&map->mutex[index]);

    entry_t *curr_entry = map->entries[index];
    entry_t **last_entry_pointer = &map->entries[index];

    while(curr_entry){
        if (strcmp(curr_entry->key, key) == 0){
            map->size--;
            *last_entry_pointer = curr_entry->next;
            free(curr_entry->value);
            free(curr_entry->key);
            free(curr_entry);
            return;
        }
        last_entry_pointer = &curr_entry->next;
        curr_entry = curr_entry->next;
    }
    pthread_mutex_unlock(&map->mutex[index]);
    semaphore_leave(map);
}

void hash_map_get(hash_map_t *map, char *key, char *value){
    semaphore_enter(map);
    int index = get_hash(key) % map->arr_size;
    pthread_mutex_lock(&map->mutex[index]);

    entry_t *curr_entry = map->entries[index];

    while(curr_entry){
        if (strcmp(curr_entry->key, key) == 0){
            strcpy(value, curr_entry->value);
            return;
        }
        curr_entry = curr_entry->next;
    }

    strcpy(value, "nil");
    pthread_mutex_unlock(&map->mutex[index]);
    semaphore_leave(map);
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
    ((hash_impl_t *)map->impl)->map = 
        create_hash_map(arr_size, arr_size * LOAD_FACTOR, multiplier);

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
    printf("Size: %d\n", hash_map.size);
    int i;
    for(i = 0; i < hash_map.arr_size; i++){
        entry_t *entry = hash_map.entries[i];
        printf("%d ", i);
        while(entry){
            printf("|%s %s| ", entry->key, entry->value);
            entry = entry->next;
        }
        printf("\n");
    }
}

void prnt(hash_map_t hash_map){

    printf("Size: %d\n", hash_map.size);
    int i;
    for(i = 0; i < hash_map.arr_size; i++){
        entry_t *entry = hash_map.entries[i];
        printf("%d ", i);
        while(entry){
            printf("|%s %s| ", entry->key, entry->value);
            entry = entry->next;
        }
        printf("\n");
    }
}