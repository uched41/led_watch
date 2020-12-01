#include "stdlib.h"
#include "string.h"

#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"

#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "RTC"
#include "log.h"
#include "hardware.h"
#include "rtc.h"
#include "rtc_hal.h"

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(1); 
typedef enum{ RTC_AM, RTC_PM }rtc_mode_t;

unsigned _bcd2bin(unsigned char val){ return (val & 0x0f) + (val >> 4) * 10; }
unsigned char _bin2bcd(unsigned val){ return ((val / 10) << 4) + val % 10; }

#define const_bcd2bin(x)    (((x) & 0x0f) + ((x) >> 4) * 10)
#define const_bin2bcd(x)    ((((x) / 10) << 4) + (x) % 10)
#define bcd2bin(x)          (__builtin_constant_p((uint8_t )(x)) ? const_bcd2bin(x) : _bcd2bin(x))
#define bin2bcd(x)          (__builtin_constant_p((uint8_t )(x)) ? const_bin2bcd(x) : _bin2bcd(x))


#define cs_activate()       nrf_drv_gpiote_out_set(RTC_CE)
#define cs_deactivate()     nrf_drv_gpiote_out_clear(RTC_CE)


/**
 * HAL Functions for SPI
 */
static uint8_t rtc_spi_write(uint8_t reg, uint8_t* buf, uint8_t len){
  cs_activate();
  uint32_t res;

  if(len < 1 || len > 255) return 1;
  uint8_t buf2[len+1];
  buf2[0] = reg | 0x80 ;    // Write bit set
  memcpy(buf2+1, buf, len);  

  res = nrf_drv_spi_transfer(&spi, buf2, len+1, NULL, 0);
  APP_ERROR_CHECK(res);
  cs_deactivate();
  return res;
}

static uint8_t rtc_spi_read(uint8_t reg, uint8_t* buf, uint8_t len){
  cs_activate();
  uint32_t res;
  uint8_t buf2[len+1];

  res = nrf_drv_spi_transfer(&spi, &reg, 1, buf2, len+1);
  APP_ERROR_CHECK(res);
  cs_deactivate();

  memcpy(buf, buf2+1, len);
  return res;
}

static uint8_t rtc_update_bit(uint8_t reg, uint8_t mask, uint8_t val){
  uint8_t orig, ret;
  ret = rtc_spi_read(reg, &orig, 1);
  if (ret != 0)return ret;

  uint8_t tmp;
  tmp = orig & ~mask;
  tmp |= val & mask;

  ret = rtc_spi_write(reg, &tmp, 1);
  return ret;
}


/**
 * RTC Interrupt Callback
 */
void rtc_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){

}


void rtc_pf_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){

}


/**
 * RTC Initialize
 */
rtc_time_t cvt_date(char const *date, char const *time){
  char s_month[5];
  int year;
  struct tm t;
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  sscanf(date, "%s %d %d", s_month, &t.tm_mday, &year);
  sscanf(time, "%2d %*c %2d %*c %2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
  t.tm_mon = (strstr(month_names, s_month) - month_names) / 3 ;    
  t.tm_year = year-1900;  
  t.tm_isdst = -1;
  mktime(&t);
  return t;
}

void rtc_init(void){
  /* Initialize SPI */                                                                       
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin    = NRF_DRV_SPI_PIN_NOT_USED;
  spi_config.mosi_pin  = RTC_SDI;
  spi_config.miso_pin  = RTC_SDO;
  spi_config.sck_pin   = RTC_SCLK;
  spi_config.frequency = NRF_DRV_SPI_FREQ_500K;
  spi_config.mode      = NRF_DRV_SPI_MODE_3;
  APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));

  /* CS Pin */
  nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
  APP_ERROR_CHECK(nrf_drv_gpiote_out_init(RTC_CE, &out_config));

  /* PF Pin */
  nrf_drv_gpiote_in_config_t in_config1 = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
  APP_ERROR_CHECK(nrf_drv_gpiote_in_init(RTC_RESET, &in_config1, rtc_pf_cb));

  /* Initialize Alarm Int0 */
  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP;

  APP_ERROR_CHECK(nrf_drv_gpiote_in_init(RTC_INT0, &in_config, rtc_cb));
  nrf_drv_gpiote_in_event_enable(RTC_INT0, true);

  /* Set trickle charge resistors */
  uint8_t trickle_val = 0b10101010 ;
  APP_ERROR_CHECK(rtc_spi_write(DS1343_TRICKLE_REG, &trickle_val, 1));

  rtc_time_t mt = cvt_date(__DATE__, __TIME__);
  rtc_print_time(&mt);
  rtc_set_time(&mt);
  LOGI("Init complete.");
}


