#include "nrf_log.h"
#include <stdlib.h>


#include "text.h"

// Mapping number to coordinate on display
const uint16_t digits[10] = {
  0b0111101101101111,   // 0
  0b0111010010010010,   // 1
  0b0111001111100111,   // 2
  0b0111100111100111,   // 3
  0b0100100111101101,   // 4
  0b0111100111001111,   // 5
  0b0111101111001111,   // 6
  0b0100100111100111,   // 7
  0b0111101111101111,   // 8
  0b0100100111101111    // 9
};

static const int8_t digit_width = 3;
static const int8_t digit_height = 5;

void write_digit(display_t* display, location_t start_loc, uint8_t digit, crgb_t col, crgb_t bckcol){
  crgb_t* buf = display->led_driver->buffer;

  if(digit>9) return;
  uint8_t iter1 = 0;    // Variable for iterating through the bits in digit
  for(int8_t length=0; length<digit_height; length++){      // Iterate through each row
    for(int8_t width=0; width<digit_width; width++){        // Iterate through each column
      if(digits[digit] & (uint16_t)(1<<(iter1++))){                   // Check if cell is on
        int8_t posi = loc2index( ((location_t){start_loc.x+width, start_loc.y+length}) );
        if(posi>=0 && posi<LEDS_COUNT){
          buf[posi] = col;
          display->ltemplate[posi] = 1;
        }
      }
      else{
        buf[ loc2index( ((location_t){start_loc.x+width, start_loc.y+length}) ) ] = bckcol;
      }
    }
  }
} 

