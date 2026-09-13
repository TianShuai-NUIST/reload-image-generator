#include <errno.h>  // errorno cosntper
#include <signal.h> // singal function
#include <stdio.h>  // format print function
#include <stdlib.h> // exit function
#include <stdbool.h>
#include <string.h> // strerror function
#include <unistd.h> // sleep function

#define MAX_SIZE 8192

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

#define COLOR_INIT {0, 0, 0}

// 编译器一定要去内存读取, 而不是读取缓存中的值
// 使用sig_atomic_t表示原子读写的信号类型
static volatile sig_atomic_t g_running = 1;
static volatile sig_atomic_t g_last_signal = 0;

static void handle_signal(int sig){
  g_last_signal = sig;
  g_running = 0;
}

static bool parse_config(FILE *fp, ImageSpec *out, char* err, size_t err_size){
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

int main() {
  struct sigaction sa;
  sa.sa_handler = handle_signal;
  sigemptyset(&sa.sa_mask);
  // 不使用 SA_RESTART:让 sleep() 被信号打断后能尽快检查标志
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  const char *input_path = "./in.fp";
  const char *output_path = "./output.ppm";

  // === get data form file
  FILE *fp = fopen(input_path, "r");
  if (fp == NULL) {
    fprintf(stderr, "Open \"%s\" faliure %d: %s\n", input_path, __LINE__, strerror(errno));
    exit(1);
  }
  FILE *ppm_p = fopen(output_path, "w+");
  if (ppm_p == NULL) {
    fprintf(stderr, "Open \"%s\" faliure %d: %s\n", output_path, __LINE__, strerror(errno));
    exit(1);
  }

  int ppm_width     = 0;
  int ppm_height    = 0;
  int ppm_max_value = 0;
  Color bef_c       = COLOR_INIT;
  Color cur_c       = COLOR_INIT;

  while(g_running) {

#ifndef DEBUG
    printf("\r\033[0k");
    printf("\rReading data...\n");
    fflush(stdout);
#endif

    sleep(1);
    fp = freopen(input_path, "r", fp);

    char err[256];
    ImageSpec spac;
    if(!parse_config(fp, &spac, err, sizeof(err))){
      fprintf(stderr, "[ERROR]: %s\n", err);
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
