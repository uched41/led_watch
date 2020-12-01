#ifndef RTC_H
#define RTC_H

#include "time.h"

typedef struct tm rtc_time_t ;

typedef struct {
  unsigned char enabled;	/* 0 = alarm disabled, 1 = alarm enabled */
  unsigned char pending;        /* 0 = alarm not pending, 1 = alarm pending */
  rtc_time_t time;         /* time the alarm is set to */
}rtc_alarm_t;


void rtc_init(void);

void rtc_deinit(void);

void rtc_print_time(rtc_time_t *dt);

uint8_t rtc_read_time(rtc_time_t *dt);

uint8_t rtc_set_time(rtc_time_t *dt);

uint8_t rtc_set_alarm(rtc_alarm_t *alarm);

uint8_t rtc_disable_alarm(void);

uint8_t TEST_RTC(void);

#endif


