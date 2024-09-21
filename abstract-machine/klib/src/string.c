#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) { // 计算字符串的长度
  size_t slen = 0;
  while (s[slen] != '\0')
  {
      slen++;
  }
  return slen;
}

char *strcpy(char *dst, const char *src) {  // 将 src 复制到 dst
  int i = 0;
  for (; src[i] != '\0'; i++)
  {
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) { // 将 src 复制到 dst，最多复制 n 个字节
  int i = 0;
  for (; src[i] != '\0' && i < n; i++)
  {
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {  // 将 src 复制到 dst 末尾
  int dst_len = 0;
  while (dst[dst_len] != '\0')
  {
    dst_len++;
  }
  int i = 0;
  for (; src[i] != '\0'; i++)
  {
    dst[dst_len++] = src[i];
  }
  dst[dst_len] = '\0';
  return dst;
}

int strcmp(const char* s1, const char* s2) // 一次递减字符串的长度， 直到字符不一样或者到末尾了 
{
	int ret = 0;
	while(!(ret=*(unsigned char*)s1-*(unsigned char*)s2) && *s1)
	{
		s1++;
		s2++;
	}
 
 
	if (ret < 0)
	{
		return -1;
	}
	else if (ret > 0)
	{
		return 1;
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) { // 比较前 n 个
    int ret = 0;
    while(!(ret=*(unsigned char*)s1-*(unsigned char*)s2) && *s1)
    {
        s1++;
        s2++;
        n--;
        if (n == 0) break;
    }
    if (ret < 0)
    {
        return -1;
    }
    else if (ret > 0)
    {
        return 1;
    }
    return 0;
}

// 将 s 处 n 个内存的数据 设置为 c
void *memset(void *s, int c, size_t n) { // unsigned char
  char *cdst = (char *) s;
  int i = 0;
  for (; i < n; i++)
  {
    cdst[i] = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) { // 在重叠区域移动内存块
  const char *s;
  char *d;

  s = (const char*) src;
  d = (char*)dst;
  if(s < d && s + n > d){ // 如果重叠了，从后往前复制
    s += n;
    d += n;
    while(n-- > 0)
      *--d = *--s;
  } else  // 不重叠，从前往后
    while(n-- > 0)
      *d++ = *s++;

  return dst;
}

void *memcpy(void *out, const void *in, size_t n) { // 复制内存块
  return memmove(out, in, n);
}

int memcmp(const void *s1, const void *s2, size_t n) {  // 比较内存块
  const unsigned char * a1;
  const unsigned char * a2;
  a1 = s1;
  a2 = s2;
  while (n-- > 0)
  {
    if (*a1 != *a2)
    {
      return *a1 - *a2;
    }
    a1++;
    a2++;
  }
  return 0;
}

#endif
