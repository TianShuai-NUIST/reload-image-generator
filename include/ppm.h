#ifndef PPM_H
#define PPM_H

#include <stdbool.h>

#include "types.h"

bool write_ppm(const char* path, const ImageSpec *spec);

#endif
