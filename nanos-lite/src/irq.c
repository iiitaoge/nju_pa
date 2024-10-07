#include <common.h>
// 引入了menuconfig设置的 STRACE
#include "../../../nemu/include/generated/autoconf.h"

void do_syscall(Context *c);


static Context* do_event(Event e, Context* c) {

#ifdef CONFIG_STRACE
  Log("系统调用编号为 %d\n", e.event);
#endif
  switch (e.event) {
    case 1: //这里没有调用 am 的 yield
      printf("event ID=%d\nc->GPR1=%d\nc->GPR2=%d\nc->GPR3=%d\nc->GPR4=%d\nc->GPRx=%d\n",
      e.event,c->GPR1,c->GPR2,c->GPR3,c->GPR4,c->GPRx);halt(0);printf("执行了halt之后");//EVENT_YIELD
      c->mepc += 4; // 从自陷返回需要 mepc += 4， 不然无限循环
      break;
    case 2: // 系统调用事件
      do_syscall(c);
      c->mepc += 4; // 系统调用完也要加4
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
