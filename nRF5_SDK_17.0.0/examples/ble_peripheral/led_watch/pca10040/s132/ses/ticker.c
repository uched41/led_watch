#include <stdlib.h>

#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "TICKER"
#include "log.h"

#include "app_timer.h"
#include "nrf_delay.h"

#include "ticker.h"
#include "firmware_config.h"

volatile ticks_t global_ticks;
ticks_t myticks[20];

APP_TIMER_DEF(ticker_timer);
#define TICKER_INTERVAL     APP_TIMER_TICKS(TICKER_RESOLUTION)


static void ticker_handler(void * p_context){
    UNUSED_PARAMETER(p_context);
    global_ticks++;
}


ticks_t TICK_DIFF_MS(ticks_t last_time){
   return abs(global_ticks - last_time) ;
}

uint8_t TICKS_ELAPSED(ticks_t ltime, uint32_t interval){
  return (global_ticks - ltime > interval) ? 1 : 0 ;
}

/**
 * Initialize Ticker Module to generate ticks
 */
void ticker_init(void){
  ret_code_t err_code;

  // Create timers.
  err_code = app_timer_create(&ticker_timer, APP_TIMER_MODE_REPEATED, ticker_handler);
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_start(ticker_timer, TICKER_INTERVAL, NULL);
  APP_ERROR_CHECK(err_code);

  // Reset global ticks value
  global_ticks = 0;

  LOGI("Init complete.");
}


void ticker_deinit(void){
  app_timer_stop(ticker_timer);
}


void tick_refresh(last_ticks_t index){
  myticks[index] = global_ticks;
}


void start_ticker(void){
  APP_ERROR_CHECK( app_timer_start(ticker_timer, TICKER_INTERVAL, NULL) );
}


void stop_ticker(void){
  APP_ERROR_CHECK( app_timer_stop(ticker_timer) );
}


uint8_t TEST_TICKER(void){
  uint32_t ctick = global_ticks;
  nrf_delay_ms(100);
  
  if( (global_ticks - ctick) == 100 /TICKER_RESOLUTION ){
    LOGI("TEST SUCCESS");
    return 0;
  }
  else{
    LOGE("TEST FAILED");
    return 1;
  }
}