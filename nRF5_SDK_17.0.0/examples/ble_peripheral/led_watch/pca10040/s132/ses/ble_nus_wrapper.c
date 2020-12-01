#include <stdarg.h>

#include "nrf_log.h"
#include "app_error.h"
#include "ble_nus_wrapper.h"


#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "BLE"
#include "log.h"

/**
 * Parse BLE commands sent by app
 * Params: uint8_t* buf -> Buffer containing data
 *         uint8_t len  -> length of buffer
 * Ret: None
 */
void ble_command_parse(const uint8_t* buf, uint8_t len){
 
  char command = 0;
  
  switch(command){

    default:
      break;
  }
}








