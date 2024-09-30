#include <am.h>
#include <nemu.h>
#include <klib.h>
// #include <riscv.h>

static uint64_t sys_init_time;

void __am_timer_init() {
  sys_init_time =(  ( (uint64_t)inl(RTC_ADDR+4) << 32 ) + (uint64_t)inl(RTC_ADDR) );
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  // 要先读高位再读低位，这样才能触发nemu对读取时间的加锁
  uptime->us = (  ( (uint64_t)inl(RTC_ADDR + 4) << 32 ) | (uint64_t)inl(RTC_ADDR)  );
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
