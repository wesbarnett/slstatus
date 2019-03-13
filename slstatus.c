/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <limits.h>
#include <linux/wireless.h>
#include <pwd.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/soundcard.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "arg.h"
#include "slstatus.h"
#include "util.h"

struct arg {
    const char *(*func)();
    const char *fmt;
    const char *args;
};

char buf[1024];
static int done;
static Display *dpy;

#include "config.h"

static void
terminate(const int signo)
{
    (void)signo;

    done = 1;
}

static void
difftimespec(struct timespec *res, struct timespec *a, struct timespec *b)
{
    res->tv_sec = a->tv_sec - b->tv_sec - (a->tv_nsec < b->tv_nsec);
    res->tv_nsec = a->tv_nsec - b->tv_nsec +
                   (a->tv_nsec < b->tv_nsec) * 1E9;
}

static void
usage(void)
{
    die("usage: %s [-s]", argv0);
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
        return bprintf("%c  %s%3d%%  %c", 0x04, map[i].symbol, perc, 0x01);
    }
    else if (perc <= battery_low) {
        return bprintf("%c  %s%3d%%  %c", 0x03, map[i].symbol, perc, 0x01);
    }
    else {
        return bprintf("  %s%3d%%  ", map[i].symbol, perc);
    }
}

const char *
datetime(const char *fmt)
{
    time_t t;

    t = time(NULL);
    if (!strftime(buf, sizeof(buf), fmt, localtime(&t))) {
        warn("strftime: Result string exceeds buffer size");
        return NULL;
    }

    return buf;
}

const char *
mail_status(const char *maildir)
{

    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;

    dirp = opendir(maildir);
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
             file_count++;
        }
    }
    closedir(dirp);

      return bprintf("%s %d", "\U0001F4E7", file_count);
}

const char *
temp(const char *file)
{
    uintmax_t temp;

    if (pscanf(file, "%ju", &temp) != 1) {
            return NULL;
    }

    return bprintf("%ju", temp / 1000);
}

const char *
username(void)
{
    struct passwd *pw;

    if (!(pw = getpwuid(geteuid()))) {
        warn("getpwuid '%d':", geteuid());
        return NULL;
    }

    return bprintf("%s", pw->pw_name);
}

const char *
vol_perc(const char *card)
{
        size_t i;
        int v, afd, devmask;
        char *vnames[] = SOUND_DEVICE_NAMES;

        if ((afd = open(card, O_RDONLY | O_NONBLOCK)) < 0) {
                warn("open '%s':", card);
                return NULL;
        }

        if (ioctl(afd, (int)SOUND_MIXER_READ_DEVMASK, &devmask) < 0) {
                warn("ioctl 'SOUND_MIXER_READ_DEVMASK':");
                close(afd);
                return NULL;
        }
        for (i = 0; i < LEN(vnames); i++) {
                if (devmask & (1 << i) && !strcmp("vol", vnames[i])) {
                        if (ioctl(afd, MIXER_READ(i), &v) < 0) {
                                warn("ioctl 'MIXER_READ(%ld)':", i);
                                close(afd);
                                return NULL;
                        }
                }
        }

        close(afd);
        if ((v & 0xff) < 1)  {
                return bprintf("\U0001F507 %3d", v & 0xff);
        }
        else if ((v & 0xff) < 10) {
                return bprintf("\U0001F508 %3d", v & 0xff);
        }
        else if ((v & 0xff) < 30) {
                return bprintf("\U0001F508 %3d", v & 0xff);
        }
        else if ((v & 0xff) < 65) {
                return bprintf("\U0001F509 %3d", v & 0xff);
        }
        else {
                return bprintf("\U0001F50A %3d", v & 0xff);
        }
}

const char *
vpn_status(const char *iface)
{
    char path[PATH_MAX];
    char status[5];
    FILE *fp;

    snprintf(path, sizeof(path), "%s%s%s", "/sys/class/net/", iface, "/operstate");

    fp = fopen(path, "r");
    if (fp == NULL) {
        return "\U0001F513";
    }
    fgets(status, 5, fp);
    fclose(fp);

    if(strcmp(status, "down") == 0) {
            return "\U0001F513";
    }
        else {
            return "\U0001F512";
        }
}

