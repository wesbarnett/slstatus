/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>

#include "../util.h"

const int battery_urgent = 10;
const int battery_low = 25;

const char * 
battery_perc(const char *bat) {
    int charge_full_design;
    int charge_now;
    int perc;
    char path[PATH_MAX];
    FILE *fp;

    snprintf(path, sizeof(path), "%s%s%s", "/sys/class/power_supply/", bat, "/charge_full_design");
    fp = fopen(path, "r");
    if (fp == NULL) {
        warn("Failed to open file %s", path);
        return NULL;
    }
    fscanf(fp, "%i", &charge_full_design);
    fclose(fp);

    snprintf(path, sizeof(path), "%s%s%s", "/sys/class/power_supply/", bat, "/charge_now");
    fp = fopen(path, "r");
    if (fp == NULL) {
        warn("Failed to open file %s", path);
        return NULL;
    }
    fscanf(fp, "%i", &charge_now);
    fclose(fp);

    perc = (int)(100.0*((double)charge_now/(double)charge_full_design));

    struct {
        char *state;
        char *symbol;
    } map[] = {
        { "Charging",    "\U0001F50C" },
        { "Discharging", "\U0001F50B" },
        { "Full",        "F" },
        { "Unknown",     "?" },
    };

    size_t i;
    char state[12];

    snprintf(path, sizeof(path), "%s%s%s", "/sys/class/power_supply/", bat, "/status");
    if (pscanf(path, "%12s", state) != 1) {
        return NULL;
    }

    for (i = 0; i < LEN(map); i++) {
        if (!strcmp(map[i].state, state)) {
            break;
        }
    }

    if (perc <= battery_urgent) {
        return bprintf("%c  %s%3d%%  %c", 0x04, map[i].symbol, perc, 0x01);
    }
    else if (perc <= battery_low) {
        return bprintf("%c  %s%3d%%  %c", 0x03, map[i].symbol, perc, 0x01);
    }
    else {
        return bprintf("  %s%3d%%  ", map[i].symbol, perc);
    }
}
