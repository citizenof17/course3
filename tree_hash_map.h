#pragma once

#include "map_interface.h"

void tree_hash_map_init(map_t *map, int size);
void tree_hash_map_free(map_t *map);
void tree_hash_map_print(map_t *map); //auxiliary function