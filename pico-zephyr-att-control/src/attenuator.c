#include "attenuator.h"
#include "att_config.h"
#include "att_codec.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <errno.h>
static const struct device *gpio = DEVICE_DT_GET(DT_NODELABEL(gpio0));
static const unsigned pins[ATT_CELL_COUNT] = ATT_GPIO_PINS;
K_MUTEX_DEFINE(att_lock);
static unsigned current_db;
static uint8_t current_mask;
static int io_error = -EAGAIN;
static int64_t last_change;
int att_init(void)
{
    k_mutex_lock(&att_lock, K_FOREVER);
    int rc = device_is_ready(gpio) ? 0 : -ENODEV;
    for (unsigned i = 0; !rc && i < ATT_CELL_COUNT; ++i)
        rc = gpio_pin_configure(gpio, pins[i],
            (ATT_ACTIVE_LOW_MASK & (1u << i)) ? GPIO_OUTPUT_HIGH : GPIO_OUTPUT_LOW);
    io_error = rc;
    last_change = k_uptime_get();
    k_mutex_unlock(&att_lock);
    return rc;
}
int att_set(unsigned db)
{
    uint8_t mask;
    int rc = att_encode(db, &mask);
    if (rc) return rc;
    k_mutex_lock(&att_lock, K_FOREVER);
    if (io_error) { rc = io_error; goto done; }
    if (mask == current_mask) goto done;
    int64_t wait = ATT_CHANGE_INTERVAL_MS - (k_uptime_get() - last_change);
    if (wait > 0) k_msleep((int32_t)wait);
    gpio_port_pins_t select = 0, values = 0;
    for (unsigned i = 0; i < ATT_CELL_COUNT; ++i) {
        select |= BIT(pins[i]);
        if ((mask ^ ATT_ACTIVE_LOW_MASK) & (1u << i)) values |= BIT(pins[i]);
    }
    /* One masked port write: avoids sequential software bit transitions.
     * Mechanical relays still have different operate/release times. */
    rc = gpio_port_set_masked_raw(gpio, select, values);
    last_change = k_uptime_get();
    if (rc) { io_error = rc; goto done; }
    k_msleep(ATT_SETTLE_MS);
    current_db = db;
    current_mask = mask;
done:
    k_mutex_unlock(&att_lock);
    return rc;
}
int att_json(char *out, size_t size)
{
    k_mutex_lock(&att_lock, K_FOREVER);
    int n = snprintf(out, size,
        "{\"attenuation_db\":%u,\"mask\":%u,\"max_db\":%u,"
        "\"ready\":%s,\"gpio_error\":%d,\"feedback\":false}",
        current_db, current_mask, ATT_MAX_DB, io_error ? "false" : "true", io_error);
    k_mutex_unlock(&att_lock);
    return n < 0 || (size_t)n >= size ? -ENOSPC : n;
}
