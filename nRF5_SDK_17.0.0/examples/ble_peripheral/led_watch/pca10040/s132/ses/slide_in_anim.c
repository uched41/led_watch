#include "nrf_delay.h"
#include "display.h"
#include "led_driver.h"
#include "user_configs.h"

extern const location_t posi1, posi2, posi3, posi4;

void displaytime_slidein_horizontal(display_t* display){
  for(int8_t i = (user_config.slidein_hori_type==0) ? -3 :-6; i<1; i++){    // For first case, we to iterate only 3 times
    leds_clear(display->led_driver);
   
    switch(user_config.slidein_hori_type){
      case 0:
        display_set_time(display, (location_t){i+posi1.x, posi1.y}, (location_t){posi2.x-i, posi2.y}, (location_t){i+posi3.x, posi3.y}, (location_t){posi4.x-i, posi4.y});
        break;
      case 1:
        display_set_time(display, (location_t){i+posi1.x, posi1.y}, (location_t){i+posi2.x, posi2.y}, (location_t){i+posi3.x, posi3.y}, (location_t){i+posi4.x, posi4.y});
        break;
      case 2:
        display_set_time(display, (location_t){i+posi1.x, posi1.y}, (location_t){i+posi2.x, posi2.y}, (location_t){-i+posi3.x, posi3.y}, (location_t){-i+posi4.x, posi4.y});
        break;
      case 3:
        break;
      default:
        break;
    }
    display_show(display);
    nrf_delay_ms(ANIM_SLIDE_IN_HORIZONTAL_DELAY);
  }
}


void displaytime_slidein_vertical(display_t* display){
  for(int8_t i = (user_config.slidein_vert_type==0) ? -5 :-10; i<1; i++){    // For first case, we to iterate only 3 times
    leds_clear(display->led_driver);
   
    switch(user_config.slidein_vert_type){
      case 0:
        display_set_time(display, (location_t){posi1.x, i+posi1.y}, (location_t){posi2.x, i+posi2.y}, (location_t){posi3.x, posi3.y-i}, (location_t){posi4.x, posi4.y-i});
        break;
      case 1:
        display_set_time(display, (location_t){posi1.x, i+posi1.y}, (location_t){posi2.x, i+posi2.y}, (location_t){posi3.x, i+posi3.y}, (location_t){posi4.x, i+posi4.y});
        break;
      case 2:
        display_set_time(display, (location_t){posi1.x, posi1.y-i}, (location_t){posi2.x, posi2.y-i}, (location_t){posi3.x, posi3.y-i}, (location_t){posi4.x, posi4.y-i});
        break;
      case 3:
        break;
      default:
        break;
    }
    display_show(display);
    nrf_delay_ms(ANIM_SLIDE_IN_VERTICAL_DELAY);
  }
}


const display_watch_animation_t slidein_horizontal_anim = {
  .fcall = displaytime_slidein_horizontal
};


const display_watch_animation_t slidein_vertical_anim = {
  .fcall = displaytime_slidein_vertical
};