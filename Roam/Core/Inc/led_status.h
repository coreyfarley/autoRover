/**
 * @file    led_status.h
 * @brief	LED status indicator driver header
 *
 * Maps system states to LED behavior (solid, blink, off)
 * with non-blocking blink timing.
 * 
 */

#ifndef INC_LED_STATUS_H_
#define INC_LED_STATUS_H_

#include "stm32l4xx_hal.h"

/** GPIO port/pin pair for a single LED */
typedef struct 
{
    GPIO_TypeDef    *port;
    uint16_t        pin;    
} led_gpio_t;

/** System states that drive LED behavior */
typedef enum 
{
    LED_STATE_IDLE = 0,
    LED_STATE_DRIVING,
    LED_STATE_SAMPLING,
    LED_STATE_FAULT,
    LED_STATE_LOW_BATTERY,
    LED_STATE_SLEEP
} led_state_t;

/**
 * Initialize the LED status module.
 *
 * @param  green  Pointer to green LED GPIO descriptor.
 * @param  amber  Pointer to amber LED GPIO descriptor.
 * @param  red    Pointer to red LED GPIO descriptor.
 */
void led_status_init(const led_gpio_t *green,
                     const led_gpio_t *amber,
                     const led_gpio_t *red);

/**
 * Set the current system state for LED indication.
 *
 * @param  state  System state to reflect on the LEDs.
 */
void led_status_set(led_state_t state);

/**
 * Update LED outputs. Call from main loop each iteration.
 * Handles blink timing for states that require it.
 *
 * @param  now_ms  Current system tick in milliseconds.
 */
void led_status_update(uint32_t now_ms);

#endif
