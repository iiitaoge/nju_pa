#ifndef ARCH_H__
#define ARCH_H__

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif

struct Context {
  // TODO: fix the order of these members to match trap.S
  uintptr_t gpr[NR_REGS];  // 通用寄存器
  uintptr_t mcause;        // 机器异常原因
  uintptr_t mstatus;       // 机器状态寄存器
  uintptr_t mepc;          // 机器异常程序计数器
  void *pdir;              // 页表基址
};

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7
#endif

#define GPR2 gpr[0]
#define GPR3 gpr[0]
#define GPR4 gpr[0]
#define GPRx gpr[0]

#endif
