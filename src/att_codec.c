#include "att_config.h"
#include "att_codec.h"
#include <errno.h>
#include <stddef.h>
static const unsigned weights[ATT_CELL_COUNT] = ATT_CELL_DB;
unsigned att_decode(uint8_t mask)
{
    unsigned sum = 0;
    for (unsigned i = 0; i < ATT_CELL_COUNT; ++i)
        if (mask & (1u << i)) sum += weights[i];
    return sum;
}
int att_encode(unsigned db, uint8_t *mask)
{
    if (mask == NULL || db > ATT_MAX_DB) return -EINVAL;
    /* Exhaustive enumeration supports repeated 32 dB cells without treating
     * the hardware as an ordinary binary-weighted register. Stable mapping. */
    for (unsigned m = 0; m < (1u << ATT_CELL_COUNT); ++m) {
        if (att_decode((uint8_t)m) == db) { *mask = (uint8_t)m; return 0; }
    }
    return -ERANGE;
}
int att_parse_db(const char *text, unsigned *db)
{
    if (!text || !*text || !db) return -EINVAL;
    unsigned value = 0;
    for (; *text; ++text) {
        if (*text < '0' || *text > '9') return -EINVAL;
        value = value * 10u + (unsigned)(*text - '0');
        if (value > ATT_MAX_DB) return -ERANGE;
    }
    *db = value;
    return 0;
}
