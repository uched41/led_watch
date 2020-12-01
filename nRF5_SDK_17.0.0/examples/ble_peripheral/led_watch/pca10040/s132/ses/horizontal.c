#include "display.h"
#include "led_driver.h"
#include "user_configs.h"
#include "nrf_delay.h"

extern const location_t posi1, posi2, posi3, posi4;

void displaytime_horiontal_linebyline(display_t* display){
  led_driver_t temp_driver;

  display_set_time(display, posi1, posi2, posi3, posi4);
  leds_clear(&temp_driver);

  for(uint8_t i=0; i<LEDS_LENGTH; i++){
    leds_copy_pixels(&temp_driver, i*LEDS_WIDTH, display->led_driver, i*LEDS_WIDTH, LEDS_WIDTH);
    leds_show(&temp_driver);
    nrf_delay_ms(ANIM_HORIZONTAL_LINE_DELAY);
  }
}


const display_watch_animation_t horizontal_line_anim = {
  .fcall  = displaytime_horiontal_linebyline
};
