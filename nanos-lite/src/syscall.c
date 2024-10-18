#include <common.h>
#include "syscall.h"

// 引入了menuconfig设置的 STRACE
#include "../../../nemu/include/generated/autoconf.h"
#include <fs.h>

// #include <sys/time.h>
#include <stdint.h>

// 引入这个结构体，方便
struct timeval {
	long		tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};

static void _SYS_exit(Context *c);
static void _SYS_write(Context *c);
static void _SYS_brk(Context *c);
static void _SYS_open(Context *c);
static void _SYS_read(Context *c);
static void _SYS_lseek(Context *c);
static void _SYS_close(Context *c);
static void _SYS_gettimeofday(Context *c);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; // 没有用a0作为系统调用号，用了a7
  // printf("系统调用号为 %d\n", a[0]);
  // a[0] 是 yield 这样的编号，用来识别 是什么系统调用

#ifdef CONFIG_STRACE
  Log("系统调用编号为 %d", a[0]);
#endif

  switch (a[0]) {
    case SYS_exit: 
      _SYS_exit(c);
      break;
    case SYS_write:
      _SYS_write(c);
      break;
    case SYS_brk: 
      _SYS_brk(c);
      break;
    case SYS_open:
      _SYS_open(c);
      break;
    case SYS_read:
      _SYS_read(c);
      break;
    case SYS_lseek:
      _SYS_lseek(c);
      break;
    case SYS_close:
      _SYS_close(c);
      break;
    case SYS_gettimeofday:
      _SYS_gettimeofday(c);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

static void _SYS_exit(Context *c)
{
  c->GPRx=0;
  halt(c->GPRx);
}

static void _SYS_write(Context *c)
{
  
  int fd = c->GPR2;
  void *buf = (void*)c->GPR3;
  int count = c->GPR4;
  // printf("fd = %d buf = %p count = %d \n", fd, c->GPR3, count);
  fs_write(fd, buf, count);
  // 这玩意一定要返回， 不然缓冲区就会被破坏
  c->GPRx = c->GPR4;  // 取出需要打印的字节数目
}

static void _SYS_brk(Context *c)
{
  //int32_t add = c->GPR2;  // 取出新地址
  c->GPRx = 0; // pa3暂时认为全部调整成功
}

static void _SYS_open(Context *c)
{
  const char * path = (const char*) c->GPR2;
  int flags = c->GPR3;
  int mode = c->GPR4;
  int fd = fs_open(path, flags, mode);
  c->GPRx = fd;
}

static void _SYS_read(Context *c)
{
  int fd = c->GPR2;
  void *buf = (void*) c->GPR3;
  size_t len = c->GPR4;
  size_t count = fs_read(fd, buf, len); // read 不一定读 len，如果len大于文件剩余长度则只读剩余长度字节
  c->GPRx = count;
}

static void _SYS_lseek(Context *c)
{
  int fd = c->GPR2;
  size_t offset = c->GPR3;
  int whence = c->GPR4;
  size_t cur_offset = fs_lseek(fd, offset, whence);
  c->GPRx = cur_offset;
}

static void _SYS_close(Context *c)
{
  int fd = c->GPR2;
  fs_close(fd);
}

// 直接用指针操作，性能感觉很强，毕竟减去了中间os和用户层通过系统调用返回值交互
static void _SYS_gettimeofday(Context *c)
{
  // 获取 tv 的指针，通过指针和固定的内存结构访问其中的成员
  struct timeval *tv = (struct timeval *)c->GPR2;
  __uint64_t time = io_read(AM_TIMER_UPTIME).us;
  tv->tv_usec = (time % 1000000);
  tv->tv_sec = (time / 1000000);
  c->GPRx = 0;
}