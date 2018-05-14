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

typedef struct list_hash_map_t {
    int size; //number of elements
    int arr_size; //size of array
    int size_limit; //max number of elements
    int multiplier; 
    int semaphore;

    entry_t **entries;
    pthread_rwlock_t *rwlock;
    pthread_rwlock_t rwlock_rebuild;
} list_hash_map_t;

list_hash_map_t *create_list_hash_map(int arr_size, int size_limit, int multiplier){
    list_hash_map_t *map = (list_hash_map_t *)malloc(sizeof(list_hash_map_t));
    map->size = 0;
    map->arr_size = arr_size;
    map->size_limit = size_limit;
    map->multiplier = multiplier;
    map->entries = (entry_t **)malloc(arr_size * sizeof(entry_t *));
    map->rwlock = (pthread_rwlock_t *)malloc(arr_size * sizeof(pthread_rwlock_t));
    
    int i;
    for(i = 0; i < map->arr_size; i++){
        pthread_rwlock_init(&map->rwlock[i], NULL);
        map->entries[i] = NULL;
    }
    pthread_rwlock_init(&map->rwlock_rebuild, NULL);
    return map;
}

void create_list_hash_map_light(list_hash_map_t * map, int arr_size, 
                           int size_limit, int multiplier){
    map->size = 0;
    map->arr_size = arr_size;
    map->size_limit = size_limit;
    map->multiplier = multiplier;
    map->entries = (entry_t **)malloc(arr_size * sizeof(entry_t *));
    map->rwlock = (pthread_rwlock_t *)malloc(arr_size * sizeof(pthread_rwlock_t));
    int i;
    for(i = 0; i < map->arr_size; i++){
        pthread_rwlock_init(&map->rwlock[i], NULL);
        map->entries[i] = NULL;
    }
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
    if (entry == NULL){ 
        return; 
    }

    if (entry->next != NULL){
        entry_recursive_free(entry->next);
    }
    free(entry->key);
    free(entry->value);
    // if(entry->next != NULL){
    //     free(entry->next);
    // }
    free(entry);
}

void delete_list_hash_map(list_hash_map_t *map){
    int i;
    // printf("SIZE: %d\n", map->arr_size);
    for(i = 0; i < map->arr_size; i++){
        entry_recursive_free(map->entries[i]);
    }

    free(map->entries);
    free(map->rwlock);
    free(map);
    // printf("%d\n", i);
}

