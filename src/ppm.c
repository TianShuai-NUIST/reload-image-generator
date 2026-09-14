#include "ppm.h"

#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "types.h"

bool write_ppm(const char* path, const ImageSpec *spec){
  // 原子读写
  char tmp_path[PATH_MAX];
  snprintf(tmp_path, sizeof(tmp_path), "%s.temp", path);

  FILE *fp = fopen(tmp_path, "w");
  if(fp == NULL){
    fprintf(stderr, "[ERROR]: (%s) %s\n", path, strerror(errno));
    return false;
  }
  fprintf(fp, "P3\n%d %d\n%d\n", spec->width, spec->height, spec->maxValue);
  for (int i = 0; i < spec->height; i++) {
    for (int j = 0; j < spec->width; j++) {
      fprintf(fp, "%d %d %d ", spec->color.r, spec->color.g, spec->color.b);
    }
    fprintf(fp, "\n");
  }
  fflush(fp);
  if(ferror(fp)){
    fclose(fp);
    remove(tmp_path);
    return false;
  }
  if(fclose(fp) != 0){
    remove(tmp_path);
    return false;
  }

  if(rename(tmp_path, path) != 0){
    fprintf(stderr, "[ERROR] rename \"%s\" -> \"%s\": %s\n", tmp_path, path, strerror(errno));
    remove(tmp_path);
    return false;
  }
  return true;
}
