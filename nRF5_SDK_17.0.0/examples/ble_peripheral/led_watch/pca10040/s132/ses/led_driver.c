#include <stdlib.h>
#include "nrf_drv_spi.h"
#include "nrf_log.h"
#include "nrf_delay.h"

#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "LED"
#include "log.h"
#include "led_driver.h"
#include "hardware.h"

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(2);    // spi instance


/**
 * Initialize Led Driver
 */
void led_driver_init(led_driver_t* leds){	                                                                                                                      
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.mosi_pin = LED_DATA;
  spi_config.sck_pin  = LED_SCK;
  spi_config.frequency = NRF_DRV_SPI_FREQ_8M;

  ret_code_t err_code;
  APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));
  
  leds_clear(leds);
  LOGD("Init Complete.");
  srand(7); 
}


/**
 * De-Initialize Led Driver
 */
void led_driver_deinit(led_driver_t* leds){
  nrf_drv_spi_uninit(&spi);
}


/**
 * Set pixel using (x, y) coordinate
 */
void leds_setpixel1(led_driver_t* leds, location_t locale, crgb_t col){
  int8_t x = loc2index(locale);
  if(x>=0 && x<LEDS_COUNT){
    leds->buffer[x] = col;
  }
}


/**
 * Set pixel using index
 */
void leds_setpixel2(led_driver_t* leds, uint8_t ind, crgb_t col){
  leds->buffer[ind] = col;
}


void leds_setpixel_len(led_driver_t* leds, uint8_t start, uint8_t len, crgb_t col){
  for(uint8_t i=start; i<start+len; i++){
    leds->buffer[i] = col;
  }
}


/**
 * Show LEDs, SPI out
 */
void leds_show(led_driver_t* leds){
  uint8_t start_buf[4] = { 0, 0, 0, 0 };
  uint8_t end_buf[13]  = { 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00 };

  int slen = sizeof(crgb_t)*LEDS_COUNT;
  uint8_t* bbuf = (uint8_t*)leds->buffer;

  /* Starting sequence */
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, start_buf, sizeof(start_buf)/sizeof(start_buf[0]), NULL, 0));      

  /* Send led data */
  if(slen<256){
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, (uint8_t*)(bbuf), slen, NULL, 0));  
  }
  else{
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, (uint8_t*)(bbuf), 255, NULL, 0));   
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, ((uint8_t*)(bbuf))+255, slen-255, NULL, 0));  
  }

  /* End sequence */
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, end_buf, sizeof(end_buf)/sizeof(end_buf[0]), NULL, 0));  
}


/* Clear Leds and show clear1 */
void leds_clear_and_show(led_driver_t* leds){
  leds_clear(leds);
  leds_show(leds);
}

/* Clear leds without show */
void leds_clear(led_driver_t* leds){
  for(int i=0; i<LEDS_COUNT; i++){
    leds->buffer[i] = (crgb_t){0b11110000, 0, 0, 0};
  }
}

/* Copy pixels directly */
void leds_copy_pixels(led_driver_t* target, uint8_t start, led_driver_t* src, uint8_t src_start, uint16_t cnt){
  if(src == NULL || target == NULL || cnt == 0) return;
  memcpy( (target->buffer)+start, (src->buffer)+src_start, cnt*sizeof(crgb_t));
}

void leds_test(led_driver_t* leds){
  leds_clear_and_show(leds);
  for(uint8_t i=0; i<LEDS_COUNT; i++){
    leds_setpixel2(leds, i, (crgb_t){0b11110000, 0, 15, 0});
    leds_show(leds);
    nrf_delay_ms(30);
  }
}

void leds_reverse_array(led_driver_t* leds, uint8_t start, uint8_t len){
  crgb_t* buf = (leds->buffer)+start;
  for(uint8_t i=0; i<4; i++){
    crgb_t tem = buf[len-i-1];
    buf[len-i-1] = buf[i];
    buf[i] = tem;
  }
}