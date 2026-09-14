#define _POSIX_C_SOURCE 200809L
// 上面一行, 不能删, 移植性问题, 别问
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "test.h"

static bool parse_text(const char *text, ImageSpec *spec, char* err, size_t n){
  FILE *f = fmemopen((void*)text, strlen(text), "r");
  if(f == NULL){
    snprintf(err, n, "Open text failure!");
    return false;
  }
  bool ok = parse_config(f, spec, err, n);
  fclose(f);
  return ok;
}

// ==================================

static void test_no_space(void){
  const char *text = "[100,50]\nmax:255\nColor:{10,10,10}\n";
  ImageSpec spec; char err[256]; 
  ASSERT_TRUE(parse_text(text, &spec, err, sizeof(err)));
}

static void test_comments_and_blank(void){
  const char *text = "\n\n[100, 50]\n  max: 255\nColor: {10, 10, 10}\n# HelloTestParse\n";
  ImageSpec spec; char err[256]; 
  ASSERT_TRUE(parse_text(text, &spec, err, sizeof(err)));
}

static void test_garbage_line(void){
  const char *text = "[100, 50]\nmax: 255\nColor: {10, 10, 10}\nHelloTestParse";
  ImageSpec spec; char err[256]; 
  ASSERT_FALSE(parse_text(text, &spec, err, sizeof(err)));
}

static void test_empty(void){
  const char *text = "";
  ImageSpec spec; char err[256]; 
  ASSERT_FALSE(parse_text(text, &spec, err, sizeof(err)));
}

static void test_missing_color(void){
  const char *text = "[100, 50]\nmax: 255\n";
  ImageSpec spec; char err[256]; 
  ASSERT_FALSE(parse_text(text, &spec, err, sizeof(err)));
}

static void test_size_zero(void){
  const char *text = "[0, 50]\nmax: 255\nColor : {0, 0, 0}\n";
  ImageSpec spec; char err[256]; 
  ASSERT_FALSE(parse_text(text, &spec, err, sizeof(err)));
}

static void test_color_negative(void){
  const char *text = "[100, 50]\nmax: 255\nColor : {-1, -1, -1}\n";
  ImageSpec spec; char err[256]; 
  ASSERT_FALSE(parse_text(text, &spec, err, sizeof(err)));
}

static void test_color_256(void){
  const char *text = "[100, 50]\nmax: 255\nColor : {256, 256, 256}\n";
  ImageSpec spec; char err[256]; 
  ASSERT_FALSE(parse_text(text, &spec, err, sizeof(err)));
}

static void test_zero_color(void){
  const char *text = "[100, 50]\nmax: 255\nColor : {0, 0, 0}\n";
  ImageSpec spec; char err[256]; 
  ASSERT_TRUE(parse_text(text, &spec, err, sizeof(err)));
}

static void test_basic(void){
  const char *text = "[100, 50]\nmax: 255\nColor : {56, 10, 59}\n";
  ImageSpec spec;
  char err[256];
  ASSERT_TRUE(parse_text(text, &spec, err, sizeof err));
  ASSERT_EQ_INT(spec.width    , 100);
  ASSERT_EQ_INT(spec.height   , 50);
  ASSERT_EQ_INT(spec.maxValue , 255);
  ASSERT_EQ_INT(spec.color.r  , 56);
  ASSERT_EQ_INT(spec.color.g  , 10);
  ASSERT_EQ_INT(spec.color.b  , 59);
}

int main(void){
  test_zero_color();
  test_color_256();
  test_size_zero();
  test_color_negative();
  test_missing_color();
  test_empty();
  test_garbage_line();
  test_comments_and_blank();
  test_no_space();
  test_basic();

  TEST_SUMMARY();
  return 0;
}
