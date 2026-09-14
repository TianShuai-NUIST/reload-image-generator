#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdio.h>

#include "types.h"

bool parse_config(FILE *fp, ImageSpec *out, char* err, size_t err_size);

#endif
