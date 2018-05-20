#pragma once

#include "map_interface.h"

// operations for using a list_hash_map
void list_hash_map_init(map_t *map, int arr_size, int multiplier);
void list_hash_map_free(map_t *map);
void list_hash_map_print(map_t *map);
