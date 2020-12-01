#ifndef FIRMWARE_CONFIG_H
#define FIRMWARE_CONFIG_H

/* Battery and Charger Module */
#define BATTERY_STATUS_UPDATE_INTERVAL    5 * 60 * 1000   /* In milliseconds */

/* Ticker */
#define TICKER_RESOLUTION       10              /* In milliseconds */
#define MS_TO_TICKS(x)          (x + TICKER_RESOLUTION / 2) / TICKER_RESOLUTION   
                                /* should be x / TICKER_RESOLUTION, but solution is used to round up */
#define TICKS_TO_MS(x)          (int)(x * TICKER_RESOLUTION)

#endif


