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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/paddr.h> // 操作内存

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1); // -1 会作为 
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;   // 将 nemu 的状态设置为退出状态
  return -1;
}

static int cmd_help(char *args);

// 单步执行
static int cmd_si(char *args)
{
  // 使用 strtoull 将字符串转换为无符号长整型
  uint64_t value = strtoull(args, NULL, 10);
  cpu_exec(value);
  return 0;
}

// 
static int cmd_info(char *args)
{
  if (strcmp(args, "r") == 0) 
  {
    isa_reg_display();
  } 
  else if (strcmp(args, "w") == 0) 
  {
    print_watch_pointer();
  } 
  else 
  {
    printf("没有这个info命令\n");
  }
  return 0;
}

// 扫描内存
static int cmd_x(char *args)
{
  // 进行参数分离
  char *n = strtok(args, " ");  // 要打印的字节数
  char *EXPR = n + strlen(n) + 1; // 表达式的起始地址
  bool success = false;
  word_t address = expr(EXPR,&success);  // 求出的表达式地址

  int len = atoi(n);
  if (len >= 4) // 先把4的整数倍长度处理了
  {
    int i = 0;
    for (; i <= len - 4; i = i + 4)
    {
      address += i;
      printf("%#010x ", paddr_read(address, 4));
    }
    address += i;
    len = len - i;
  }
  switch (len)  // 此时只剩下四种可能， 1，2，3，0
  {
  case 1:
    printf("%#04x ", paddr_read(address, 1));
    break;
  
  case 2:
    printf("%#06x ", paddr_read(address, 2));

  case 3:
    printf("%#08x ", paddr_read(address, 2) + paddr_read(address + 2, 1) * 16 * 16 * 16 * 16);
    // printf("%#04x ", paddr_read(address + 2, 1));

  default:
    break;
  }
  printf("\n");
  return 0;
}

// 打印表达式值
static int cmd_p(char *args)
{
  bool success = true;  // 定义一个实际的 bool 变量并初始化为 true
  printf("%u\n", expr(args, &success)); // 传递 success 的地址
  return 0;
}

// 创建监视点
static int cmd_w(char *args)
{
  sdb_new_wp(args);
  return 0;
}

// 删除监视点
static int cmd_d(char *args)
{
  delete_wp(args);
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  // 以下都为自己补充的命令
  { "si", "程序单步执行 N 条指令后停止", cmd_si},
  { "info", "info r ：打印寄存器的值， info w ：打印监视点信息", cmd_info },
  { "x", "x N EXPR : 求出表达式EXPR的值, 将结果作为起始内存地址, 以十六进制形式输出连续的N个4字节", cmd_x },
  { "p", "打印表达式的值", cmd_p},
  { "w", "w *0x80000000 创建监视点", cmd_w},
  { "d", "d 2 删除指定序号监视点", cmd_d},
  // { "t", "测试专用", cmd_test},
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1; // 1 是空格的长度
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
