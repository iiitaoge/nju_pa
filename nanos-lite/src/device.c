#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

// 标准输出
size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; i++) 
    putch(*((char *)buf + i));
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  // int keycode = io_read(AM_INPUT_KEYBRD).keycode;
  // printf("os 层接收到 keycode = %d\n", keycode);
  if (ev.keycode == AM_KEY_NONE)
  {
    return 0;
  }
  else
  {
    // int keydown = io_read(AM_INPUT_KEYBRD).keydown;
    int keydown = ev.keydown;
    char *buffer = (char *)buf;  // 转换为 char* 便于指针运算

    // printf("keydown = %d\n", keydown);
    // 复制 "kd " 或 "ku "
    if (keydown)
      memcpy(buffer, "kd ", 3);
    else
      memcpy(buffer, "ku ", 3);

    buffer += 3;  // 将指针前移3个字节，指向下一段要复制的位置

    // 复制 keyname 对应的字符串
    strncpy(buffer, keyname[ev.keycode], len - 3);  // 只复制剩余的 len - 3 字节
    return len;
  }
}


size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