void rtc_deinit(void){
  nrf_drv_spi_uninit(&spi);
  nrf_drv_gpiote_in_uninit(RTC_INT0);
  nrf_drv_gpiote_in_event_disable(RTC_INT0);
}


/**
 * RTC read time
 */
uint8_t rtc_read_time(rtc_time_t *dt){
  unsigned char buf[7];
  uint8_t res;

  res = rtc_spi_read(DS1343_SECONDS_REG, buf, 7);
  if (res)
    return res;

  dt->tm_sec	= bcd2bin(buf[0]);
  dt->tm_min	= bcd2bin(buf[1]);
  dt->tm_hour	= bcd2bin(buf[2] & 0x3F);
  dt->tm_wday	= bcd2bin(buf[3]) - 1;
  dt->tm_mday	= bcd2bin(buf[4]);
  dt->tm_mon	= bcd2bin(buf[5] & 0x1F) - 1;
  dt->tm_year	= bcd2bin(buf[6]) + 100; /* year offset from 1900 */

  return 0;
}


/**
 * RTC set time
 */
uint8_t rtc_set_time(rtc_time_t *dt){
  uint8_t buf[7];

  buf[0] = bin2bcd(dt->tm_sec);
  buf[1] = bin2bcd(dt->tm_min);
  buf[2] = bin2bcd(dt->tm_hour) & 0x3F;
  buf[3] = bin2bcd(dt->tm_wday + 1);
  buf[4] = bin2bcd(dt->tm_mday);
  buf[5] = bin2bcd(dt->tm_mon + 1);
  buf[6] = bin2bcd(dt->tm_year - 100);

  return rtc_spi_write(DS1343_SECONDS_REG,buf, sizeof(buf));
}


uint8_t rtc_set_alarm(rtc_alarm_t *alarm){
  unsigned char buf[4];
  uint8_t res = 0;

  res = rtc_update_bit(DS1343_CONTROL_REG, DS1343_A0IE, 0);
  if (res) return res;

  buf[0] = bin2bcd(alarm->time.tm_sec);
  buf[1] = bin2bcd(alarm->time.tm_min);
  buf[2] = bin2bcd(alarm->time.tm_hour);
  buf[3] = bin2bcd(alarm->time.tm_mday);

  res = rtc_spi_write(DS1343_ALM0_SEC_REG, buf, 4);
  if (res) return res;

  if (alarm->enabled)
    res = rtc_update_bit(DS1343_CONTROL_REG, DS1343_A0IE, DS1343_A0IE);

  return res;
}


uint8_t rtc_disable_alarm(void){
  return rtc_update_bit(DS1343_CONTROL_REG, DS1343_A0IE, 0);
}


void rtc_print_time(rtc_time_t *dt){
  LOGI("%d:%d:%d %d:%d:%d", dt->tm_hour, dt->tm_min, dt->tm_sec, dt->tm_year+1900, dt->tm_mon+1, dt->tm_mday);
}


uint8_t TEST_RTC(void){
  rtc_time_t mtime;
  rtc_read_time(&mtime);
  rtc_print_time(&mtime);

  nrf_delay_ms(2000);
  rtc_read_time(&mtime);
  rtc_print_time(&mtime);
}
