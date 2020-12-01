#include "nrf_drv_gpiote.h"
#include "nrf_drv_saadc.h"
#include "app_timer.h"
#include "string.h"

#include "fsm.h"
#include "charging_module.h"
#include "firmware_config.h"
#include "hardware.h"

#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "BATTERY"
#include "log.h"

APP_TIMER_DEF(battery_timer);
#define BATTERY_UPDATE_TICKS     APP_TIMER_TICKS(BATTERY_STATUS_UPDATE_INTERVAL)
#define BAT_ADC                  NRF_SAADC_INPUT_AIN3

void saadc_callback(nrf_drv_saadc_evt_t const * p_event){};

static void battery_level_cb(void * p_context){
  UNUSED_PARAMETER(p_context);

  ret_code_t err_code;
  nrf_saadc_channel_config_t channel_config_bat = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(BAT_ADC);
  
  APP_ERROR_CHECK(nrf_drv_saadc_init(NULL, saadc_callback));
  APP_ERROR_CHECK(nrf_drv_saadc_channel_init(1, &channel_config_bat));

  nrf_saadc_value_t sample;
  APP_ERROR_CHECK(nrfx_saadc_sample_convert(1, &sample));

  float bat_voltage = (sample *2 *3.6) / 1024;
  set_battery_voltage(bat_voltage);

  /* Deinitialize and power down ADC */
  nrf_drv_saadc_uninit();
}


void charger_module_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
  bool PWR, STAT;
  PWR = nrf_drv_gpiote_in_is_set(CHG_POW);
  STAT = nrf_drv_gpiote_in_is_set(CHG_STAT);

  /* CHARGING */
  if(!PWR && !STAT){
    set_charging_state(CHARGING);
    LOGI("Charging");
  }

  /* CHARGING DONE, STILL USB POWER */
  else if(!PWR && STAT){
    set_charging_state(CHARGE_DONE_USB_POWER);
    LOGI("Battery full");
  }

  /* NO USB NOT CHARGING */
  else if(PWR && STAT){
    set_charging_state(NOT_CHARGING_NO_USB_POWER);
    LOGI("Not charging");
  }

  battery_level_cb(NULL);
}


/**
 * Initialize Battery Monitor
 */
void charging_module_init(void){
  ret_code_t err_code;
  
  /* Initialize pins for battery interrupts*/
  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP;

  APP_ERROR_CHECK(nrf_drv_gpiote_in_init(CHG_STAT, &in_config, charger_module_cb));
  nrf_drv_gpiote_in_event_enable(CHG_STAT, true);

  APP_ERROR_CHECK(nrf_drv_gpiote_in_init(CHG_POW, &in_config, charger_module_cb));
  nrf_drv_gpiote_in_event_enable(CHG_POW, true);

  /* Create timers to periodically update battery level */
  err_code = app_timer_create(&battery_timer, APP_TIMER_MODE_REPEATED, battery_level_cb);
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_start(battery_timer, BATTERY_UPDATE_TICKS, NULL);
  APP_ERROR_CHECK(err_code);

  /* Call charger cb once to set initial state */
  set_charging_state(CHARGE_UNKNOWN);
  charger_module_cb(1, NRF_GPIOTE_POLARITY_TOGGLE);
  LOGI("Init Complete");
}


/**
 * Denitialize Battery Monitor
 */
void charging_module_deinit(void){
  nrf_drv_gpiote_in_uninit(CHG_POW);
  nrf_drv_gpiote_in_uninit(CHG_STAT);
  nrf_drv_gpiote_in_event_disable(CHG_POW);
  nrf_drv_gpiote_in_event_disable(CHG_STAT);
  app_timer_stop(battery_timer);
}