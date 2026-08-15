/**
 * @file    sys_fsm.h
 * @brief   Finite State Machine header
 *
 * Defines the system states and transitions for task handling.
 * Includes init, update, and get_state
 */

#ifndef INC_SYS_FSM_H_
#define INC_SYS_FSM_H_

/* Includes */
#include "stm32l4xx_hal.h"

// FSM States ENUM
typedef enum {
    SYS_STATE_IDLE = 0,
    SYS_STATE_DRIVING,
    SYS_STATE_AVOIDANCE,
    SYS_STATE_SAMPLING,
    SYS_STATE_ABORTING,
    SYS_STATE_LOW_BATTERY,
    SYS_STATE_FAULT,
    SYS_STATE_SLEEP,
    SYS_STATE_COUNT
} sys_state_t;

/* Function Prototypes */

/**
 * @brief   Initialize the Finite State Machine
 * @param   now_ms - current time (milliseconds)
 */
void sys_fsm_init(uint32_t now_ms);

/**
 * @brief   Update the Finite State Machine
 * @param   now_ms - current time (milliseconds)
 */
void sys_fsm_update(uint32_t now_ms);

/**
 * @brief   Get the current state of the Finite State Machine
 */
sys_state_t sys_fsm_get_state(void);

#endif
