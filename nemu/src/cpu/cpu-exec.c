/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <sdb/watchpoint.h>
#include <sdb/trace.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
#ifdef CONFIG_FTRACE
  static int call_depth = 0;  // 函数调用的深度
#endif

#ifdef CONFIG_IRINGBUF

  static char * iringbuf[5];  // 环形缓冲区

  static void up_buf(char *strlog) // 更新iringbuf
  {
  free(iringbuf[0]);
  for (int i = 0; i < 4; i++)
  {
    iringbuf[i] = iringbuf[i + 1];
  }
  iringbuf[4] = strlog;
  }

  static void printf_buf()
  {
    for (int i = 0; i < 5; i++)
    {
      if (iringbuf[i] != NULL)  // 检查指针是否为空
      {
        printf("%s\n", iringbuf[i]);
      }
      else
      {
        printf("(null)\n");  // 打印一个占位符，指示此位置为空
      }
    }
  }
#endif


#ifdef CONFIG_FTRACE
  static void ftrace(Decode *_this, vaddr_t dnpc) // 函数踪迹
  { 
    char pos[20];
    strncpy(pos, iringbuf[4], 10);  // 获得现在的 pc
    word_t ftrace_pc = strtol(pos, NULL, 16); // 现在 pc 的 十六进制格式
    for (int i = 0; i < func_amount; i++)
    {
      // 需要在jal 指令中识别出跳转的函数名
      bool match_yes = false;
      // 同时出现这三个 jalr zero ra 才是 ret
      if (strstr(_this->logbuf,"jalr") && strstr(_this->logbuf, "ra") && strstr(_this->logbuf, "zero"))
      {
        match_yes = true; // 出现 ret
      }

      if (dnpc == functions[i].start) // 如果下一个地址为一个函数的开头，则输出此时调用函数指令的地址
      {
        // 进入函数，增加嵌套深度
        printf("%s:%*s call [%s@0x%#x]\n", pos, call_depth * 2, "", functions[i].name, dnpc);
        call_depth++;
      }
      // 出现 ret , 此时在什么函数里，就从什么函数返回
      else if (match_yes && ftrace_pc <= functions[i].end && ftrace_pc >= functions[i].start)  // 在jalr指令中识别出返回的地址 -> 函数名
      {
        // 函数返回，减少嵌套深度
        call_depth--;
        printf("%#x:%*s ret  [%s]\n", ftrace_pc, call_depth * 2, "", functions[i].name);  // 输出是从哪里返回的
      }
    }
  }
#endif

void device_update();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) { // 程序踪迹
#ifdef CONFIG_ITRACE_COND
  if (CONFIG_ITRACE_COND) { log_write("%s\n", _this->logbuf); } // 把 ITRACE_COND 换成 CONFIG_ITRACE_COND
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));

#ifdef CONFIG_IRINGBUF
  char *strlog = strdup(_this->logbuf); 
  up_buf(strlog); // 更新 up_buf
#endif


#ifdef CONFIG_FTRACE
  ftrace(_this, dnpc);  // 打印函数踪迹
#endif

// 监视点
#ifdef CONFIG_WATCHPOINT
  bool is_value_new = print_watch_pointer();
  if (is_value_new) puts(_this->logbuf);
#endif
}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc; 
  isa_exec_once(s); // 取出了二进制指令， 这里执行完之后如果发生了跳转，那么dnpc指向跳转的地址， 同时snpc指向顺序的下一条指令
  cpu.pc = s->dnpc; // 这里很坑，要在译码阶段改变dnpc的值，而非pc
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc); // 记录 pc 的值, 此时已经跳转了
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);  // 记录 指令 的值
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
#endif
}

static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {  // 当断言失败，比如发生段错误时候触发
  isa_reg_display();
  statistic();
#ifdef CONFIG_IRINGBUF
  printf_buf();
#endif
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT); // 当 n < 设定最大值时， g_print_step为true
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
