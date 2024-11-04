#include <NDL.h>
#include <SDL.h>
#include <assert.h>

#define keyname(k) #k,
#define MAX_EVENT 10

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static uint8_t keystate[sizeof(keyname)/sizeof(keyname[0])] = {0}; // 存储所有按键的状态

static void get_first_word(const char *buf, char *first_word) 
{
    int i = 0;

    // 读取第一个单词，直到遇到空格或字符串结束
    while (buf[i] != '\0' && !isspace(buf[i])) 
    {
        first_word[i] = buf[i];
        i++;
    }
    first_word[i] = '\0';  // 终止字符串
}

// 定义事件队列结构
typedef struct 
{
    SDL_Event events[MAX_EVENT];  // 存储事件的数组
    int head;                     // 队列头指针
    int tail;                     // 队列尾指针
    int count;                    // 队列中的事件计数
} EventQueue;
static EventQueue eventQueue = { .head = 0, .tail = 0, .count = 0 };  // 初始化队列

// 检查队列是否已满
static bool isEventQueueFull() {
    return eventQueue.count == MAX_EVENT;
}

// 检查队列是否为空
static bool isEventQueueEmpty() {
    return eventQueue.count == 0;
}


// 将事件加入队列
int SDL_PushEvent(SDL_Event *ev) {
  if (isEventQueueFull())  // 队列满了
    return 0;
  eventQueue.events[eventQueue.tail] = *ev;
  eventQueue.tail = (eventQueue.tail + 1) % MAX_EVENT;
  eventQueue.count++;
  return 1;
}

int SDL_PollEvent(SDL_Event *ev) 
{
  char buf[64];
  int keycode = NDL_PollEvent(buf, sizeof(buf));
  if (isEventQueueEmpty())
  { 
    if (keycode) // 当前有事件
    {
      // 读取事件类型
      char first_word[8];
      get_first_word(buf, first_word);  // 获取按下或松开
      ev->type = (strcmp("kd", first_word) == 0) ? SDL_KEYDOWN : SDL_KEYUP;
      ev->key.keysym.sym = keycode;

      // 更新 keystate 数组
      keystate[keycode] = (ev->type == SDL_KEYDOWN) ? 1 : 0;
      return 1;
    }
    return 0;
  }
  *ev = eventQueue.events[eventQueue.head];
  eventQueue.head = (eventQueue.head + 1) % MAX_EVENT;
  eventQueue.count--;
  printf("队列不为空\n");
  if (keycode)
  {
    SDL_PushEvent(ev);
  }
  return 1;
}

// 等待事件的实现
int SDL_WaitEvent(SDL_Event *event) 
{
  while (SDL_PollEvent(event) == 0) 
  {
    // 如果没有事件可用，则继续等待
    char buf[64];
    int keycode = NDL_PollEvent(buf, sizeof(buf));
    if (keycode) 
    {
      // 读取事件类型
      char first_word[8];
      get_first_word(buf, first_word);  // 获取按下或松开
      event->type = (strcmp("kd", first_word) == 0) ? SDL_KEYDOWN : SDL_KEYUP;
      event->key.keysym.sym = keycode;
      SDL_PushEvent(event);  // 将事件推入队列
      break;
    }
  }
  // 队列中有事件，直接弹出
  return SDL_PollEvent(event);
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return keystate;
}

