#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "BUTTON"
#include "log.h"

#include "nrf_drv_gpiote.h"
#include "app_timer.h"

#include "firmware_config.h"
#include "hardware.h"
#include "ticker.h"

APP_TIMER_DEF(debounce_timer);
volatile ticks_t push_btn_pressed_time;

void button_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
  bool btn;
  btn = nrf_drv_gpiote_in_is_set(PUSH_BUTTON);

  /* Button Press */
  if(!btn){
      /* Start debouncing timer */
      app_timer_start(debounce_timer, APP_TIMER_TICKS(30), NULL);
  }

  /* Button release */
  else{
    ticks_t time_elapsed = TICK_DIFF_MS(myticks[PUSH_BUTTON_LT]);

      /* Short button press 100ms - 1500ms */
      if (time_elapsed > 10 && time_elapsed < 150){
        LOGD("Short button press: %d", time_elapsed);
      }

      /* Longer button press 1500ms - 4500ms */
      else if(time_elapsed > 150 && time_elapsed < 450){
        LOGD("Long button press: %d", time_elapsed);
      }

      /* Very long button press 1500ms - 4500ms */
      else if(time_elapsed > 450 && time_elapsed < 1000){
        LOGD("Very long button press: %d", time_elapsed);
      }
  }

}

/* Debounce timer callback */
static void dtimer_cb(void * p_context){
    UNUSED_PARAMETER(p_context);
    
    if(!nrf_drv_gpiote_in_is_set(PUSH_BUTTON)){   // if pin is still low
      tick_refresh(PUSH_BUTTON_LT);
    }
}


void button_init(void){
  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP;

  APP_ERROR_CHECK(nrf_drv_gpiote_in_init(PUSH_BUTTON, &in_config, button_cb));
  nrf_drv_gpiote_in_event_enable(PUSH_BUTTON, true);
  
  APP_ERROR_CHECK(app_timer_create(&debounce_timer, APP_TIMER_MODE_SINGLE_SHOT, dtimer_cb));

  LOGI("Init complete.");
}


void setup_working_button(void){

}

