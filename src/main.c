#include <errno.h>  // errorno cosntper
#include <signal.h> // singal function
#include <stdio.h>  // format print function
#include <stdlib.h> // exit function
#include <string.h> // strerror function
#include <unistd.h> // sleep function
#include <sys/stat.h> // struct stat
#include <sys/inotify.h>
#include <limits.h>
#include <assert.h> 
#include <poll.h> // poll function to get event

#include "types.h"
#include "utils.h"
#include "config.h"
#include "ppm.h"

// 超过 ERROR_MAX 次数就退出
#define ERROR_MAX 5

void init(){
  setvbuf(stdout, NULL, _IOLBF, 0);

  // 注册信号
  struct sigaction sa;
  sa.sa_handler = handle_signal;
  sigemptyset(&sa.sa_mask);
  // 不使用 SA_RESTART:让 sleep() 被信号打断后能尽快检查标志
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char **argv) {
  init();

  if (argc > 3) {
    fprintf(stderr, "usage: %s [input.fp] [output.ppm]\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *input_path = argc > 1 ? argv[1] : "./in.fp";
  const char *output_path = argc > 2 ? argv[2] : "./output.ppm";
  char input_path_dir[PATH_MAX];
  char input_path_base[NAME_MAX];

  get_dir_base(input_path, input_path_dir, input_path_base);

  int ifd = init_inotify(input_path_dir, input_path_base);

  struct stat last_st = {
    .st_mtim = {0, 0},
    .st_size = -1 // 首次无条件执行
  };
  ImageSpec prev = {0};
  bool first_frame = true; // 首次无条件执行
  // 只有环境问题才会触发计数, 解析问题不计
  unsigned int error_times = 0;
  struct pollfd pfd = {
    .fd = ifd,
    .events = POLLIN
  };

  while(g_running) {
    // sleep(1);
    if(error_times > ERROR_MAX){
      fprintf(stderr, "连续错误次数过多: %d!\n", error_times);
      exit(EXIT_FAILURE);
    }

    int result_poll = poll(&pfd, 1, 1000);
    if(!has_require_event(ifd, pfd, result_poll, &error_times)) continue;

    if(stat_equal(input_path, &last_st , &error_times)) continue;

    char err[256];
    ImageSpec spec;

    FILE *fp = fopen(input_path, "r");
    if (fp == NULL) {
      fprintf(stderr, "Open \"%s\" faliure %d: %s\n", input_path, __LINE__, strerror(errno));
      error_times = 0; // 文件打开失败没关系, 这不是运行系统的问题, 所以错误次数不算
      continue;
    }

    bool ok = parse_config(fp, &spec, err, sizeof(err));
    fclose(fp);

    if(!ok){
      fprintf(stderr, "[ERROR]: %s\n", err);
      error_times = 0; // 不是环境问题
      continue;
    }

    if(first_frame || !spec_equal(&spec, &prev)){
      if(write_ppm(output_path, &spec)){
        printf("[INFO]: %dx%d max %d color {%d, %d, %d}\n",
               spec.width, spec.height, spec.maxValue,
               spec.color.r, spec.color.g, spec.color.b);
        prev = spec;
        first_frame = false;
      }else{
        fprintf(stderr, "[ERROR] write \"%s\" failed\n", output_path);
      }
    }

    error_times = 0;
  }
  printf("[INFO]: exit on signal %d\n", (int)g_last_signal);

  return 0;
}
