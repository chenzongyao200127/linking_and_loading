// By GPT-4

#define _GNU_SOURCE // 启用 GNU 特定的编译器特性和扩展
#include <dlfcn.h>  // 包含动态链接函数的定义
#include <stdio.h>  // 包含标准输入输出函数的定义
#include <stdlib.h> // 包含标准库函数的定义

void *malloc(size_t size)
{
  static void *(*real_malloc)(size_t) = NULL; // 定义一个指向真正的 malloc 函数的指针
  if (!real_malloc)
  {
    // 如果 real_malloc 未初始化，则通过 dlsym 函数动态获取原生 malloc 函数的地址
    real_malloc = dlsym(RTLD_NEXT, "malloc");
  }

  void *ptr = real_malloc(size);                    // 调用真正的 malloc 函数分配内存
  fprintf(stderr, "malloc(%zu) = %p\n", size, ptr); // 将分配的大小和返回的指针打印到标准错误
  return ptr;                                       // 返回分配的内存指针
}

void free(void *ptr)
{
  static void (*real_free)(void *) = NULL; // 定义一个指向真正的 free 函数的指针
  if (!real_free)
  {
    // 如果 real_free 未初始化，则通过 dlsym 函数动态获取原生 free 函数的地址
    real_free = dlsym(RTLD_NEXT, "free");
  }

  fprintf(stderr, "free(%p)\n", ptr); // 将释放的内存指针打印到标准错误
  real_free(ptr);                     // 调用真正的 free 函数释放内存
}
