#include <am.h>
#include <nemu.h>
// #include <riscv.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint32_t time_low = inl(RTC_ADDR);
  uint32_t time_high = inl(RTC_ADDR + 4);
  uint64_t time_now = ((uint64_t)time_high << 32) | time_low;
  uptime->us = time_now;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
