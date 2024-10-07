#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case -1: case 1:
        // printf("c->mcause = %d\n", c->mcause);
        ev.event=EVENT_YIELD; // os 的 do_event会用到这个事件号
        break;
      
      case 0:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:case 10:case 11:case 12:case 13:case 14:case 15:case 16:case 17:case 18:case 19:
        // printf("c->mcause = %d\n", c->mcause);
        ev.event=EVENT_SYSCALL;
        break;

      default: ev.event = EVENT_ERROR; 
      break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {  // 这是操作系统直接调用yield，调用号是 -1 如果是navy-app的那个测试，它是 1
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall"); // 这里给 a7 赋的值就是 事件分发时的编号
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
