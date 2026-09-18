#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <signal.h>
#include <sys/stat.h>
#include <poll.h>

#include "types.h"

// 编译器一定要去内存读取, 而不是读取缓存中的值
// 使用sig_atomic_t表示原子读写的信号类型
extern volatile sig_atomic_t g_running;
extern volatile sig_atomic_t g_last_signal;

void handle_signal(int sig);

int init_inotify(char *input_path_dir, char *input_path_base);
bool has_require_event(const int ifd, struct pollfd pfd, const int result_poll, unsigned int *error_times);

bool spec_equal(const ImageSpec *a, const ImageSpec *b);
bool stat_equal(const char * input_path, struct stat *s, unsigned int *error_times);

void get_dir_base(const char *input_file_path, char *dir, char *base);

#endif
