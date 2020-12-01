#include "nrf_delay.h"
#include "display.h"
#include "led_driver.h"
#include "user_configs.h"

extern const location_t posi1, posi2, posi3, posi4;

void displaytime_pixel_follow(display_t* display){
  led_driver_t temp_driver;
  leds_clear(&temp_driver);
  display_set_time(display, posi1, posi2, posi3, posi4);

  for(uint8_t i=0; i<LEDS_COUNT; i++){
    leds_setpixel2(&temp_driver, i, display->led_driver->buffer[i]);
    leds_show(&temp_driver);
    nrf_delay_ms(ANIM_PIXEL_FOLLOW_DELAY);
  }
}



const display_watch_animation_t pixel_follow_anim = {
  .fcall = displaytime_pixel_follow
};
