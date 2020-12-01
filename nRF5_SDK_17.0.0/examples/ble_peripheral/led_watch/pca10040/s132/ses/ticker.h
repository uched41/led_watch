#ifndef TICKER_H
#define TICKER_H

#include "stdint.h"

typedef uint32_t ticks_t;

uint8_t TICKS_ELAPSED(ticks_t ltime, uint32_t interval);

ticks_t TICK_DIFF_MS(ticks_t last_time);  

/* Enumeration used to access list of ticks */
typedef enum{
  PUSH_BUTTON_LT,
  WATCH_DISPLAY_START_TIME
}last_ticks_t;
 
/* Put all ticks in one place so that they can be easily reset */
extern ticks_t myticks[20];

void ticker_init(void);

void ticker_deinit(void);

void tick_refresh(last_ticks_t index);

uint8_t TEST_TICKER(void);

#endif


