#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define FD_EVENT 11

static int evtdev = -1;
static int fbdev = -1;
//屏幕大小
static int screen_w = 0, screen_h = 0;
//画布大小
static int canvas_w = 0,canvas_h = 0;
//相对于屏幕左上角的画布位置坐标
static int canvas_x = 0,canvas_y = 0;

// 函数：跳过空白字符（空格、制表符等）
char *skip_whitespace(char *ptr) {
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')
    {
        ptr++;
    }
    return ptr;
}

// 函数：提取数字
int extract_number(char **ptr) 
{
    int num = 0;
    while (**ptr >= '0' && **ptr <= '9')
     {
        num = num * 10 + (**ptr - '0');
        (*ptr)++;
    }
    return num;
}

// 函数：检查并匹配字符串
int match_string(char **ptr, const char *str) 
{
    while (*str) {
        if (**ptr != *str) {
            return 0;  // 不匹配
        }
        (*ptr)++;
        str++;
    }
    return 1;  // 匹配成功
}


// 函数：解析dispinfo字符串，提取WIDTH和HEIGHT的值
void parse_dispinfo(char *info, int *width, int *height) 
{
    char *ptr = info;

    while (*ptr) {
        // 跳过前导空白字符
        ptr = skip_whitespace(ptr);

        // 检查是否是 "WIDTH"
        if (match_string(&ptr, "WIDTH")) 
        {
            // 跳过空白字符和冒号
            ptr = skip_whitespace(ptr);
            if (*ptr == ':') {
                ptr++;
                ptr = skip_whitespace(ptr);
                *width = extract_number(&ptr);
            }
        }
        // 检查是否是 "HEIGHT"
        else if (match_string(&ptr, "HEIGHT")) 
        {
            // 跳过空白字符和冒号
            ptr = skip_whitespace(ptr);
            if (*ptr == ':') {
                ptr++;
                ptr = skip_whitespace(ptr);
                *height = extract_number(&ptr);
            }
        }

        // 跳过当前行剩余部分或其他字符
        while (*ptr && *ptr != '\n') 
        {
            ptr++;
        }

        // 如果遇到换行符，跳到下一行
        if (*ptr == '\n') 
        {
            ptr++;
        }
    }
}

// 获取自系统启动开始的毫秒数
uint32_t NDL_GetTicks() 
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec / 1000 + tv.tv_sec * 1000;
}

// 键盘事件,打开文件然后读取
int NDL_PollEvent(char *buf, int len) 
{
  int fd = open("/dev/events", 0, 0);
  int re = read(fd, buf, len);
  return re;
}

void NDL_OpenCanvas(int *w, int *h) 
{
  char * buf[32];
  int fd = open("/proc/dispinfo", 0, 0);
  int ret = read(fd, buf, sizeof(buf));

  int width = 0, height = 0;
  parse_dispinfo(buf, &width, &height); // 读取屏幕大小
  screen_w = width; 
  screen_h = height;
  if (*w == 0 && *h == 0) // 如果画布大小为0，那么画布将是整个屏幕
  {
    *w = screen_w;
    *h = screen_h;
  }
  // 画布大小
  canvas_w = *w;
  canvas_h = *h;
  // 如果有画布，就去中心点绘画，画布 == 0，就在(0,0)开始画
  canvas_x = (screen_w - canvas_w) / 2; // 自定义坐标
  canvas_y = (screen_h - canvas_h) / 2;
}

// pixels是存储照片像素的指针 ， w h 是需要打印的矩形（画布大小）
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) 
{
  int fd = open("/dev/fb", 0, 0);
  for (int i = 0; i < h && y + i < canvas_h; ++i) 
  {
    // 找到坐标，每次递增画布的一行
    // x, y 默认为0所以需要加上我们自定义的坐标才能居中显示 
    lseek(fd, ((y + canvas_y + i) * screen_w + (x + canvas_x)) * 4, SEEK_SET);
    // 每次写入一行，每次递增画布的一行
    write(fd, pixels + i * w, 4 * (w < canvas_w - x ? w : canvas_w - x)); 
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
