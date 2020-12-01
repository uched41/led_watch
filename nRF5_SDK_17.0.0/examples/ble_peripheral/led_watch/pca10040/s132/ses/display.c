#include <stdlib.h>
#include "nrf_delay.h"

#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "DISPLAY"
#include "log.h"
#include "hardware.h"
#include "user_configs.h"
#include "text.h"
#include "display.h"
#include "fsm.h"
#include "watch.h"

display_t mdisplay = {
  .led_driver               = NULL,
  .ltemplate                = {0},
  .display_state            = DISPLAY_NONE,
  .display_watch_state      = DISPLAY_WATCH_START,
  .current_watch_animation  = SLIDEIN_VERTICAL_ANIM,
  .brightness               = HALF_BRIGHTNESS
};

/* Extern Functions for different modes */
extern void display_watch_init    (void);
extern void display_watch_fsm     (display_t* display);
extern void display_pomodoro_fsm  (display_t* display);
extern void display_pomodoro_init (void);


/* Initialize display driver */
void display_init(display_t* display){
  display->led_driver = (led_driver_t*)malloc(sizeof(led_driver_t)); 
  led_driver_init(display->led_driver);

  display_watch_init();
  display_pomodoro_init();
  LOGD("Init Complete.");
}


/* De-initialize display driver */
void display_deinit(display_t* display){
  led_driver_deinit(display->led_driver);
  if(display->led_driver) free(display->led_driver);
}


/* Show current time */
void display_show(display_t* display){
  leds_show(display->led_driver);
}


/* Clear Display */
void display_clear(display_t* display){
  leds_clear_and_show(display->led_driver);
}


/* Display State Machine */
void display_fsm(display_t* display){
  switch(device_status.device_mode){

    case DEVICE_WATCH:
      display_watch_fsm(display);
      break;
      
    case DEVICE_POMODORO:
      display_pomodoro_fsm(display);
      break;

    default:
      break;
  }
}
