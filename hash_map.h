#pragma once

#include "map_interface.h"

void hash_map_init(map_t *map, int arr_size, int multiplier);
void hash_map_free(map_t *map);
void hash_map_print(map_t *map);
