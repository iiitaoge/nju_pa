#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) //打印到屏幕
{
    va_list args;
    va_start(args, fmt);
    
    while (*fmt) 
    {  // 遍历整个格式字符串
        if (*fmt == '%') 
        {  // 处理格式化输出
            fmt++;
            if (*fmt == 'd') 
            {
                int i = va_arg(args, int); // 获取第一个整数
                char *num_str;  // 假设数字不会超过20位
                num_str = itoa(i);  // 将整数转换为字符串
                for (char *p = num_str; *p != '\0'; p++) 
                {
                    putch(*p);
                }
            } 
            else if (*fmt == 's') 
            {
                char *s = va_arg(args, char *);  // 获取字符串参数
                for (; *s != '\0'; s++) 
                {
                    putch(*s);
                }
            }
            else if (*fmt == 'c')
            {
                int c = va_arg(args, int);  // 使用 int 而不是 char
                putch((char)c);
            }
            else 
            {
                putch(*fmt);  // 直接输出非格式化字符
            }
        } 
        else 
        {
            putch(*fmt);  // 直接输出普通字符
        }
        fmt++;  // 移动到下一个字符
    }
    
    va_end(args);
    return 0;
}


int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) 
{
  va_list args;
  va_start(args, fmt);
  char *str = out;
  
  while (*fmt) 
  {
      if (*fmt == '%') 
      {
          fmt++;
          if (*fmt == 'd') 
          {
              int i = va_arg(args, int);
              char *num_str;
              num_str = itoa(i); 
              for (char *p = num_str; *p != '\0'; p++) 
              {
                  *str++ = *p;
              }
          } 
          else if (*fmt == 's') 
          {
              char *s = va_arg(args, char*);
              while (*s) 
              {
                  *str++ = *s++;
              }
          }
      } 
      else 
      {
          *str++ = *fmt;
      }
      fmt++;
  }
  
  *str = '\0';  
  va_end(args);
  return (str - out);  
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
