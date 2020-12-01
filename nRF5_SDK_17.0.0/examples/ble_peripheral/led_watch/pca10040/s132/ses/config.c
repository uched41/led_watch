#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "CONFIG"
#include "log.h"

#include "display.h"
#include "hardware.h"
#include "firmware_config.h"
#include "user_configs.h"

volatile user_config_t user_config;

void config_init(void){
  user_config.hour_color             = RED;
  user_config.mins_color             = RED;
  user_config.bck_color              = BLACK;
  user_config.brightness             = HALF_BRIGHTNESS;
  user_config.slidein_vert_type      = 0;
  user_config.slidein_hori_type      = 2;
  user_config.watch_show_time_delay  = MS_TO_TICKS(WATCH_SHOW_TIME_DELAY);
}

