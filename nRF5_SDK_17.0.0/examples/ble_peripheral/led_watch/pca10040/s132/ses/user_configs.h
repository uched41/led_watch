#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include "led_driver.h"
#include "firmware_config.h"
#include "fsm.h"

void config_init(void);

typedef struct{
  crgb_t    hour_color;         // Hour Color
  crgb_t    mins_color;         // Minutes Color
  crgb_t    bck_color;          // Background Color
  uint8_t   brightness;         // Brightness
  uint8_t   slidein_vert_type;
  uint8_t   slidein_hori_type;
  uint16_t  watch_show_time_delay;
}user_config_t;

volatile extern user_config_t user_config;


/* Confiigurable Parameters */ 

/* Watch Animations */
#define ANIM_HORIZONTAL_LINE_DELAY      70  /* In milliseconds */
#define ANIM_CIRCULAR_DELAY             10
#define ANIM_PIXEL_FOLLOW_DELAY         40
#define ANIM_SLIDE_IN_HORIZONTAL_DELAY  150
#define ANIM_SLIDE_IN_VERTICAL_DELAY    100
#define ANIM_STACK_DOWN_DELAY           50

/* Watch Delay */
#define WATCH_SHOW_TIME_DELAY           4000 /* In milliseconds */

#define FULL_BRIGHTNESS 0b11111111
#define HALF_BRIGHTNESS 0b11110000
#define LOW_BRIGHTNESS  0b11100010
#define VLOW_BRIGHTNESS 0b11100001

#endif


