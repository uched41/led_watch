#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include "stdint.h"

#define DEVICE_NAME           "Lio"
#define MANUFACTURER_NAME     "Mescana"
#define MODEL                 "CH.NF.1X"
#define SOFTWARE_REVISION     "2.0.0"
#define HARDWARE_REVISION     "1.0.0"
#define FIRMWARE_REVISION     "0.0.1"

#define UNIQUE_DEVICE_ID_0    NRF_FICR->DEVICEID[0]
#define UNIQUE_DEVICE_ID_1    NRF_FICR->DEVICEID[1]

#define MANUFACTURER_ID       0
#define ORG_UNIQUE_ID         0

#endif