#include <am.h>

#define RTC_ADDR        (0x02000000 + 0x0000048)

uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint32_t hi = inl(RTC_ADDR + 4);  //顺序很重要
  uint32_t lo = inl(RTC_ADDR);
  uptime->us = ((uint64_t)hi << 32) | lo;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
