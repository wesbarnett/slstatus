/* See LICENSE file for copyright and license details. */
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "../util.h"

const char *
battery_perc(const char *bat)
{
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

    return bprintf("%d", perc);
}

const char *
battery_power(const char *bat)
{
	int watts;
	char path[PATH_MAX];

	snprintf(path, sizeof(path), "%s%s%s", "/sys/class/power_supply/", bat, "/power_now");
	return (pscanf(path, "%i", &watts) == 1) ?
	       bprintf("%d", (watts + 500000) / 1000000) : NULL;
}

const char *
battery_state(const char *bat)
{
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
	char path[PATH_MAX], state[12];

	snprintf(path, sizeof(path), "%s%s%s", "/sys/class/power_supply/", bat, "/status");
	if (pscanf(path, "%12s", state) != 1) {
		return NULL;
	}

	for (i = 0; i < LEN(map); i++) {
		if (!strcmp(map[i].state, state)) {
			break;
		}
	}
	return (i == LEN(map)) ? "?" : map[i].symbol;
}
