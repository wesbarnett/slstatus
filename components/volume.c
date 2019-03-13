/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>

#include "../util.h"

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
