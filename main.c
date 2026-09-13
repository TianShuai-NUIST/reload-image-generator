#include <errno.h>  // errorno cosntper
#include <signal.h> // singal function
#include <stdio.h>  // format print function
#include <stdlib.h> // exit function
#include <stdbool.h>
#include <string.h> // strerror function
#include <unistd.h> // sleep function
#include <limits.h>
#include <sys/stat.h>

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

static bool spec_equal(const ImageSpec *a, const ImageSpec *b){
  if(a->width != b->width)       return false;
  if(a->height != b->height)     return false;
  if(a->maxValue != b->maxValue) return false;
  if(a->color.r != b->color.r)   return false;
  if(a->color.g != b->color.g)   return false;
  if(a->color.b != b->color.b)   return false;
  return true;
}

static bool write_ppm(const char* path, const ImageSpec *spec){
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

int main(int argc, char **argv) {
  setvbuf(stdout, NULL, _IOLBF, 0);

  if (argc > 3) {
    fprintf(stderr, "usage: %s [input.fp] [output.ppm]\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *input_path = argc > 1 ? argv[1] : "./in.fp";
  const char *output_path = argc > 2 ? argv[2] : "./output.ppm";

  struct sigaction sa;
  sa.sa_handler = handle_signal;
  sigemptyset(&sa.sa_mask);
  // 不使用 SA_RESTART:让 sleep() 被信号打断后能尽快检查标志
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  struct timespec last_mtime = {0, 0};
  // 首次无条件执行
  off_t last_size = -1;

  ImageSpec prev = {0};
  // 首次无条件执行
  bool first_frame = true;
  while(g_running) {
    sleep(1);
    struct stat st;
    if(stat(input_path, &st) == 0){
      if(st.st_size == last_size &&
        st.st_mtim.tv_sec == last_mtime.tv_sec &&
        st.st_mtim.tv_nsec == last_mtime.tv_nsec
      ){
        continue;
      }
      last_mtime = st.st_mtim;
      last_size = st.st_size;
    }

    char err[256];
    ImageSpec spec;

    FILE *fp = fopen(input_path, "r");
    if (fp == NULL) {
      fprintf(stderr, "Open \"%s\" faliure %d: %s\n", input_path, __LINE__, strerror(errno));
      continue;
    }

    bool ok = parse_config(fp, &spec, err, sizeof(err));
    fclose(fp);

    if(!ok){
      fprintf(stderr, "[ERROR]: %s\n", err);
      continue;
    }

    if(first_frame || !spec_equal(&spec, &prev)){
      if(write_ppm(output_path, &spec)){
        printf("[Info] %dx%d max %d color {%d, %d, %d}\n",
               spec.width, spec.height, spec.maxValue,
               spec.color.r, spec.color.g, spec.color.b);
        prev = spec;
        first_frame = false;
      }else{
        fprintf(stderr, "[ERROR] write \"%s\" failed\n", output_path);
      }
    }
  }
  printf("[Info] exit on signal %d\n", (int)g_last_signal);

  return 0;
}
