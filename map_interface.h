#pragma once

#include "protocol.h"

typedef struct map_t {
    response_t (*set) (struct map_t *, char *, char *);
    response_t (*get) (struct map_t *, char *);
    response_t (*rem) (struct map_t *, char *);
    void *impl;
} map_t;