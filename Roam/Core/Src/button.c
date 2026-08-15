/**
 * @file    button.c
 * @brief   Button & debounce driver logic
 *
 * Polls the user button each main-loop iteration and debounces it
 * (accept-after-stable, 25 ms window). Exposes latched, consume-on-read
 * short-press and long-press events for the state machine.
 *
 * A short press resolves on release, so one hold never reports both events.
 *
 * Requirements: NFR_007, IF_008, FR_003, FR_004, FR_040.
 */

/* Includes */
#include "button.h"

/* Defines */
#define BUTTON_DEBOUNCE_MS     25
#define BUTTON_PRESSED_LEVEL   GPIO_PIN_RESET
#define BUTTON_LONG_PRESS_MS   1000

/* Static Variables */
static GPIO_TypeDef    *s_port = NULL;
static uint16_t        s_pin;
static GPIO_PinState   s_last_raw;             // previous raw sample
static GPIO_PinState   s_debounced;            // the accepted state
static uint32_t        s_last_change_ms;       // timestamp of the last raw change
static uint32_t        s_press_start_ms;       // when this hold began
static bool            s_long_fired;           // whether the long event already fired for this hold
static bool            s_long_event;           // long-press flag
static bool            s_press_event;          // latched consume-on-read flag

/* Stores the button GPIO, seeds the debounce state from the current pin
 * level (so a button idle at boot produces no phantom press), and clears
 * the latched events. */
void button_init(GPIO_TypeDef *port, uint16_t pin)
{
    s_port = port;
    s_pin  = pin;

    // start settled to avoid a phantom press at boot
    GPIO_PinState current_raw = HAL_GPIO_ReadPin(s_port, s_pin);
    s_last_raw  = current_raw;
    s_debounced = current_raw;

    s_last_change_ms = 0;
    s_press_start_ms = 0;

    // a hold already in progress at boot is not ours to report, so retire it
    s_long_fired = (current_raw == BUTTON_PRESSED_LEVEL);

    s_long_event = false;
    s_press_event = false;
}

/* Samples the pin, runs the accept-after-stable debounce, and latches whichever
 * event the hold turned out to be. Non-blocking; call every loop. */
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

            if (s_debounced == BUTTON_PRESSED_LEVEL)
            {
                // hold started - which event it becomes is not known yet
                s_press_start_ms = now_ms;
                s_long_fired = false;
            }
            else if (!s_long_fired)
            {
                // released before the threshold, so it was a short press
                s_press_event = true;
            }
        }
    }

    // the long event fires mid-hold, so it is a level check, not an edge check
    if (s_debounced == BUTTON_PRESSED_LEVEL &&
        !s_long_fired &&
        (now_ms - s_press_start_ms) >= BUTTON_LONG_PRESS_MS)
    {
        s_long_event = true;
        s_long_fired = true;
    }
}

/* Returns the latched short-press event and clears it (consume-on-read). */
bool button_pressed(void)
{
    bool was_pressed = s_press_event;
    s_press_event = false;
    return was_pressed;
}

/* Returns the latched long-press event and clears it (consume-on-read). */
bool button_long_pressed(void)
{
    bool was_long_pressed = s_long_event;
    s_long_event = false;
    return was_long_pressed;
}