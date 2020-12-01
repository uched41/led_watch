#ifndef BLE_NUS_WRAPPER_H
#define BLE_NUS_WRAPPER_H

#include "ble_nus.h"

extern uint16_t m_conn_handle;  /* Connection handle controlled in main.c */
extern ble_nus_t m_nus;         /* NUS object defined in main module */


void ble_command_parse(const uint8_t* buf, uint8_t len);
#endif


