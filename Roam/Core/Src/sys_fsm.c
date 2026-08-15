/**
 * @file    sys_fsm.c
 * @brief   Finite State Machine implementation
 *
 * Implements the logic for managing the system's finite state machine,
 * including state transitions and event handling.
 *
 * Requirements: NFR_002, FR_003, FR_004, FR_025, FR_035, FR_037, FR_040
 */

/* Includes */
#include <stdbool.h>

#include "sys_fsm.h"
#include "button.h"
#include "led_status.h"
#include "uart_log.h"

/* Function Prototypes */
static bool fsm_transition(uint32_t now_ms, sys_state_t next, const char *reason);
static bool fsm_handle_button(uint32_t now_ms, sys_state_t short_next, sys_state_t long_next);

/* Static Variables */
static uint32_t     s_state_entered_ms;     // tick the current state was entered, for state timeouts
static sys_state_t  s_state;

static const char *state_names[SYS_STATE_COUNT] = {
    [SYS_STATE_IDLE]        = "IDLE",
    [SYS_STATE_DRIVING]     = "DRIVING",
    [SYS_STATE_AVOIDANCE]   = "AVOIDANCE",
    [SYS_STATE_SAMPLING]    = "SAMPLING",
    [SYS_STATE_ABORTING]    = "ABORTING",
    [SYS_STATE_LOW_BATTERY] = "LOW_BATTERY",
    [SYS_STATE_FAULT]       = "FAULT",
    [SYS_STATE_SLEEP]       = "SLEEP",
};

static const led_state_t state_leds[SYS_STATE_COUNT] = {
    [SYS_STATE_IDLE]        = LED_STATE_IDLE,
    [SYS_STATE_DRIVING]     = LED_STATE_DRIVING,
    [SYS_STATE_AVOIDANCE]   = LED_STATE_DRIVING,
    [SYS_STATE_SAMPLING]    = LED_STATE_SAMPLING,
    [SYS_STATE_ABORTING]    = LED_STATE_SAMPLING,
    [SYS_STATE_LOW_BATTERY] = LED_STATE_LOW_BATTERY,
    [SYS_STATE_FAULT]       = LED_STATE_FAULT,
    [SYS_STATE_SLEEP]       = LED_STATE_SLEEP,
};

/* Function Definitions */

/* System starts in idle with matching LED. Boot is not a transition,
 * so it sets the state directly (doesn't go through fsm_transition). */
void sys_fsm_init(uint32_t now_ms)
{
    s_state_entered_ms = now_ms;
    s_state = SYS_STATE_IDLE;
    led_status_set(state_leds[SYS_STATE_IDLE]);
    uart_log_write(LOG_TAG_SYS, "event=BOOT  state=IDLE");
}

/* Runs the state machine one step. Non-blocking; call every loop. */
void sys_fsm_update(uint32_t now_ms)
{
    // TODO - Global Guards
    // FR_038/FR_039 battery check
    // Subsystem fault check

    /* Each case handles everything that can move the system out of that state.
     * Button first so the operator overrides whatever the rover was doing. */
    switch (s_state)
    {
        case SYS_STATE_IDLE:
            if (fsm_handle_button(now_ms, SYS_STATE_DRIVING, SYS_STATE_SLEEP)) { break; }
            break;

        case SYS_STATE_DRIVING:
            if (fsm_handle_button(now_ms, SYS_STATE_ABORTING, SYS_STATE_SLEEP)) { break; }
            // TODO FR_009: obstacle detected -> AVOIDANCE
            // TODO FR_011: travel interval reached -> SAMPLING
            break;

        case SYS_STATE_AVOIDANCE:
            if (fsm_handle_button(now_ms, SYS_STATE_ABORTING, SYS_STATE_SLEEP)) { break; }
            // TODO FR_009: path clear -> DRIVING, full 360 sweep with no clear path -> FAULT
            break;

        case SYS_STATE_SAMPLING:
            if (fsm_handle_button(now_ms, SYS_STATE_ABORTING, SYS_STATE_SLEEP)) { break; }
            // TODO FR_016: valid reading captured and probe stowed -> DRIVING
            // TODO FR_005: Nth sample collected -> SLEEP
            // TODO FR_037: second deploy timeout -> FAULT
            break;

        case SYS_STATE_ABORTING:
            // swallows both presses. shutting down mid-abort would strand the probe.
            if (fsm_handle_button(now_ms, SYS_STATE_ABORTING, SYS_STATE_ABORTING)) { break; }
            // TODO FR_004/FR_015: probe stow travel elapsed -> IDLE
            break;

        case SYS_STATE_LOW_BATTERY:
            if (fsm_handle_button(now_ms, SYS_STATE_LOW_BATTERY, SYS_STATE_SLEEP)) { break; }
            // TODO FR_039: warning hold duration elapsed -> SLEEP
            break;

        case SYS_STATE_FAULT:
            if (fsm_handle_button(now_ms, SYS_STATE_IDLE, SYS_STATE_SLEEP)) { break; }
            break;

        case SYS_STATE_SLEEP:
            if (fsm_handle_button(now_ms, SYS_STATE_SLEEP, SYS_STATE_SLEEP)) { break; }
            break;

        case SYS_STATE_COUNT:
            break;
    }
}

/*
 * Transition the FSM to a new state.
 * @param  now_ms      Current system tick in milliseconds.
 * @param  next        The state to transition to.
 * @param  reason      The reason for the transition.
 * @return             True if the transition occurred, false otherwise.
 */
static bool fsm_transition(uint32_t now_ms, sys_state_t next, const char *reason)
{
    // don't transition from same state
    if (next == s_state)
    {
        return false;
    }

    uart_log_write(LOG_TAG_SYS, "event=STATE_CHANGE  from=%s  to=%s  reason=%s", state_names[s_state], state_names[next], reason);
    led_status_set(state_leds[next]);
    s_state = next;
    s_state_entered_ms = now_ms;
    return true;
}

/**
 * Handle button events and update the FSM state accordingly.
 *
 * @param  now_ms      Current system tick in milliseconds.
 * @param  short_next  State to transition to on a short press.
 * @param  long_next   State to transition to on a long press.
 * @return             True if a transition occurred, false otherwise.
 */
static bool fsm_handle_button(uint32_t now_ms, sys_state_t short_next, sys_state_t long_next)
{
    if (button_long_pressed())
    {
        return fsm_transition(now_ms, long_next, "LONG_PRESS");
    }

    if (button_pressed())
    {
        if (short_next == s_state)
        {
            return false;
        }

        return fsm_transition(now_ms, short_next, "BUTTON");
    }

    return false;
}

/**
 * Get the current state of the FSM.
 *
 * @return  The current system state.
 */
sys_state_t sys_fsm_get_state(void)
{
    return s_state;
}
