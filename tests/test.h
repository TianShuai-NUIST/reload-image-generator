#ifndef TEST_H
#define TEST_H

#include <stdio.h>

static int test_total    = 0;
static int test_failures = 0;

#define ASSERT_TRUE(cond)\
do{\
  test_total++;\
  if(!(cond)){\
    test_failures++;\
    printf("[FAIL] %s:%d %s\n", __FILE__, __LINE__, #cond);\
  }\
}while(0)

#define ASSERT_FALSE(cond)\
do{\
  test_total++;\
  if((cond)){\
    test_failures++;\
    printf("[FAIL] %s:%d %s\n", __FILE__, __LINE__, #cond);\
  }\
}while(0)

#define ASSERT_EQ_INT(a, b)\
do{\
  test_total++;\
  long _a = (a);\
  long _b = (b);\
  if(_a != _b){\
    test_failures++;\
    printf("[FAIL] %s:%d (%s) %ld != %ld\n", __FILE__, __LINE__, #a " == " #b, _a, _b);\
  }\
}while(0)

#define TEST_SUMMARY()\
do{\
  printf("%d test, %d failure.\n", test_total, test_failures);\
  return test_failures == 0 ? 0 : 1;\
}while(0)

#endif
