#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "WATCH"
#include "log.h"

#include <stdlib.h>
#include "nrf_delay.h"
#include "hardware.h"
#include "user_configs.h"
#include "text.h"
#include "display.h"
#include "fsm.h"
#include "ticker.h"
#include "rtc.h"
#include "watch.h"

/* positions for writing digits */
const location_t posi1 = {0,0};
const location_t posi2 = {4,0};
const location_t posi3 = {0,6};
const location_t posi4 = {4,6};

/* Include animations */
extern const display_watch_animation_t simple_show_anim ;
extern const display_watch_animation_t horizontal_line_anim;
extern const display_watch_animation_t stack_down_anim;
extern const display_watch_animation_t slidein_vertical_anim;
extern const display_watch_animation_t slidein_horizontal_anim;
extern const display_watch_animation_t cirular_anim;
extern const display_watch_animation_t pixel_follow_anim;

display_watch_animation_t     display_time_animations[ANIMATION_COUNT];
static ticks_t                watch_start_time; 
static display_watch_state_t  watch_prev_state;

static char* get_state_string(display_watch_state_t state){
  switch(state){
    case DISPLAY_WATCH_IDLE: 
      return "DISPLAY_WATCH_IDLE"    ; 
    case DISPLAY_WATCH_START: 
      return "DISPLAY_WATCH_START"   ; 
    case DISPLAY_WATCH_SHOWING: 
      return "DISPLAY_WATCH_SHOWING" ; 
    case DISPLAY_WATCH_FINISH: 
      return "DISPLAY_WATCH_FINISH"  ; 
    default:
      return " "                     ; 
  }
}

static void watch_change_state(display_t* display, display_watch_state_t state){
  watch_prev_state              = display->display_watch_state;
  display->display_watch_state  = state;

  char* prev_string = get_state_string(watch_prev_state);
  char* cur_string  = get_state_string(state);

  LOGD("%s -> %s", prev_string, cur_string);
}


void display_watch_init(void){
  display_time_animations[0] = simple_show_anim;
  display_time_animations[1] = horizontal_line_anim;
  display_time_animations[2] = stack_down_anim;
  display_time_animations[3] = slidein_vertical_anim;
  display_time_animations[4] = slidein_horizontal_anim;
  display_time_animations[5] = cirular_anim;
  display_time_animations[6] = pixel_follow_anim;
}


void display_set_time(display_t* display, location_t p1, location_t p2, location_t p3, location_t p4){
  rtc_read_time(&utime);
  //rtc_print_time(&utime);
  write_digit(display, p1, (utime.tm_hour)/10, user_config.hour_color, user_config.bck_color);
  write_digit(display, p2, (utime.tm_hour)%10, user_config.hour_color, user_config.bck_color);
  write_digit(display, p3, (utime.tm_min )/10, user_config.mins_color, user_config.bck_color);
  write_digit(display, p4, (utime.tm_min )%10, user_config.mins_color, user_config.bck_color);
}


void display_watch_fsm(display_t* display){
  
  switch(display->display_watch_state){
    case DISPLAY_WATCH_IDLE:
      break;

    case DISPLAY_WATCH_START:
      ( *(display_time_animations[display->current_watch_animation].fcall) )(display);
      display->display_state = DISPLAY_SHOWING;
      watch_change_state(display, DISPLAY_WATCH_SHOWING);     /* Move to next state */ 
      tick_refresh(WATCH_DISPLAY_START_TIME);
      break;

    case DISPLAY_WATCH_SHOWING:
      if(TICKS_ELAPSED(myticks[WATCH_DISPLAY_START_TIME], user_config.watch_show_time_delay)){
        watch_change_state(display, DISPLAY_WATCH_FINISH);     /* Move to next state */ 
      }
      break;

    case DISPLAY_WATCH_FINISH:
      display_clear(display);
      display->display_state = DISPLAY_FINISH;
      watch_change_state(display, DISPLAY_WATCH_IDLE);        /* Move to next state */ 
      break;

    default:
      break;
  }
}


