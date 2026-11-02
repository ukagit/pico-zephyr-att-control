#pragma once
/* Logical cell order, NOT connector pin numbering.
 * 3200-1 profile: eight independent cells, 0..127 dB.
 * Confirm against the actual attenuator and driver before connection.
 * Active high refers to the Pico output AFTER accounting for external driver.
 */
#define ATT_CELL_COUNT 8
#define ATT_CELL_DB {1, 2, 4, 8, 16, 32, 32, 32}
#define ATT_GPIO_PINS {19, 20, 22, 26, 27, 18, 21, 28}
#define ATT_ACTIVE_LOW_MASK 0x00u
#define ATT_MAX_DB 127
#define ATT_CHANGE_INTERVAL_MS 200
#define ATT_SETTLE_MS 10
