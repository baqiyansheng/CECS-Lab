#include <base.h>
#include <dev-mmio.h>

void __timer_init()
{
}

void __timer_uptime(DEV_TIMER_UPTIME_T *uptime)
{
    // Lab6 TODO: implement uptime MMIO
    // hint: use function `inl` and RTC_ADDR to access RTC MMIO
    uint32_t low = inl(0xA0000040);
    uint32_t high = inl(0xA0000044);
    uptime->us = (uint64_t)high << 32 | low;
}

void __timer_rtc(DEV_TIMER_RTC_T *rtc)
{
    // Lab6 TODO: implement RTC MMIO
    // hint: use function `inl` and RTC_ADDR with offset  to access RTC MMIO
    rtc->year=inl(0xA000005C);
    rtc->month=inl(0xA0000058);
    rtc->day=inl(0xA0000054);
    rtc->hour=inl(0xA0000050);
    rtc->minute=inl(0xA000004C);
    rtc->second=inl(0xA0000048);
}