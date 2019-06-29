/* See LICENSE file for copyright and license details. */

/* interval between updates (in ms) */
static const int interval = 1000;

/* text to show if no value can be retrieved */
static const char unknown_str[] = "n/a";

static const int battery_urgent = 10;
static const int battery_low = 25;

static const char sep[] =  "  ";

/* maximum output string length */
#define MAXLEN 2048

static const struct arg args[] = {
    /* function format          argument */
    { mail_status, "%s", "/home/wes/Mail/INBOX/new" },
    { gap, sep, NULL},

    { vpn_status,  "%s ", "tun0" },
    { wifi_essid,  "%s ", "wlan0" }, 
    { wifi_perc,   "%s%%", "wlan0" }, 
    { gap, sep, NULL},

    { battery_perc,  "%s", "BAT1" },
    { gap, sep, NULL},

    { vol_perc, "%s%%", "/dev/mixer" }, 
    { gap, sep, NULL},

    { datetime, "%s", "%a %d %b %Y %H:%M" },
    { gap, sep, NULL},
};
