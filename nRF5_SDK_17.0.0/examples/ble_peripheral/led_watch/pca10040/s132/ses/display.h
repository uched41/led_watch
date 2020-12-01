#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "led_driver.h"

typedef enum{
  DISPLAY_NONE,
  DISPLAY_START,
  DISPLAY_SHOWING,
  DISPLAY_FINISH
}display_state_t;

typedef enum{
  DISPLAY_WATCH_IDLE,
  DISPLAY_WATCH_START,
  DISPLAY_WATCH_SHOWING,
  DISPLAY_WATCH_FINISH
}display_watch_state_t;
 
typedef enum{
  DISPLAY_POMODORO_IDLE,
  DISPLAY_POMODORO_START,
  DISPLAY_POMODORO_TIME_ALMOST_UP,
  DISPLAY_POMODORO_TIME_UP,
  DISPLAY_POMODORO_FINISH
}display_pomodoro_state_t;

typedef struct {
  led_driver_t*             led_driver;
  uint8_t                   ltemplate[LEDS_COUNT];
  display_state_t           display_state;
  display_watch_state_t     display_watch_state;
  display_pomodoro_state_t  display_pomodoro_state;
  uint8_t                   current_watch_animation;
  uint8_t                   brightness;
}display_t;

typedef struct{
  void (*fcall) (display_t* display);
}display_watch_animation_t;

extern display_t mdisplay;

void display_init       (display_t* display);
void display_show       (display_t* display);
void display_fsm        (display_t* display);
void display_set_time   (display_t* display, location_t p1, location_t p2, location_t p3, location_t p4);

// Display Functions
typedef void (*DISPLAY_FUNCTION)      (display_t* display);    // Function pointer definition
void display_clear                    (display_t* display);
void displaytime_just_show            (display_t* display);
void displaytime_horiontal_linebyline (display_t* display);
void displaytime_randomize            (display_t* display);
void displaytime_pixel_follow         (display_t* display);
void displaytime_slidein_horizontal   (display_t* display);
void displaytime_slidein_vertical     (display_t* display);
void displaytime_circular             (display_t* display);
void displaytime_stackdown            (display_t* display);


/* Color definitions */
#define RED     (crgb_t){mdisplay.brightness, 0,  20, 0}
#define BLACK   (crgb_t){mdisplay.brightness, 0,  0,  0}
#define WHITE   (crgb_t){mdisplay.brightness, 20, 20, 20}
#define BLUE    (crgb_t){mdisplay.brightness, 0,  0,  20}
#define GREEN   (crgb_t){mdisplay.brightness, 20, 0,  0}

#endif


