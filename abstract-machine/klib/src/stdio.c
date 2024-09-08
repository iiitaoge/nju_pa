#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *str = out;
  
  while (*fmt) {
      if (*fmt == '%') {
          fmt++;
          if (*fmt == 'd') {
              int i = va_arg(args, int);
              char *num_str;
              num_str = itoa(i); 
              for (char *p = num_str; *p != '\0'; p++) {
                  *str++ = *p;
              }
          } else if (*fmt == 's') {
              char *s = va_arg(args, char*);
              while (*s) {
                  *str++ = *s++;
              }
          }
      } else {
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
