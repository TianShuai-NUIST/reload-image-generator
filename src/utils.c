#include "utils.h"

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
