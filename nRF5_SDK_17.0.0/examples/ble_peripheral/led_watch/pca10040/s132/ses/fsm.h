#ifndef FSM_H
#define FSM_H

#include "stdint.h"

#include "rtc.h"
#include "charging_module.h"

typedef enum {
  STATE_STARTUP,
  STATE_INITIALIZATION_START,
  STATE_INITIALIZATION_END,
  STATE_SHOWTIME_START,
  STATE_SHOWTIME_END,
  STATE_WAIT_FOR_EVENT,
  STATE_DEEP_SLEEP,
  STATE_BATTERY_LOW,
  STATE_DFU_START,
  STATE_CHARGING
}main_state_t;


typedef enum{
  DEVICE_WATCH,
  DEVICE_POMODORO,
  DEVICE_STOPWATCH
}device_mode_t;


/***
 * Struct to store general device status information
 */
typedef struct{
  main_state_t prev_state;
  main_state_t state;
  device_mode_t device_mode;
  charging_state_t charging_state;
  uint8_t connected;
  uint8_t dfu_ready;
  uint8_t notification_enabled;
  float battery_voltage;
}device_status_t;

extern device_status_t device_status;

extern rtc_time_t utime;

void fsm_loop(void);

void change_state(main_state_t new_state);

void set_charging_state(charging_state_t state);

void set_battery_voltage(float volt);

#endif


