#include "nrf_delay.h"
#include "display.h"
#include "led_driver.h"
#include "user_configs.h"

extern const location_t posi1, posi2, posi3, posi4;


void displaytime_stackdown(display_t* display){
  display_set_time(display, posi1, posi2, posi3, posi4);
  led_driver_t temp_driver;
  leds_clear(&temp_driver);

  for(int8_t i=LEDS_LENGTH-1; i>-1; i--){
    if(i==5) continue;
    for(int8_t j=0; j<i+1; j++){
      leds_copy_pixels(&temp_driver, (j*LEDS_WIDTH), display->led_driver, (i*LEDS_WIDTH), LEDS_WIDTH);

      //if(j%2) leds_reverse_array(&temp_driver, (j*LEDS_WIDTH), LEDS_WIDTH);
      leds_show(&temp_driver);
      nrf_delay_ms(ANIM_STACK_DOWN_DELAY);

      leds_setpixel_len(&temp_driver, (j*LEDS_WIDTH), LEDS_WIDTH, BLACK);
    }
    leds_copy_pixels(&temp_driver, (i*LEDS_WIDTH), display->led_driver, (i*LEDS_WIDTH), LEDS_WIDTH);
  }
}


const display_watch_animation_t stack_down_anim = {
  .fcall = displaytime_stackdown
};