void get_all_entries(entry_t **aux_entries, list_hash_map_t *map){
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
void delete_list_hash_map_light(list_hash_map_t *map){
    int i;
    for(i = 0; i < map->arr_size; i++){
        entry_recursive_free_light(map->entries[i]);
    }

    free(map->rwlock);
    free(map->entries);
}

void list_hash_map_insert_light(list_hash_map_t *map, entry_t *entry){
    int index = get_hash(entry->key) % map->arr_size;
    if (map->entries[index] == NULL){
        map->entries[index] = entry;
    }
    else{
        entry_t *curr_entry = map->entries[index];
        while(curr_entry != NULL){
            if (curr_entry->next == NULL){
                curr_entry->next = entry;
                return;                  
            }
            curr_entry = curr_entry->next;
        } 
        
    }
}

void list_hash_map_rebuild(list_hash_map_t *map){
    // store all entries in map
    entry_t **aux_entries = (entry_t**)malloc(map->size * sizeof(entry_t*));
    int i;

    get_all_entries(aux_entries, map);
    // free old map
    int size = map->size;
    int size_limit = map->size_limit;
    int multiplier = map->multiplier;
    int arr_size = map->arr_size;
    delete_list_hash_map_light(map);
    
    int new_size = arr_size * multiplier;
    create_list_hash_map_light(map, new_size, new_size * LOAD_FACTOR, multiplier);
    map->size = size;

    // add all entries to new hash map
    for (i = 0; i < size; i++){
        list_hash_map_insert_light(map, aux_entries[i]);
    }
    free(aux_entries);
}

void list_hash_map_insert(list_hash_map_t *map, char *key, char *value){
    // if map size is bigger than size limit then hash map 
    // should be rebuilded
    pthread_rwlock_wrlock(&map->rwlock_rebuild);  // VERY BAD
    if (map->size >= map->size_limit){
        list_hash_map_rebuild(map);
    }
    pthread_rwlock_unlock(&map->rwlock_rebuild);

    int index = get_hash(key) % map->arr_size;
    
    pthread_rwlock_rdlock(&map->rwlock_rebuild);   
    pthread_rwlock_wrlock(&map->rwlock[index]);
    if (map->entries[index] == NULL){
        map->entries[index] = make_new_entry(key, value);
        map->size++;
    }
    else{
        entry_t *curr_entry = map->entries[index];

        while(curr_entry){
            if (strcmp(curr_entry->key, key) == 0){
                free(curr_entry->value);
                curr_entry->value = (char *)malloc(strlen(key) + 1);
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
    pthread_rwlock_unlock(&map->rwlock[index]);
    pthread_rwlock_unlock(&map->rwlock_rebuild);   
}

void list_hash_map_erase(list_hash_map_t *map, char *key){
    pthread_rwlock_rdlock(&map->rwlock_rebuild);   
    int index = get_hash(key) % map->arr_size;
    pthread_rwlock_wrlock(&map->rwlock[index]);

    entry_t *curr_entry = map->entries[index];
    entry_t **last_entry_pointer = &map->entries[index];

    while(curr_entry){
        if (strcmp(curr_entry->key, key) == 0){
            map->size--;
            *last_entry_pointer = curr_entry->next;
            free(curr_entry->value);
            free(curr_entry->key);
            free(curr_entry);
            break;
        }
        last_entry_pointer = &curr_entry->next;
        curr_entry = curr_entry->next;
    }
    pthread_rwlock_unlock(&map->rwlock[index]);
    pthread_rwlock_unlock(&map->rwlock_rebuild);   
}

void list_hash_map_get(list_hash_map_t *map, char *key, char *value){
    pthread_rwlock_rdlock(&map->rwlock_rebuild);   
    int index = get_hash(key) % map->arr_size;
    pthread_rwlock_rdlock(&map->rwlock[index]);

    entry_t *curr_entry = map->entries[index];

    char f = 0;
    while(curr_entry){
        if (strcmp(curr_entry->key, key) == 0){
            strcpy(value, curr_entry->value);
            f = 1;
            break;
        }
        curr_entry = curr_entry->next;
    }

    if (!f){
        strcpy(value, "nil");
    }
    pthread_rwlock_unlock(&map->rwlock[index]);
    pthread_rwlock_unlock(&map->rwlock_rebuild);   
}

#define PREPARE_IMPL(map) \
    assert(map); assert(map->impl); \
    hash_impl_t* impl = (hash_impl_t *)map->impl;

typedef struct {
    list_hash_map_t *map;
} hash_impl_t;

static response_t get(map_t *map, char *key){
    PREPARE_IMPL(map)

    response_t response;
    list_hash_map_get(impl->map, key, response.value);
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

    list_hash_map_insert(impl->map, key, value);
    response_t response = {"STORED", ""};
    return response;
}

static response_t rem(map_t *map, char *key){
    PREPARE_IMPL(map)

    list_hash_map_erase(impl->map, key);
    response_t response = {"DELETED", ""};
    return response;
}

void list_hash_map_init(map_t *map, int arr_size, int multiplier){
    map->impl = (hash_impl_t *)malloc(sizeof(hash_impl_t));
    ((hash_impl_t *)map->impl)->map = 
        create_list_hash_map(arr_size, arr_size * LOAD_FACTOR, multiplier);

    map->get = &get;
    map->set = &set;
    map->rem = &rem;
}

void list_hash_map_free(map_t *map){
    PREPARE_IMPL(map)
    delete_list_hash_map(impl->map);
    free(map->impl);
}

void list_hash_map_print(map_t *map){
    PREPARE_IMPL(map)

    list_hash_map_t hash_map = *(impl->map);
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