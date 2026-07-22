=== 4/28/2026 ===
uart_log header file created. Public API for the UART logging module is now defined.
Commit: df8b5f3

-- log_tag_t enum (IF_012) --
What: enumerates the five log categories (IMU, TEMP, HUM, SOIL, SYS) plus a LOG_TAG_COUNT trailing sentinel node.
Decided: Tag is an enum, not a string.
Reasons: IF_012 fixes the tag set. Enum makes typos a compile error and lets the .c file own the padding/format
         in one lookup table instead of every caller getting it perfectly.

-- uart_log_init --
What: sets up the module by accepting a UART handle pointer.
Decided: caller passes the handle in (e.g. uart_log_init(&huart1)) rather than the module hardcoding a global.
Reasons: Decouples the driver from the specific UART. When we change to final board selection, only main.c changes.

-- uart_log_write --
What: writes one tagged, timestamped log line to UART
Decided: printf-style variadic signature
Reason: call sites stay one line; the module owns all formatting,
        so the line format is consistent across every caller.

Decided: timestamp captured inside the function via HAL_GetTick(),
         not passed in by the caller.
Reason: guarantees IF_011 compliance; callers can't forget the
        timestamp or format it differently.

=== 5/1/2026 ===
uart_log.c implementation completed. Module is functional. Takes tag + format + values,
produces tagged timestamped line, transmits over UART. Not yet integrated into main.c
Commit: bb57df5

-- buffers -- 
What: two static file scope buffers (s_buf (128 bytes) and s_user_msg(96 bytes))
Decided: two buffers instead of one shared buffer.
Reasons: easier to read and debug, automatic truncation handling via vsnprintf/snprintf bounds,
         no manual pointer math. RAM tradeoff here/now is fine.

-- variadic --
What: uart_log_write uses va_list / va_start / va_end with vsnprintf to expand
      the caller's format string and arguments in s_user_msg.
Reasons: vsnprintf does the format string parsing, type conversions, and bounds-checked writing in one call.

-- log line assembly --
What: snprintf builds the full line "[TAG]t=<ms>  <user msg>\r\n" in one call. 
Decided: Tag padding lives in the lookup table (each entry padded to 7 characters), 
         not in the format string.

-- failure handling --
What: every failure mode in uart_log_write results in silent return
Decided: logging code never escalates failures
Reasons: silent failure means missing log lines are the symptom, which is observable.

-- transmit strategy --
What: blocking HAL_UART_Transmit with HAL_MAX_DELAY timeout
Decided: this is fine during bring-up and pre-state-machine development
Reasons: Simple. Logger is the only thing using this UART.

-- deferred --
Non-blocking transmit (DMA + ring buffer) when state machine lands per NFR_001
SD card mirroring per FR_026

=== 5/2 ===
led_status (.h/.c) have been completed. Not tested with board yet. 

-- LED_BLINK_INTERVAL_MS --
adjust this speed to find the right idle / battery toggling.

-- init --
copies the gpio descriptors by dereferencing the caller's pointers into static variables. Then drives all pins low so we ensure the starting state

-- s_last_toggle_ms : records when the last toggle happened
-- s_blink_on: records which direction the LED is currently in (on or off)
(both get reset in led_status_set so that entering a blink state always starts a fresh cycle from LED off)

=== 7/21 ===
button (.h/.c) done and tested on hardware. Debounced button press. Verified: one press = one clean event,
green LED blinks (idle) / solid (driving), UART logs each press.

-- detection: polling, not interrupt --
Decided: poll the pin every loop in button_update(now_ms), same pattern as led_status.
Reasons: interrupt gets clunky here - bounce fires the ISR a bunch of times, and you'd need a timer
         inside it to debounce anyway. Polling keeps it simple and non-blocking. 
Deferred: use EXTI on PC13 as a wake-from-sleep source later (polling can't wake a sleeping MCU).

-- debounce: accept-after-stable, 25ms --
Decided: only accept a new state after the pin reads the same for 25ms straight. Timed off now_ms, not a sample count.
Reasons: rejects glitches instead of trusting the first edge. 25ms is past the bounce but way under
         what a human notices. Timing off now_ms (not counting samples) means the window stays 25ms
         even when the loop slows down later. Same reason led_status uses now_ms.

-- event: latched, consume-on-read --
Decided: button_pressed() returns true once per press and clears itself on read. Reading eats the event.
Reasons: the FSM cares about the press moment, not "is it held." Latching means I can't miss it,
         consume-on-read means it can't double-fire off one push. Driver owns the edge detection so
         the FSM doesn't have to.
Deferred: long-press to shutdown. No manual shutdown path in reqs right now (possibly worth adding).




