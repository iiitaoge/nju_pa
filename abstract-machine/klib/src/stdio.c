#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// 将指针转换为十六进制字符串
char *itoa_hex(unsigned long value, char *buffer) {
    const char *hex_digits = "0123456789abcdef";
    char *ptr = buffer;
    
    // 指针通常是64位，处理方式也适用于32位系统
    for (int i = sizeof(value) * 2 - 1; i >= 0; i--) {
        *ptr++ = hex_digits[(value >> (i * 4)) & 0xF]; // 提取每4位（1个16进制位）
    }
    *ptr = '\0';
    return buffer;
}

// 十进制转十六进制字符串
void dec_to_hex(int decimal, char *hex) {
    char hex_chars[] = "0123456789ABCDEF";
    int i = 0;
    int remainder;

    // 处理负数
    if (decimal < 0) {
        decimal = -decimal;
        hex[i++] = '-';
    }

    // 将十进制数转换为十六进制字符串
    do {
        remainder = decimal % 16;
        hex[i++] = hex_chars[remainder];
        decimal /= 16;
    } while (decimal > 0);

    // 如果结果为空，表示输入为 0
    if (i == 0) {
        hex[i++] = '0';
    }

    // 反转字符串
    for (int j = 0; j < i / 2; j++) {
        char temp = hex[j];
        hex[j] = hex[i - j - 1];
        hex[i - j - 1] = temp;
    }

    hex[i] = '\0';  // 添加字符串终止符
}

// printf 实现的有问题， 在write系统调用里面用printf会输出乱码
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
            else if (*fmt == 'x')
            {
                int i = va_arg(args, int);
                char hex_numstr[10];  // 足够大的缓冲区来存储十六进制字符串
                dec_to_hex(i, hex_numstr);
                putch('0');
                putch('x');
                for (char *p = hex_numstr; *p != '\0'; p++) {
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
            else if (*fmt == 'p')
            {
                void *ptr = va_arg(args, void *);  // 获取指针参数
                char ptr_str[20];  // 存放指针地址的字符串
                putch('0'); putch('x');  // 打印 "0x" 前缀
                itoa_hex((unsigned long)ptr, ptr_str);  // 将指针地址转换为十六进制
                for (char *p = ptr_str; *p != '\0'; p++) 
                {
                    putch(*p);
                }
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

int snprintf(char *out, size_t n, const char *fmt, ...) 
{
  va_list args;
  va_start(args, fmt);
  char *str = out;
  size_t written = 0;

  while (*fmt && written < n - 1)  // 保证至少为 \0 留出一个空间
  {
    if (*fmt == '%') 
    {
      fmt++;
      if (*fmt == 'd') 
      {
        int i = va_arg(args, int);
        char *num_str;
        num_str = itoa(i);
        for (char *p = num_str; *p != '\0' && written < n - 1; p++) 
        {
          *str++ = *p;
          written++;
        }
      } 
      else if (*fmt == 's') 
      {
        char *s = va_arg(args, char*);
        while (*s && written < n - 1) 
        {
          *str++ = *s++;
          written++;
        }
      }
    } 
    else 
    {
      *str++ = *fmt;
      written++;
    }
    fmt++;
  }

  // 确保结果字符串以 '\0' 结尾
  *str = '\0';
  
  va_end(args);

  // 返回实际应写入的字符数，不受 n 的限制
  return written;  
}


int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
