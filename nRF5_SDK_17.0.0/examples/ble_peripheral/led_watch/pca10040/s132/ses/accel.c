#include "nrf_drv_gpiote.h"
#include "nrf_drv_spi.h"
#include "app_error.h"
#include "app_timer.h"
#include "string.h"

#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "ACC"
#include "log.h"
#include "nrf_log.h"
#include "hardware.h"

//#define INV_MSG_ENABLE
#ifdef  INV_MSG_ENABLE
static  void msg_printer(int level, const char * str, va_list ap);
#define MSG_LEVEL INV_MSG_LEVEL_DEBUG
#endif

#include "Invn/Devices/Drivers/Icm20648/Icm20648.h"
#include "Invn/Devices/Drivers/Icm20648/Icm20648DataBaseControl.h"
#include "Invn/Devices/SensorTypes.h"
#include "Invn/Devices/SensorConfig.h"
#include "Invn/EmbUtils/Message.h"

#include "acc_sensor.h"

#define COMPASS_IS_AK9912   0

static const nrf_drv_spi_t  spi = NRF_DRV_SPI_INSTANCE(0); 
#define cs_activate()       nrf_drv_gpiote_out_clear(MPU_CS)
#define cs_deactivate()     nrf_drv_gpiote_out_set(MPU_CS)

volatile uint8_t acc_interrupt_flag = 0;


/**
 * HAL Read and Write for IMu Driver
 */
int idd_io_hal_read_reg(void * context, uint8_t reg, uint8_t * rbuffer, uint32_t rlen){
  ret_code_t res;
  uint8_t buf[rlen+1];
  uint8_t rreg = reg | 0x80;

  cs_activate();
  res =  nrf_drv_spi_transfer(&spi, &rreg, 1, buf, rlen+1);
  cs_deactivate();

  memcpy(rbuffer, buf+1, rlen);
  APP_ERROR_CHECK(res);
  return res;
}

int idd_io_hal_write_reg(void * context, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen){
  ret_code_t res = 0;
  int start = 0;
  cs_activate();
  res = nrf_drv_spi_transfer(&spi, &reg, 1, NULL, 0);   /* Send register */

  while( (start < wlen) ){
    int len = (wlen / 255) ? (255) : (wlen % 255); 
    uint8_t temp_buf[len];
    memcpy(temp_buf, wbuffer+start, len);   // Copy data to correct memory segment

    res =  nrf_drv_spi_transfer(&spi, temp_buf, len, NULL, 0);
    APP_ERROR_CHECK(res);
    start += len;
  }
  cs_deactivate();
  return res;
}


/**
 * IMU Interrupt Callback
 */
void acc_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
  acc_interrupt_flag = 1;

  // inv_icm20648_poll_sensor(&icm_device, (void *)0, build_sensor_event_data);
}


/**
 * Accel Initialize 
 */
void acc_init(void){
  /* Initialize interrupt */
  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
  in_config.pull = NRF_GPIO_PIN_PULLDOWN;
  APP_ERROR_CHECK(nrf_drv_gpiote_in_init(MPU_INT1, &in_config, acc_cb));
  nrf_drv_gpiote_in_event_enable(MPU_INT1, true);

  /* Initialize SPI */
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin     = NRF_DRV_SPI_PIN_NOT_USED;
  spi_config.mosi_pin   = MPU_SDI;
  spi_config.miso_pin   = MPU_SDO;
  spi_config.sck_pin    = MPU_SCLK;
  spi_config.frequency  = NRF_DRV_SPI_FREQ_1M;
  spi_config.mode       = NRF_DRV_SPI_MODE_3;
  APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));

  /* CS Pin */
  nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(true);
  APP_ERROR_CHECK(nrf_drv_gpiote_out_init(MPU_CS, &out_config));
  
  /* Initialize icm20648 serif structure */
  struct inv_icm20648_serif icm20648_serif;
  icm20648_serif.context   = 0; /* no need */
  icm20648_serif.read_reg  = idd_io_hal_read_reg;
  icm20648_serif.write_reg = idd_io_hal_write_reg;
  icm20648_serif.max_read  = 255; /* maximum number of bytes allowed per serial read */
  icm20648_serif.max_write = 1024*1; /* maximum number of bytes allowed per serial write */
  icm20648_serif.is_spi    = true;

  /* Setup Message printer */
  INV_MSG_SETUP(MSG_LEVEL, msg_printer);

  /* Reset icm20648 driver states */
  inv_icm20648_reset_states(&icm_device, &icm20648_serif);

  /* Setup the icm20648 device */
  icm20648_sensor_setup();

  /*
  * Now that Icm20648 device was initialized, we can proceed with DMP image loading
  * This step is mandatory as DMP image are not store in non volatile memory
  */
  APP_ERROR_CHECK(load_dmp3());

  inv_icm20648_enable_sensor(&icm_device, INV_ICM20648_SENSOR_B2S, 1);

  /* Just get the whoami */
  uint8_t whoami = 0xff;
  APP_ERROR_CHECK(inv_icm20648_get_whoami(&icm_device, &whoami));
  INV_MSG(INV_MSG_LEVEL_INFO, "ICM WHOAMI=%02X", whoami);
}


#ifdef INV_MSG_ENABLE
/**
 * Printer function for message facility
 */
static void msg_printer(int level, const char * str, va_list ap){

  static char out_str[256]; /* static to limit stack usage */
  unsigned idx = 0;
  const char * ptr = out_str;
  const char * s[INV_MSG_LEVEL_MAX] = {
          "",    // INV_MSG_LEVEL_OFF
          " [E] ", // INV_MSG_LEVEL_ERROR
          " [W] ", // INV_MSG_LEVEL_WARNING
          " [I] ", // INV_MSG_LEVEL_INFO
          " [V] ", // INV_MSG_LEVEL_VERBOSE
          " [D] ", // INV_MSG_LEVEL_DEBUG
  };
  idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "%s", s[level]);
  if(idx >= (sizeof(out_str)))
          return;
  idx += vsnprintf(&out_str[idx], sizeof(out_str) - idx, str, ap);
  if(idx >= (sizeof(out_str)))
          return;
  idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "\r\n");
  if(idx >= (sizeof(out_str)))
          return;

  printf("%s", ptr);
}
#endif

void TEST_ACCEL(void){

}