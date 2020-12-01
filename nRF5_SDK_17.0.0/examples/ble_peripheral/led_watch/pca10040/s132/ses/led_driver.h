#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <stdint.h>

// structure for storing color
#define LEDS_WIDTH  7
#define LEDS_LENGTH 11
#define LEDS_COUNT  LEDS_WIDTH * LEDS_LENGTH

// convert coordiante location to index
#define loc2index(l) ( (l.y>=0 && l.y<LEDS_LENGTH && l.x>=0 && l.x<LEDS_WIDTH) ? ( ((l.y*LEDS_WIDTH)+l.x) )  : (-1) )

typedef struct {
 uint8_t br, b, g, r;
}crgb_t;

typedef struct {
 uint8_t b, g, r;
}crg_t;

// X and Y coordinate of pixel starting from 0
typedef struct {
  int8_t x, y;
}location_t;

// Class for led driver
typedef struct {
    crgb_t buffer[LEDS_COUNT];
}led_driver_t;


void led_driver_deinit  (led_driver_t* leds);
void led_driver_init    (led_driver_t* leds);
void leds_setpixel1     (led_driver_t* leds, location_t locale, crgb_t col);
void leds_setpixel2     (led_driver_t* leds, uint8_t ind, crgb_t col);
void leds_setpixel_len  (led_driver_t* leds, uint8_t start, uint8_t len, crgb_t col);
void leds_copy_pixels   (led_driver_t* target, uint8_t start, led_driver_t* src, uint8_t src_start, uint16_t cnt);
void leds_show          (led_driver_t* leds);
void leds_clear_and_show(led_driver_t* leds);
void leds_clear         (led_driver_t* leds);
void leds_test          (led_driver_t* leds);
void leds_reverse_array (led_driver_t* leds, uint8_t start, uint8_t len);


#endif