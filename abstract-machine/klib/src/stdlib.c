#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

static uintptr_t last_allocated_addr = 0;
#define ALIGNMENT 8  // 假设要求8字节对齐

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

// 整数转换字符串
char *itoa(int num) {
    static char str[32];  // 足够容纳最大的整数和负号
    int i = 0, is_negative = 0;
    
    // 处理0这个特殊情况
    if (num == 0) {
        str[i++] = '0';
    }

    // 处理负数
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // 转换过程
    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    // 如果是负数，要加负号
    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';  // 设置字符串终点

    // 因为转换时候是从高到低的，所以要反转过来
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    return str;
}

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  //  uintptr_t addr = (uintptr_t) heap.start;
  // // 计算对齐后的地址
  // uintptr_t malloc_addr = (addr + size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
  
#endif
  if (size == 0) {
      return NULL;  // 处理特殊情况
  }

  // 如果是第一次调用 malloc，初始化 last_allocated_addr
  if (last_allocated_addr == 0) {
      last_allocated_addr = (uintptr_t) heap.start;
  }

  // 计算对齐后的地址
  uintptr_t aligned_addr = (last_allocated_addr + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

  uintptr_t old_addr = aligned_addr;

  // 更新 last_allocated_addr 为下一个可用地址
  last_allocated_addr = aligned_addr + size;

  // 检查是否超出堆的范围
  if (last_allocated_addr > (uintptr_t) heap.end) {
      // 堆空间不足，返回 NULL
      return NULL;
  }

  // 返回对齐后的地址
  return (void*)old_addr;
}

void free(void *ptr) {
}

#endif
