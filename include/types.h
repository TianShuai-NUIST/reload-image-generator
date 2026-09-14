#ifndef TYPES_H
#define TYPES_H

typedef struct Color {
  int r;
  int g;
  int b;
} Color;

typedef struct ImageSpec {
  int width;
  int height;
  int maxValue;
  Color color;
} ImageSpec;

#define MAX_SIZE 8192

#endif
