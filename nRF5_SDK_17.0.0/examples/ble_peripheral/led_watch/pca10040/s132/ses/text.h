#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include "led_driver.h"
#include "display.h"

void write_digit(display_t* display, location_t start_loc, uint8_t digit, crgb_t col, crgb_t bckcol);

#endif