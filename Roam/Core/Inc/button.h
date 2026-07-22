/**
 * @file    button.h
 * @brief   Button & debounce driver header
 *
 * Interface for initializing, updating, and querying
 * user-button state.
 *
 * Requirements: NFR_007, IF_008, FR_003, FR_004.
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

/* Includes */
#include "stm32l4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* Function Prototypes */

/**
 * Initialize the Button module.
 * Call once at startup.
 * Reads initial level so no phantom press at boot.
 *
 * @param  port    Pointer to GPIO_TypeDef port
 * @param  pin     Pin mask (e.g. GPIO_PIN_13)
 */
void button_init(GPIO_TypeDef *port, uint16_t pin);

/**
 * Call every main-loop iteration.
 * Owns the debounce timing.
 *
 * @param  now_ms  current system tick in milliseconds.
 */
void button_update(uint32_t now_ms);

/**
 * Returns true once per debounced press.
 * READING CONSUMES THIS EVENT.
 */
bool button_pressed(void);

#endif
