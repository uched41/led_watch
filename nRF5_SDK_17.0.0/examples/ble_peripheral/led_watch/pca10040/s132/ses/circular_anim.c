#include "display.h"
#include "led_driver.h"
#include "ticker.h"
#include "user_configs.h"
#include "nrf_delay.h"

extern const location_t posi1, posi2, posi3, posi4;

void displaytime_circular(display_t* display){
  led_driver_t temp_driver;

  display_set_time(display, posi1, posi2, posi3, posi4);
  leds_clear(&temp_driver);

  int8_t middle_x = 3;
  int8_t middle_y = 5;
  
  for(int8_t i=0; i<6; i++){
    int8_t spiraln = i+1;

    // Iterate to right
    int8_t indr = loc2index( ((location_t){middle_x+i, middle_y}) ) ;
    leds_setpixel2(&temp_driver, indr, display->led_driver->buffer[indr]);
    leds_show(&temp_driver);
    nrf_delay_ms(ANIM_CIRCULAR_DELAY);

    // Iterate to up
    if(i<4){
      for(int8_t k=0; k<spiraln; k++){
        int8_t indt = loc2index( ((location_t){middle_x+i, middle_y-k}) ) ;
        leds_setpixel2(&temp_driver, indt, display->led_driver->buffer[indt]);
        leds_show(&temp_driver);
        nrf_delay_ms(ANIM_CIRCULAR_DELAY);
      }
    }

    // Iterate to left
    for(int8_t k=0; k<(spiraln-1)*2; k++){
      int8_t indl = loc2index( ((location_t){middle_x+i-k, middle_y-i}) ) ;
      leds_setpixel2(&temp_driver, indl, display->led_driver->buffer[indl]);
      leds_show(&temp_driver);
      nrf_delay_ms(ANIM_CIRCULAR_DELAY);
    }

    // Iterate to down
    if(i<4){
      for(int8_t k=0; k<(spiraln-1)*2; k++){
        int8_t ind = loc2index( ((location_t){middle_x-i, middle_y-i+k}) ) ;
        leds_setpixel2(&temp_driver, ind, display->led_driver->buffer[ind]);
        leds_show(&temp_driver);
        nrf_delay_ms(ANIM_CIRCULAR_DELAY);
      }
    }
    
    // Iterate to right
    for(int8_t k=0; k<(spiraln-1)*2; k++){
      int8_t ind = loc2index( ((location_t){middle_x-i+k, middle_y+i}) ) ;
      leds_setpixel2(&temp_driver, ind, display->led_driver->buffer[ind]);
      leds_show(&temp_driver);
      nrf_delay_ms(ANIM_CIRCULAR_DELAY);
    }

    // Iterate to up
    if(i<4){
       for(int8_t k=0; k<(spiraln); k++){
        int8_t ind = loc2index( ((location_t){middle_x+i, middle_y+i-k}) ) ;
        leds_setpixel2(&temp_driver, ind, display->led_driver->buffer[ind]);
        leds_show(&temp_driver);
        nrf_delay_ms(ANIM_CIRCULAR_DELAY);
      }
    }
   
  }
}


const display_watch_animation_t cirular_anim = {
  .fcall = displaytime_circular
};
