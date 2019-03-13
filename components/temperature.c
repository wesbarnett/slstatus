/* See LICENSE file for copyright and license details. */
#include <stddef.h>
#include <stdint.h>

#include "../util.h"

const char *
temp(const char *file)
{
    uintmax_t temp;

    if (pscanf(file, "%ju", &temp) != 1) {
            return NULL;
    }

    return bprintf("%ju", temp / 1000);
}
