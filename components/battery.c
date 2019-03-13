/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>

#include "../util.h"

const int battery_urgent = 10;
const int battery_low = 25;

#if defined(__linux__)
    #include <limits.h>
    #include <stdint.h>
    #include <unistd.h>

    static const char *
    pick(const char *bat, const char *f1, const char *f2, char *path,
         size_t length)
    {
        if (esnprintf(path, length, f1, bat) > 0 &&
            access(path, R_OK) == 0) {
            return f1;
        }

        if (esnprintf(path, length, f2, bat) > 0 &&
            access(path, R_OK) == 0) {
            return f2;
        }

        return NULL;
    }

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
            return bprintf("%c%s%3d%%%c", 0x04, map[i].symbol, perc, 0x01);
        }
        else if (perc <= battery_low) {
            return bprintf("%c%s%3d%%%c", 0x03, map[i].symbol, perc, 0x01);
        }
        else {
            return bprintf("%s%3d%%", map[i].symbol, perc);
        }
    }
#elif defined(__OpenBSD__)
    #include <fcntl.h>
    #include <machine/apmvar.h>
    #include <sys/ioctl.h>
    #include <unistd.h>

    static int
    load_apm_power_info(struct apm_power_info *apm_info)
    {
        int fd;

        fd = open("/dev/apm", O_RDONLY);
        if (fd < 0) {
            warn("open '/dev/apm':");
            return 0;
        }

        memset(apm_info, 0, sizeof(struct apm_power_info));
        if (ioctl(fd, APM_IOC_GETPOWER, apm_info) < 0) {
            warn("ioctl 'APM_IOC_GETPOWER':");
            close(fd);
            return 0;
        }
        return close(fd), 1;
    }

    const char *
    battery_perc(const char *unused)
    {
        struct apm_power_info apm_info;

        if (load_apm_power_info(&apm_info)) {
            return bprintf("%d", apm_info.battery_life);
        }

        return NULL;
    }

    const char *
    battery_state(const char *unused)
    {
        struct {
            unsigned int state;
            char *symbol;
        } map[] = {
            { APM_AC_ON,      "+" },
            { APM_AC_OFF,     "-" },
        };
        struct apm_power_info apm_info;
        size_t i;

        if (load_apm_power_info(&apm_info)) {
            for (i = 0; i < LEN(map); i++) {
                if (map[i].state == apm_info.ac_state) {
                    break;
                }
            }
            return (i == LEN(map)) ? "?" : map[i].symbol;
        }

        return NULL;
    }

    const char *
    battery_remaining(const char *unused)
    {
        struct apm_power_info apm_info;

        if (load_apm_power_info(&apm_info)) {
            if (apm_info.ac_state != APM_AC_ON) {
                return bprintf("%uh %02um",
                                   apm_info.minutes_left / 60,
                               apm_info.minutes_left % 60);
            } else {
                return "";
            }
        }

        return NULL;
    }
#elif defined(__FreeBSD__)
    #include <sys/sysctl.h>

    const char *
    battery_perc(const char *unused)
    {
        int cap;
        size_t len;

        len = sizeof(cap);
        if (sysctlbyname("hw.acpi.battery.life", &cap, &len, NULL, 0) == -1
                || !len)
            return NULL;

        return bprintf("%d", cap);
    }

    const char *
    battery_state(const char *unused)
    {
        int state;
        size_t len;

        len = sizeof(state);
        if (sysctlbyname("hw.acpi.battery.state", &state, &len, NULL, 0) == -1
                || !len)
            return NULL;

        switch(state) {
            case 0:
            case 2:
                return "+";
            case 1:
                return "-";
            default:
                return "?";
        }
    }

    const char *
    battery_remaining(const char *unused)
    {
        int rem;
        size_t len;

        len = sizeof(rem);
        if (sysctlbyname("hw.acpi.battery.time", &rem, &len, NULL, 0) == -1
                || !len
                || rem == -1)
            return NULL;

        return bprintf("%uh %02um", rem / 60, rem % 60);
    }
#endif
