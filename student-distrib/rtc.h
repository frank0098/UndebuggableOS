#ifndef RTC_H
#define RTC_H
#include "types.h"

int time[3];
void initial_rtc();
void rtc_int_handle();
int32_t rtc_open();
int32_t rtc_wait();
int32_t rtc_change_freq(int32_t freq);
int32_t rtc_close();
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t transfer_to_rate(int32_t freq);
int check_power(int32_t x);
void set_time();
void status_bar();
void clear_statusbar();

static volatile int INT = 0;

#endif


