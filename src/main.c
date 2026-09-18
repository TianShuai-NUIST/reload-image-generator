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
#include <stdalign.h> // get align of type
#include <poll.h> // poll function to get event

#include "types.h"
#include "utils.h"
#include "config.h"
#include "ppm.h"

// 超过 ERROR_MAX 次数就退出
#define ERROR_MAX 5

int main(int argc, char **argv) {
  setvbuf(stdout, NULL, _IOLBF, 0);

  if (argc > 3) {
    fprintf(stderr, "usage: %s [input.fp] [output.ppm]\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *input_path = argc > 1 ? argv[1] : "./in.fp";
  const char *output_path = argc > 2 ? argv[2] : "./output.ppm";

  char input_path_dir[PATH_MAX];
  char input_path_base[NAME_MAX];
  // slash 指向最后一个'/'的位置, 以此为界, 前面是dir, 后面是base
  const char *slash = strrchr(input_path, '/');
  if(slash != NULL){
    // ./in.fp
    // /home/.../in.fp
    // 最后一个'/'与开头的距离就是dir 
    size_t n = (size_t)(slash - input_path);
    n = n == 0 ? 1 : n;
      assert(n < PATH_MAX);
    memcpy(input_path_dir, input_path, n);
    input_path_dir[n] = '\0';

    snprintf(input_path_base, sizeof(input_path_base), "%s", (slash + 1));
  }else{
    // 输入字符串没有'/', 所以只能是in.fp这种
    snprintf(input_path_dir, sizeof(input_path_dir), ".");
    snprintf(input_path_base, sizeof(input_path_base), "%s", input_path);
  }

  int ifd = inotify_init1(IN_CLOEXEC);
  if(ifd < 0){
    perror("inotify_init1");
    return EXIT_FAILURE;
  }
  int wd;
  if((wd = inotify_add_watch(ifd, input_path_dir, IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE))< 0){
    perror("inotify_add_watch");
    return EXIT_FAILURE;
  }
  printf("[INFO]: watch \"%s\" in \"%s\"\n", input_path_base, input_path_dir);

  struct sigaction sa;
  sa.sa_handler = handle_signal;
  sigemptyset(&sa.sa_mask);
  // 不使用 SA_RESTART:让 sleep() 被信号打断后能尽快检查标志
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  struct timespec last_mtime = {0, 0};
  off_t last_size = -1; // 首次无条件执行
  ImageSpec prev = {0};
  bool first_frame = true; // 首次无条件执行
  unsigned int error_times = 0;

  while(g_running) {
    // sleep(1);
    if(error_times > ERROR_MAX){
      fprintf(stderr, "error times to much: %d!\n", error_times);
      exit(EXIT_FAILURE);
    }

    struct pollfd pfd = {
      .fd = ifd,
      .events = POLLIN
    };
    int result_poll = poll(&pfd, 1, 1000);

    if(result_poll > 0 ){
      if(pfd.revents & POLLIN){
        alignas(struct inotify_event) char buf[4096];
        ssize_t n = read(ifd, buf, sizeof buf);
        if(n < 0){
          if(errno != EINTR){
            error_times++;
            perror("read(inotify)");
          }
          continue;
        }
        char *p = buf;
        while(p < buf + n){
          struct inotify_event *cur = (struct inotify_event *)p;
          printf("[Event] mask=0x%x len=%u name=\"%s\"\n",
                 cur->mask, cur->len, cur->len > 0 ? cur->name : "");
          p += sizeof(struct inotify_event) + cur->len;
        }
      }else if(pfd.revents & POLLERR || pfd.revents & POLLHUP || pfd.revents & POLLNVAL){
        // Check event of poll
        error_times++;
        perror("poll(events)");
        fprintf(stderr, "[ERROR]: poll evnets get error! (revents :%d)\n", pfd.revents);
      }
    }else if(result_poll == 0){
      // do nothing... (goto stat)
    }else if(result_poll == -1){
      if(errno != EINTR){
        perror("poll(events)");
        error_times++;
      }
      continue;
    }

    // 我不想成为老古董, 所以这里我使用了C11的语法, 而不是非GNU扩展, 这个玩意只需要知道就可以了, 太丑陋了.
    // char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
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
