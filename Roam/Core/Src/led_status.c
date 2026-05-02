/**
 * @file    led_status.c
 * @brief   LED status indicator driver implementation
 *
 * Solid states drive a single LED continuously. Blink states
 * toggle on a timer checked each main-loop iteration via led_status_update.
 *
 * Requirements: FR_030-FR_035, IF_013, NFR_001
 */

/* Includes */
#include "led_status.h"
#include <stdbool.h>

/* Defines */
#define LED_BLINK_INTERVAL_MS 750

/* Function Prototypes */
static void leds_drive(GPIO_PinState green, GPIO_PinState amber, GPIO_PinState red);

/* Static Variables */
static led_gpio_t s_green;
static led_gpio_t s_amber;
static led_gpio_t s_red;
static led_state_t s_state = LED_STATE_IDLE;
static uint32_t s_last_toggle_ms = 0;			// records when the last LED toggle happened
static bool s_blink_on = false;					// records which direction the LED is currently in

/* Copies GPIO descriptors and drives all LEDs off to establish a known starting state. */
void led_status_init(const led_gpio_t *green,
                     const led_gpio_t *amber,
                     const led_gpio_t *red)
{
	s_state = LED_STATE_IDLE;
	s_green = *green;
	s_amber = *amber;
	s_red = *red;

	leds_drive(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
}

/* Resets blink tracking so any blink state begins a fresh cycle. */
void led_status_set(led_state_t state)
{
	s_state = state;
	s_last_toggle_ms = 0;
	s_blink_on = false;
}

/* Drives pins immediately for solid states.
 * For blink states, checks elapsed time against LED_BLINK_INTERVAL_MS
 * and toggles when the interval expires. */
void led_status_update(uint32_t now_ms)
{
	switch (s_state) {
		case LED_STATE_DRIVING:
			// green on, amber & red off
			leds_drive(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
			break;

		case LED_STATE_SAMPLING:
			// amber on, green & red off
			leds_drive(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
			break;

		case LED_STATE_FAULT:
			// red on, green & amber off
			leds_drive(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
			break;

		case LED_STATE_SLEEP:
			// all off
			leds_drive(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
			break;
			
		case LED_STATE_IDLE:
			// toggle blink state if interval has elapsed
			if ((now_ms - s_last_toggle_ms) > LED_BLINK_INTERVAL_MS )
			{
				// flip LED on/off if interval has elapsed
				s_blink_on = !s_blink_on;
				s_last_toggle_ms = now_ms;
			}
			// green follows blink state, amber & red always off
			leds_drive(s_blink_on ? GPIO_PIN_SET : GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
			break;

		case LED_STATE_LOW_BATTERY:
			// toggle blink state if interval has elapsed
			if ((now_ms - s_last_toggle_ms) > LED_BLINK_INTERVAL_MS )
			{
				// flip LED on/off and reset toggle timestamp
				s_blink_on = !s_blink_on;
				s_last_toggle_ms = now_ms;
			}
			// red follows blink state, green & amber off
			leds_drive(GPIO_PIN_RESET, GPIO_PIN_RESET, s_blink_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
			break;
		default:
			break;
	}
}

/* helper function for HAL_GPIO_WritePin calls */
static void leds_drive(GPIO_PinState green, GPIO_PinState amber, GPIO_PinState red)
{
	HAL_GPIO_WritePin(s_green.port, s_green.pin, green);
	HAL_GPIO_WritePin(s_amber.port, s_amber.pin, amber);
	HAL_GPIO_WritePin(s_red.port, s_red.pin, red);
}

