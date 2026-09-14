#include "config.h"
#include "types.h"

#include <stddef.h>
#include <stdio.h>

bool parse_config(FILE *fp, ImageSpec *out, char* err, size_t err_size){
  char line[256];
  int line_no = 0;
  bool has_size = false;
  bool has_max = false;
  bool has_color = false;

  while(fgets(line, sizeof(line), fp) != NULL){
    line_no++;
    char *p = line;
    while(*p == ' ' || *p == '\t') p++;
    if(p[0] == '#' || p[0] == '\n' || p[0] == ' ') continue;

    int w, h, m, r, g, b, n = 0;

    if(sscanf(p, "[%d , %d ]%n", &w, &h, &n) == 2){
      if(w > 0 && h > 0 && w < MAX_SIZE && h < MAX_SIZE){
        has_size = true;
        out->height = h;
        out->width = w;
      }else{
        snprintf(err, err_size, "%d : width or height in 1..!\n %s", line_no, p);
        return false;
      }
    }else if(sscanf(p, "max : %d%n", &m, &n) == 1){
      if(m > 0 && m < 256){
        has_max = true;
        out->maxValue = m;
      }else{
        snprintf(err, err_size, "%d : max value in 1..255!\n %s", line_no, p);
        return false;
      }
    }else if(sscanf(p, "Color : { %d , %d , %d }%n", &r, &g, &b, &n) == 3){
      if(r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255){
        out->color = (Color){ .r = r, .g = g, .b = b };
        has_color = true;
      }else{
        snprintf(err, err_size, "%d : color(rgb) in 0..255!\n %s", line_no, p);
        return false;
      }
    }else{
      snprintf(err, err_size, "%d: %s", line_no, p);
      return false;
    }
    char *rest = p + n;
    if(!(*rest == '\0' || *rest == '\n' || *rest == '#')){
      snprintf(err, err_size, "%d: %s", line_no, p);
      return false;
    }
  }
  if(!has_size){
    snprintf(err, err_size, "miss argument with Size!");
    return false;
  }
  if(!has_max){
    snprintf(err, err_size, "miss argument with Max Value!");
    return false;
  }
  if(!has_color){
    snprintf(err, err_size, "miss argument with Color!");
    return false;
  }
  return true;
}
