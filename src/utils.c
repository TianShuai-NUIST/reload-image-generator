#include "utils.h"

#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/inotify.h>
#include <poll.h> // poll function to get event
#include <stdalign.h> // get align of type
#include <unistd.h> // sleep function

volatile sig_atomic_t g_running     = 1;
volatile sig_atomic_t g_last_signal = 0;

void handle_signal(int sig){
  g_last_signal = sig;
  g_running = 0;
}

bool spec_equal(const ImageSpec *a, const ImageSpec *b){
  if(a->width != b->width)       return false;
  if(a->height != b->height)     return false;
  if(a->maxValue != b->maxValue) return false;
  if(a->color.r != b->color.r)   return false;
  if(a->color.g != b->color.g)   return false;
  if(a->color.b != b->color.b)   return false;
  return true;
}

int init_inotify(char *input_path_dir, char *input_path_base){
  int ifd = inotify_init1(IN_CLOEXEC);
  if(ifd < 0){
    perror("inotify_init1");
    exit(EXIT_FAILURE);
  }
  int wd;
  if((wd = inotify_add_watch(ifd, input_path_dir, IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE))< 0){
    perror("inotify_add_watch");
    exit(EXIT_FAILURE);
  }
  printf("[INFO]: watch \"%s\" in \"%s\"\n", input_path_base, input_path_dir);
  return ifd;
}

bool has_require_event(const int ifd, struct pollfd pfd, const int result_poll, unsigned int *error_times){
  if(result_poll > 0 ){
    if(pfd.revents & POLLIN){
      // 按照struct inotify_event 类型分配内存
      // 我不想成为老古董, 所以这里我使用了C11的语法, 而不是非GNU扩展, 这个玩意只需要知道就可以了, 太丑陋了.
      // char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
      alignas(struct inotify_event) char buf[4096];
      ssize_t n = read(ifd, buf, sizeof buf);
      if(n < 0){
        if(errno != EINTR){
          *error_times += 1;
          perror("read(inotify)");
        }
        return false;
      }
      // 重要, 用于避免编译器自动乘以类型大小, 这样我们的内存可以正常的步进
      char *p = buf;
      while(p < buf + n){
        struct inotify_event *cur = (struct inotify_event *)p;
        printf("[Event] mask=0x%x len=%u name=\"%s\"\n",
               cur->mask, cur->len, cur->len > 0 ? cur->name : "");
        p += sizeof(struct inotify_event) + cur->len;
      }
    }else if(pfd.revents & POLLERR || pfd.revents & POLLHUP || pfd.revents & POLLNVAL){
      //TODO: Check event of poll
      *error_times += 1;
      fprintf(stderr, "[ERROR]: poll events get error! (revents :%d)\n", pfd.revents);
    }
  }else if(result_poll == 0){
    // do nothing... (goto stat)
    return true;
  }else if(result_poll == -1){
    if(errno != EINTR){
      perror("poll(events)");
      *error_times += 1;
    }
    return false;
  }
  return false;
}

bool stat_equal(const char * input_path, struct stat *s, unsigned int *error_times){
  struct stat st;
  if(stat(input_path, &st) == 0){
    if(st.st_size == s->st_size&&
      st.st_mtim.tv_sec == s->st_mtim.tv_sec &&
      st.st_mtim.tv_nsec == s->st_mtim.tv_nsec
    ){
      *error_times = 0;
      // continue;
      return true;
    }
    s->st_mtim = st.st_mtim;
    s->st_size = st.st_size;
    return false;
  }else{
    return false;
  }
}

void get_dir_base(const char *input_file_path, char *dir, char *base){
  // slash 指向最后一个'/'的位置, 以此为界, 前面是dir, 后面是base
  const char *slash = strrchr(input_file_path, '/');
  if(slash != NULL){
    // ./in.fp
    // /home/.../in.fp
    // 最后一个'/'与开头的距离就是dir 
    size_t n = (size_t)(slash - input_file_path);
    n = n == 0 ? 1 : n;
      assert(n < PATH_MAX);
    memcpy(dir, input_file_path, n);
    dir[n] = '\0';

    snprintf(base, sizeof(base), "%s", (slash + 1));
  }else{
    // 输入字符串没有'/', 所以只能是in.fp这种
    snprintf(dir, sizeof(dir), ".");
    snprintf(base, sizeof(base), "%s", input_file_path);
  }
}
