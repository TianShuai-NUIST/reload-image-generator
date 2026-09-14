#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <signal.h>

#include "types.h"

// 编译器一定要去内存读取, 而不是读取缓存中的值
// 使用sig_atomic_t表示原子读写的信号类型
extern volatile sig_atomic_t g_running;
extern volatile sig_atomic_t g_last_signal;

void handle_signal(int sig);
bool spec_equal(const ImageSpec *a, const ImageSpec *b);

#endif
