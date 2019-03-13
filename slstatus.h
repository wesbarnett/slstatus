/* See LICENSE file for copyright and license details. */

/* battery */
const char *battery_perc(const char *);

/* datetime */
const char *datetime(const char *fmt);

/* run_command */
const char *run_command(const char *cmd);

/* temperature */
const char *temp(const char *);

/* user */
const char *gid(void);
const char *username(void);
const char *uid(void);

/* volume */
const char *vol_perc(const char *card);

/* wifi */
const char *wifi_perc(const char *interface);
const char *wifi_essid(const char *interface);

/* vpn */
const char *vpn_status(const char *);

/* mail */
const char *mail_status(const char *);
