#include <curses.h> // bool type
#include <errno.h>  // errorno cosntper
#include <signal.h> // singal function
#include <stdio.h>  // format print function
#include <stdlib.h> // exit function
#include <string.h> // strerror function
#include <unistd.h> // sleep function

typedef struct {
  int r;
  int g;
  int b;
} Color;

#define COLOR_INIT {0, 0, 0}

bool loop_running = true;

void break_loop(int sig) { loop_running = false; }

// TODO: 修复宏, 使用类OpenGL的内联宏而不是下面的宏
#define CHECK_FILE_POINTER(POINTER)                                            \
  do {                                                                         \
    if (POINTER == NULL) {                                                     \
      fprintf(stderr, "Open file faliure: %s\n", strerror(errno));             \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define CHECK_FSCANF_NUMBER(NUMBER, MESSAGE)                                   \
  do {                                                                         \
    if (result != NUMBER) {                                                    \
      fprintf(stderr, "[ERROR]: Read %s form file faliure\n", MESSAGE);        \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

int main(int argc, char **argv) {

  signal(SIGINT, break_loop);

  const char *input_path = "./in.fp";
  const char *output_path = "./output.ppm";

  // === get data form file
  FILE *fp = fopen(input_path, "r");
  CHECK_FILE_POINTER(fp);
  FILE *ppm_p = fopen(output_path, "r+");
  CHECK_FILE_POINTER(ppm_p);

  int ppm_width = 0;
  int ppm_height = 0;
  int ppm_max_value = 0;
  Color bef_c = COLOR_INIT;
  Color cur_c = COLOR_INIT;
  size_t result;

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

    char buf[256];
    if (!fgets(buf, 256, fp)) {
      fprintf(stderr, "Open file faliure: %s\n", strerror(errno));
      exit(1);
    }

    // TODO: 增加注释的识别 字符提取使用`fgets+sscanf()`.
    //     result = fscanf(fp, "[%d, %d]\nmax: %d\n", &ppm_width, &ppm_height,
    //                     &ppm_max_value);
    //     CHECK_FSCANF_NUMBER(3, "Meat Data");
    //     result = fscanf(fp, "Color: {%d, %d, %d}", &cur_c.r, &cur_c.g,
    //     &cur_c.b); CHECK_FSCANF_NUMBER(3, "Color Data");

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
