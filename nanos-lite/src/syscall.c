#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; // 没有用a0作为系统调用号，用了a7
  // printf("系统调用号为 %d\n", a[0]);
  // a[0] 是 yield 这样的编号，用来识别 是什么系统调用
  switch (a[0]) {
    case 0: //SYS_exit系统调用
      c->GPRx=0;
      halt(c->GPRx);
    case 4: // SYS_write
      c->GPRx=c->GPR4;  // 取出需要打印的字节数目
      char *buf = (char*) c->GPR3;  // 取出缓冲区地址
      printf("%s", buf);  // 开始打印
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
