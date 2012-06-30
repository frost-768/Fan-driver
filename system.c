#include "globaldefs.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include "message.h"
#include "system.h"
#include "parser.h"

const char temperatures[] = "temperatures:";

int get_temp_ibm() {//������ȡ�¶� ������ߵ��¶�
	int i=0, res, retval=0, ibm_temp, *tmp;
	ssize_t r;
	char *input;
	input = rbuf;

	if (unlikely(((ibm_temp = open(IBM_TEMP, O_RDONLY)) < 0)
			|| ((r = read(ibm_temp, rbuf, 128)) < 14)
			|| (close(ibm_temp) < 0))) {
		showerr(IBM_TEMP);
		errcnt++;
		return ERR_T_GET;
	}
	rbuf[r] = 0;

	skip_space(&input);
	if (likely(parse_keyword(&input, temperatures) != NULL)) {
		for (i = 0; ((tmp = parse_int(&input)) && (i < 16)); i++) {
			res = *tmp + config->sensors->bias[i];
			if (res > retval) retval = res;
			free(tmp);
		}
		if (unlikely(i < 2)) {
			message(LOG_ERR, MSG_ERR_T_GET);
			errcnt++;
			retval = ERR_T_GET;
		}
	}
	else {
		message(LOG_ERR, MSG_ERR_T_PARSE(rbuf));
		errcnt++;
		retval = ERR_T_GET;
	}
	return retval;
}

/***********************************************************
 * Set fan speed (IBM interface).
 ***********************************************************/
void setfan_ibm() {
	int ibm_fan;
	char *buf = malloc(18 * sizeof(char));

	if (unlikely((ibm_fan = open(IBM_FAN, O_RDWR, O_TRUNC)) < 0)) {//���ļ�����򷵻ش�����Ϣ
		showerr(IBM_FAN);
		errcnt++;
	}
	else {
		if (unlikely(cur_lvl == INT_MIN)) strcpy(buf, "level disengaged\n");
		else snprintf(buf, 10, "level %d\n", cur_lvl);
		if (unlikely(write(ibm_fan, buf, 8) != 8)) {
			showerr(IBM_FAN);
			message(LOG_ERR, MSG_ERR_FANCTRL);
			errcnt++;
		}
		close(ibm_fan);
	}
	free(buf);
}

/*********************************************************
 * Checks for fan_control support in thinkpad_acpi and
 * activates the fan watchdog.
 *********************************************************/
int init_fan_ibm() {
	char *line = NULL;
	size_t count = 0;
	FILE *ibm_fan;
	int module_valid=0;//���ã�

	if ((ibm_fan = fopen(IBM_FAN, "r+")) == NULL) {
		showerr(IBM_FAN);
		errcnt++;
		message(LOG_ERR, MSG_ERR_FANFILE_IBM);
		return ERR_FAN_INIT;
	}
	while (getline(&line, &count, ibm_fan) != -1)
		if (!strncmp("commands:", line, 9)) module_valid = 1;
	if (!module_valid) {
		message(LOG_ERR, MSG_ERR_MODOPTS);
		errcnt++;
		return ERR_FAN_INIT;
	}
	fprintf(ibm_fan, "watchdog %d\n", watchdog_timeout);//watchdog_timeout���ã�
	fclose(ibm_fan);
	free(line);
	return 0;
}

/*********************************************************
 * Restores automatic fan control.
 *********************************************************/


 /****************************************************************
 * get_temp_sysfs() reads the temperature from all files that
 * were specified as "sensor ..." in the config file and returns
 * the highest temperature.
 ****************************************************************/
int get_temp_sysfs() {
	int num, fd, idx = 0;
	long int rv = 0, tmp;
	char buf[7];
	char *endptr;
	while(idx < config->num_sensors) {
		if (unlikely((fd = open(config->sensors[idx].path, O_RDONLY)) == -1
				|| (num = read(fd, &buf, 6)) == -1
				|| close(fd) < 0)) {
			showerr(config->sensors[idx].path);
			errcnt++;
			return ERR_T_GET;
		}
		buf[num] = 0;
		tmp = config->sensors[idx].bias[0] + strtol(buf, &endptr, 10)/1000;
		if (tmp > rv) rv = tmp;
		idx++;
	}
	if (unlikely(rv < 1)) {
		message(LOG_ERR, MSG_ERR_T_GET);
		errcnt++;
	}
	return rv;
}

/***********************************************************
 * Set fan speed (sysfs interface).
 ***********************************************************/
void setfan_sysfs() {
	int fan, r;
	ssize_t ret;
	char *buf = malloc(5 * sizeof(char));
	memset(buf, 0, 5);

	if (unlikely((fan = open(config->fan, O_WRONLY)) < 0)) {
		showerr(config->fan);
		errcnt++;
	}
	else {
		r = snprintf(buf, 5, "%d\n", cur_lvl);
		ret = (int)r + write(fan, buf, 5);
		close(fan);
		if (unlikely(ret < 2)) {
			message(LOG_ERR, MSG_ERR_FANCTRL);
			errcnt++;
		}
	}
	free(buf);
}
