#include <curses.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int r;
  int g;
  int b;
} Color;

#define COLOR_INIT {0, 0, 0}

bool loop_running = true;

void break_loop(int sig) { loop_running = false; }

int main() {

  signal(SIGINT, break_loop);

  const char *input_path = "./in.fp";
  const char *output_path = "./output.ppm";

  // === get data form file
  FILE *fp = fopen(input_path, "r");
  if (fp == NULL) {
    fprintf(stderr, "Open file failue: %s\n", strerror(errno));
    exit(1);
  }

  FILE *ppm_p = fopen(output_path, "r+");
  if (ppm_p == NULL) {
    fprintf(stderr, "Open ppm file failuer: %s\n", strerror(errno));
    exit(1);
  }

  int ppm_width = 0;
  int ppm_height = 0;
  int ppm_max_value = 0;
  Color bef_c = COLOR_INIT;
  Color cur_c = COLOR_INIT;

  size_t result;
  // [50, 50]
  // max: 255
  // Color: {5, 6, 7}

  while (loop_running) {

#ifndef DEBUG
    printf("\r\033[0k");
    printf("\rReading data...");
    fflush(stdout);
#endif

    sleep(1);
    fp = freopen(input_path, "r", fp);
    // ppm_p = freopen(output_path, "w", ppm_p);

    fseek(fp, 0, SEEK_SET);

    result = fscanf(fp, "[%d, %d]\nmax: %d\n", &ppm_width, &ppm_height,
                    &ppm_max_value);
    if (result != 3) {
      fprintf(stderr, "[ERROR]: Read data form file failue %s: %s\n",
              "metadata", strerror(errno));
      exit(1);
    }

    result = fscanf(fp, "Color: {%d, %d, %d}", &cur_c.r, &cur_c.g, &cur_c.b);
    if (result != 3) {
      fprintf(stderr, "[ERROR]: Read data form file failue %s: %s\n",
              "colordata", strerror(errno));
      exit(1);
    }
#ifdef DEBUG
    printf("Reading data...\n");
    printf("matadata: %d %d %d\n", ppm_width, ppm_height, ppm_max_value);
    printf("Color data: %d %d %d\n", cur_c.r, cur_c.g, cur_c.b);
#endif

    if (bef_c.r != cur_c.r || bef_c.g != cur_c.g || bef_c.b != cur_c.b) {
      printf("\n[Info] Change data to {%d, %d, %d}\n", cur_c.r, cur_c.g,
             cur_c.b);
      // === write data into file
      fseek(ppm_p, 0, SEEK_SET);
      fprintf(ppm_p, "P3\n%d %d\n%d\n", ppm_width, ppm_height, ppm_max_value);
      for (int i = 0; i < ppm_height; i++) {
        for (int j = 0; j < ppm_width; j++) {
          fprintf(ppm_p, "%d %d %d ", cur_c.r, cur_c.g, cur_c.b);
        }
        fprintf(ppm_p, "\n");
      }
      fflush(ppm_p);
    }

    bef_c = cur_c;
  }

  // === clean file pointer

  fclose(ppm_p);
  fclose(fp);
  return 0;
}
