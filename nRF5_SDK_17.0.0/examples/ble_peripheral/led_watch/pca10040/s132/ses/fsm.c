#include "app_error.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"

#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "FSM"
#include "log.h"

#include "fsm.h"
#include "display.h"
#include "button.h"
#include "accel.h"
#include "charging_module.h"
#include "ticker.h"
#include "user_configs.h"

device_status_t device_status = {
  .device_mode  = DEVICE_WATCH,
  .state        = STATE_STARTUP,
  .connected    = false,
  .dfu_ready    = false,
  .notification_enabled = false
};

rtc_time_t utime;

/***
 * Get String name for FSM State
 */
static char* get_state_string(main_state_t state){
  switch(state){
    case STATE_STARTUP:
      return "STATE_STARTUP"               ;
    case STATE_INITIALIZATION_START: 
      return "STATE_INITIALIZATION_START"  ;
    case STATE_INITIALIZATION_END: 
      return "STATE_INITIALIZATION_END"    ; 
    case STATE_SHOWTIME_START: 
      return "STATE_SHOWTIME_START"        ; 
    case STATE_SHOWTIME_END: 
      return "STATE_SHOWTIME_END"          ; 
    case STATE_WAIT_FOR_EVENT: 
      return "STATE_WAIT_FOR_EVENT"        ; 
    case STATE_DEEP_SLEEP: 
      return "STATE_DEEP_SLEEP"            ; 
    case STATE_BATTERY_LOW: 
      return "STATE_BATTERY_LOW"           ; 
    case STATE_DFU_START: 
      return "STATE_DFU_START"             ; 
    case STATE_CHARGING: 
      return "STATE_CHARGING"              ; 
    default:
      return " "                           ; 
   }
   return " ";
}

void set_charging_state(charging_state_t state){
  device_status.charging_state = state;
}

void set_battery_voltage(float volt){
  device_status.battery_voltage = volt;
}

/***
 * Change FSM State
 */
void change_state(main_state_t new_state){
  device_status.prev_state = device_status.state;
  device_status.state = new_state;
 
  char* prev_string = get_state_string(device_status.prev_state);
  char* cur_string  = get_state_string(device_status.state);

  LOGD("%s -> %s", prev_string, cur_string);
}


/***
 * Main FSM Loop
 */
void fsm_loop(void){
  ret_code_t err_code;

  switch(device_status.state){

    case STATE_STARTUP:
      APP_ERROR_CHECK(nrf_drv_gpiote_init());
      acc_init();
      button_init();
      config_init();
      change_state(STATE_INITIALIZATION_START);
      nrf_delay_ms(10);
      break;

    case STATE_INITIALIZATION_START:
      ticker_init();
      display_init(&mdisplay);
      rtc_init();
      //charging_module_init();

//#define RUN_TESTS
#ifdef RUN_TESTS
      TEST_TICKER();
      TEST_RTC();
#endif

      change_state(STATE_INITIALIZATION_END);
      nrf_delay_ms(10);
      break;


    case STATE_INITIALIZATION_END:
      change_state(STATE_SHOWTIME_START);
      break;


    case STATE_SHOWTIME_START:
      mdisplay.display_state = DISPLAY_START;
      display_fsm(&mdisplay);
      
      if(mdisplay.display_state == DISPLAY_FINISH){
        change_state(STATE_SHOWTIME_END);
      }
      break;


    case STATE_SHOWTIME_END:
      break;


    case STATE_WAIT_FOR_EVENT:
      break;


    case STATE_DEEP_SLEEP:
      break;


    case STATE_BATTERY_LOW:
      break;


    case STATE_DFU_START:
      break;


    case STATE_CHARGING:
      break;

    default:
      break;
  }

}