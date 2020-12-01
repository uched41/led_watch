#ifndef CHARGING_MODULE_H
#define CHARGING_MODULE_H

typedef enum{
  CHARGE_UNKNOWN,
  CHARGING,
  CHARGE_DONE_USB_POWER,
  NOT_CHARGING_NO_USB_POWER
}charging_state_t;

void charging_module_init(void);

void charging_module_deinit(void);

#endif


