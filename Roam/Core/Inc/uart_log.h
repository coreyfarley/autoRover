/**
 * @file	uart_log.h
 * @brief	Tagged, time-stamped UART logging for Roam AutoRover
 *
 * Provides formatted log output over UART (req IF_011 and IF_012).
 */

#ifndef INC_UART_LOG_H_
#define INC_UART_LOG_H_

/* Includes */
#include "stm32l4xx_hal.h"   // board specific to L-4 family, will change when final board has arrived


// Log tags
typedef enum {
	LOG_TAG_IMU = 0,		// Accelerometer / Gyroscope snapshot
	LOG_TAG_TEMP,			// Temperature change event
	LOG_TAG_HUM,			// Humidity change event
	LOG_TAG_SOIL,			// Soil Moisture sample
	LOG_TAG_SYS,			// System events
	LOG_TAG_COUNT			// sentinel: number of valid tags, used to size arrays in uart_log.c
} log_tag_t;

/**
 * Initialize UART logging module.
 *
 * Call once at startup, before any uart_log_write() calls.
 *
 * @param  huart  Pointer to the UART handle to send log output through.
 *                The handle must stay valid for the whole program -
 *                pass in a global like &huart1.
 */
void uart_log_init(UART_HandleTypeDef *huart);

/**
 * Write a tagged log line to UART.
 * Output format:
 * [TAG]   t=<ms>   <message>\r\n
 *
 * Time-stamp is captured from HAL_GetTick() at call time.
 * If message exceeds the internal buffer, message gets truncated.
 *
 * @param  tag  Log category (see log_tag_t).
 * @param  fmt  printf-style format string for message body.
 * @param  ...  Values to fill in the %place-holders in fmt, if any.
 */
void uart_log_write(log_tag_t tag, const char *fmt, ...);


#endif 
