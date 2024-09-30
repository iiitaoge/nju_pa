#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t am_scancode = inl(KBD_ADDR);
  // 最高位是判断是按下还是弹回
  if (am_scancode & KEYDOWN_MASK) // 最高位是 1 按下
  {
    kbd->keydown = 1;
    kbd->keycode = am_scancode ^ KEYDOWN_MASK;
  }
  else
  {
    kbd->keydown = 0;
    kbd->keycode = am_scancode;
  }
}
