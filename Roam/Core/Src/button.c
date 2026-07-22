/**
 * @file    button.c
 * @brief   Button & debounce driver logic
 *
 * Polls the user button each main-loop iteration and debounces it
 * (accept-after-stable, 25 ms window). Exposes a latched,
 * consume-on-read press event for the state machine.
 *
 * Requirements: NFR_007, IF_008, FR_003, FR_004.
 */

/* Includes */
#include "button.h"

/* Defines */
#define BUTTON_DEBOUNCE_MS     25
#define BUTTON_PRESSED_LEVEL   GPIO_PIN_RESET

/* Static Variables */
static GPIO_TypeDef    *s_port = NULL;
static uint16_t        s_pin;
static GPIO_PinState   s_last_raw;             // previous raw sample
static GPIO_PinState   s_debounced;            // the accepted state
static uint32_t        s_last_change_ms;       // timestamp of the last raw change
static bool            s_press_event;          // latched consume-on-read flag

/* Stores the button GPIO, seeds the debounce state from the current pin
 * level (so a button idle at boot produces no phantom press), and clears
 * the latched event. */
void button_init(GPIO_TypeDef *port, uint16_t pin)
{
    s_port = port;
    s_pin  = pin;

    // start settled to avoid a phantom press at boot
    GPIO_PinState current_raw = HAL_GPIO_ReadPin(s_port, s_pin);
    s_last_raw  = current_raw;
    s_debounced = current_raw;

    s_press_event = false;
    s_last_change_ms = 0;
}

/* Samples the pin, runs the accept-after-stable debounce, and latches a
 * press event when the button goes from released to pressed. Non-blocking; call every loop. */
void button_update(uint32_t now_ms)
{
    if (s_port == NULL)
    {
        return;
    }

    GPIO_PinState current_raw = HAL_GPIO_ReadPin(s_port, s_pin);

    // raw level changed so restart the stability timer
    if (current_raw != s_last_raw)
    {
        s_last_raw = current_raw;
        s_last_change_ms = now_ms;
    }

    // raw level held steady for the full debounce window so accept it
    if ((now_ms - s_last_change_ms) >= BUTTON_DEBOUNCE_MS)
    {
        if (current_raw != s_debounced)
        {
            s_debounced = current_raw;

            // button just went from released to pressed, so latch the event
            if (s_debounced == BUTTON_PRESSED_LEVEL)
            {
                s_press_event = true;
            }
        }
    }
}

/* Returns the latched press event and clears it (consume-on-read). */
bool button_pressed(void)
{
    bool was_pressed = s_press_event;
    s_press_event = false;
    return was_pressed;
}