const char *
wifi_perc(const char *interface)
{
    int cur;
    size_t i;
    char *p, *datastart;
    char path[PATH_MAX];
    char status[5];
    FILE *fp;

    if (esnprintf(path, sizeof(path), "/sys/class/net/%s/operstate",
                  interface) < 0) {
        return NULL;
    }
    if (!(fp = fopen(path, "r"))) {
        warn("fopen '%s':", path);
        return NULL;
    }
    p = fgets(status, 5, fp);
    fclose(fp);
    if (!p || strcmp(status, "up\n") != 0) {
        return NULL;
    }

    if (!(fp = fopen("/proc/net/wireless", "r"))) {
        warn("fopen '/proc/net/wireless':");
        return NULL;
    }

    for (i = 0; i < 3; i++) {
        if (!(p = fgets(buf, sizeof(buf) - 1, fp)))
            break;
    }
    fclose(fp);
    if (i < 2 || !p) {
        return NULL;
    }

    if (!(datastart = strstr(buf, interface))) {
        return NULL;
    }

    datastart = (datastart+(strlen(interface)+1));
    sscanf(datastart + 1, " %*d   %d  %*d  %*d\t\t  %*d\t   "
           "%*d\t\t%*d\t\t %*d\t  %*d\t\t %*d", &cur);

    /* 70 is the max of /proc/net/wireless */
    return bprintf("%d", (int)((float)cur / 70 * 100));
}

const char *
wifi_essid(const char *interface)
{
    static char id[IW_ESSID_MAX_SIZE+1];
    int sockfd;
    struct iwreq wreq;

    memset(&wreq, 0, sizeof(struct iwreq));
    wreq.u.essid.length = IW_ESSID_MAX_SIZE+1;
    if (esnprintf(wreq.ifr_name, sizeof(wreq.ifr_name), "%s",
                  interface) < 0) {
        return NULL;
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        warn("socket 'AF_INET':");
        return NULL;
    }
    wreq.u.essid.pointer = id;
    if (ioctl(sockfd,SIOCGIWESSID, &wreq) < 0) {
        warn("ioctl 'SIOCGIWESSID':");
        close(sockfd);
        return NULL;
    }

    close(sockfd);

    if (!strcmp(id, "")) {
        return NULL;
    }

    return id;
}

int
main(int argc, char *argv[])
{
    struct sigaction act;
    struct timespec start, current, diff, intspec, wait;
    size_t i, len;
    int sflag, ret;
    char status[MAXLEN];
    const char *res;

    sflag = 0;
    ARGBEGIN {
        case 's':
            sflag = 1;
            break;
        default:
            usage();
    } ARGEND

    if (argc) {
        usage();
    }

    memset(&act, 0, sizeof(act));
    act.sa_handler = terminate;
    sigaction(SIGINT,  &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    if (!sflag && !(dpy = XOpenDisplay(NULL))) {
        die("XOpenDisplay: Failed to open display");
    }

    while (!done) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
            die("clock_gettime:");
        }

        status[0] = '\0';
        for (i = len = 0; i < LEN(args); i++) {
            if (!(res = args[i].func(args[i].args))) {
                res = unknown_str;
            }
            if ((ret = esnprintf(status + len, sizeof(status) - len,
                                args[i].fmt, res)) < 0) {
                break;
            }
            len += ret;
        }

        if (sflag) {
            puts(status);
            fflush(stdout);
            if (ferror(stdout))
                die("puts:");
        } else {
            if (XStoreName(dpy, DefaultRootWindow(dpy), status)
                            < 0) {
                die("XStoreName: Allocation failed");
            }
            XFlush(dpy);
        }

        if (!done) {
            if (clock_gettime(CLOCK_MONOTONIC, &current) < 0) {
                die("clock_gettime:");
            }
            difftimespec(&diff, &current, &start);

            intspec.tv_sec = interval / 1000;
            intspec.tv_nsec = (interval % 1000) * 1E6;
            difftimespec(&wait, &intspec, &diff);

            if (wait.tv_sec >= 0) {
                if (nanosleep(&wait, NULL) < 0 &&
                    errno != EINTR) {
                    die("nanosleep:");
                }
            }
        }
    }

    if (!sflag) {
        XStoreName(dpy, DefaultRootWindow(dpy), NULL);
        if (XCloseDisplay(dpy) < 0) {
            die("XCloseDisplay: Failed to close display");
        }
    }

    return 0;
}
