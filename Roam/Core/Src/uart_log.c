/**
 * @file    uart_log.c
 * @brief   UART Log Module implementation
 */

/* Includes */
#include "uart_log.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

/* Defines */
#define UART_LOG_BUF_SIZE       128
#define UART_LOG_USER_MSG_SIZE  96

/* Static Variables */
static UART_HandleTypeDef *s_huart = NULL;
static char s_buf[UART_LOG_BUF_SIZE];
static char s_user_msg[UART_LOG_USER_MSG_SIZE];

static const char *tag_strings[LOG_TAG_COUNT] =
{
    [LOG_TAG_IMU]  = "[IMU]  ",
    [LOG_TAG_TEMP] = "[TEMP] ",
    [LOG_TAG_HUM]  = "[HUM]  ",
    [LOG_TAG_SOIL] = "[SOIL] ",
    [LOG_TAG_SYS]  = "[SYS]  ",
};

/* Function Definitions */

void uart_log_init(UART_HandleTypeDef *huart)
{
    s_huart = huart;
}

void uart_log_write(log_tag_t tag, const char *fmt, ...)
{
    // Ensure uart_log_init has run before any uart_log_write call
    if (s_huart == NULL)
    {
        return;
    }

    if (tag >= LOG_TAG_COUNT)
    {
        return;
    }

    // Expand the user's format string into the user-message buffer
    va_list args;
    va_start(args, fmt);
    vsnprintf(s_user_msg, UART_LOG_USER_MSG_SIZE, fmt, args);
    va_end(args);

    // Look up tag string and capture timestamp
    const char *tag_str = tag_strings[tag];
    uint32_t timestamp = HAL_GetTick();

    // Assemble the full log line: [TAG] t=<ms>  <user msg>\r\n
    int len = snprintf(s_buf, UART_LOG_BUF_SIZE,
                       "%st=%lu  %s\r\n",
                        tag_str,
                        (unsigned long)timestamp,
                        s_user_msg);

    // Bail on encoding error & clamp if snprintf truncated
    if (len < 0)
    {
        return;
    }
    if (len >= UART_LOG_BUF_SIZE)
    {
        len = UART_LOG_BUF_SIZE - 1;
    }

    /* Push the line out the UART. (blocking is fine during bring up)
     * Revist when state machine lands per NFR_001 */
    HAL_UART_Transmit(s_huart, (uint8_t *)s_buf, len, HAL_MAX_DELAY);
}
