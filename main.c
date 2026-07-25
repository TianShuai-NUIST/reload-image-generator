#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int r;
  int g;
  int b;
} Color;

#define COLOR_INIT {0, 0, 0}

int main() {
  // === get data form file
  FILE *fp = fopen("./in.fp", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Open file failue: %s\n", strerror(errno));
    exit(1);
  }

  int ppm_width = 0;
  int ppm_height = 0;
  int ppm_max_value = 0;
  Color c = COLOR_INIT;

  size_t result;
  // [50, 50]
  // max: 255
  // Color: {5, 6, 7}
  result = fscanf(fp, "[%d, %d]\nmax: %d\n", &ppm_width, &ppm_height,
                  &ppm_max_value);
  if (result != 3) {
    fprintf(stderr, "[ERROR]: Read data form file failue %s: %s\n", "metadata",
            strerror(errno));
    exit(1);
  }

  result = fscanf(fp, "Color: {%d, %d, %d}", &c.r, &c.g, &c.b);
  if (result != 3) {
    fprintf(stderr, "[ERROR]: Read data form file failue %s: %s\n", "colordata",
            strerror(errno));
    exit(1);
  }

  printf("%p %p %p\n", &c.r, &c.g, &c.b);
  printf("%d %d %d\n", c.r, c.g, c.b);

  // === write data into file

  FILE *ppm_p = fopen("./output.ppm", "wb");
  if (ppm_p == NULL) {
    fprintf(stderr, "Open ppm file failuer: %s\n", strerror(errno));
    exit(1);
  }

  fprintf(ppm_p, "P3\n%d %d\n%d\n", ppm_width, ppm_height, ppm_max_value);
  for (int i = 0; i < ppm_height; i++) {
    for (int j = 0; j < ppm_width; j++) {
      fprintf(ppm_p, "%d %d %d ", c.r, c.g, c.b);
    }
    fprintf(ppm_p, "\n");
  }

  // === clean file pointer

  fclose(ppm_p);
  fclose(fp);
  return 0;
}
