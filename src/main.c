#include <errno.h>  // errorno cosntper
#include <signal.h> // singal function
#include <stdio.h>  // format print function
#include <stdlib.h> // exit function
#include <stdbool.h>
#include <string.h> // strerror function
#include <unistd.h> // sleep function
#include <limits.h>
#include <sys/stat.h>

#include "types.h"
#include "utils.h"
#include "config.h"
#include "ppm.h"

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
