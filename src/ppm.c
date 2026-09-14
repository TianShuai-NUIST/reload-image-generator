#include "ppm.h"

#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "types.h"

bool write_ppm(const char* path, const ImageSpec *spec){
  // 原子读写
  char tmp_path[PATH_MAX];
  snprintf(tmp_path, sizeof(tmp_path), "%s.temp", path);

  FILE *fp = fopen(tmp_path, "wb");
  if(fp == NULL){
    fprintf(stderr, "[ERROR]: (%s) %s\n", path, strerror(errno));
    return false;
  }
  fprintf(fp, "P6\n%d %d\n%d\n", spec->width, spec->height, spec->maxValue);

  size_t row_bytes =(size_t)spec->width * 3;
  unsigned char *row = (unsigned char*) malloc(row_bytes);
  if(row == NULL) goto FP_CLOSE;
  for(int x = 0; x < spec->width; x++){
    row[x * 3 + 0] = (unsigned char)spec->color.r;
    row[x * 3 + 1] = (unsigned char)spec->color.g;
    row[x * 3 + 2] = (unsigned char)spec->color.b;
  }

  for(int y = 0; y < spec->height; y++){
    if(fwrite(row, 1, row_bytes, fp) != row_bytes){
      free(row);
      goto FP_CLOSE;
    }
  }
  free(row);

  if(ferror(fp)) goto FP_CLOSE;
  if(fclose(fp) != 0) goto FP_REMOVE;

  if(rename(tmp_path, path) != 0){
    fprintf(stderr, "[ERROR] rename \"%s\" -> \"%s\": %s\n", tmp_path, path, strerror(errno));
    goto FP_REMOVE;
  }
  return true;

FP_CLOSE:
  fclose(fp);
FP_REMOVE:
  remove(tmp_path);
  return false;
}
