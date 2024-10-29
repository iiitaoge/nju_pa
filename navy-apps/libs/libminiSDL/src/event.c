#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static void get_first_word(const char *buf, char *first_word) {
    int i = 0;

    // 读取第一个单词，直到遇到空格或字符串结束
    while (buf[i] != '\0' && !isspace(buf[i])) {
        first_word[i] = buf[i];
        i++;
    }
    first_word[i] = '\0';  // 终止字符串
}

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64];
  int keycode = NDL_PollEvent(buf, sizeof(buf));
  if (keycode)
  {
    char *first_word[8];
    get_first_word(buf, first_word);  // 获取按下还松开
    int up_down = strcmp("ku", first_word); // 非0为按下，不为0为松开
    if (up_down)  // 按下
    {
      event->type = SDL_KEYDOWN;
    }
    else
    {
      event->type = SDL_KEYUP;
    }
    event->key.keysym.sym = keycode;
  }
  // if (keycode)
  // {
  //   printf("状态为 %s 键盘码 = %d\n", first_word, keycode);
  //   printf("a = %d\n", a);
  // }
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
