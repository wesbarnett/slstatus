/* See LICENSE file for copyright and license details. */
#include <dirent.h>
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
