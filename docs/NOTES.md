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

