/* See LICENSE file for copyright and license details. */
#include <err.h>
#include <ifaddrs.h>
#include <linux/wireless.h>
#include <sys/socket.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../util.h"

const char *
vpn_status(const char *iface)
{
	char path[PATH_MAX];
	char status[5];
	FILE *fp;

	snprintf(path, sizeof(path), "%s%s%s", "/sys/class/net/", iface, "/operstate");

	fp = fopen(path, "r");
	if (fp == NULL) {
		return "down";
	}
	fgets(status, 5, fp);
	fclose(fp);

	if(strcmp(status, "down") == 0) {
		return "down";
	}
    else {
		return "up";
    }

}
