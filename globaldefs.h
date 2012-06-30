#ifndef GLOBALDEFS_H
#define GLOBALDEFS_H

#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define VERSION "made by wushuangzhi and shangguokan"

#define ERR_T_GET     	INT_MIN
#define ERR_FAN_INIT   	-2
#define ERR_CONF_NOFILE	-3
#define ERR_CONF_LOST	-4
#define ERR_CONF_RELOAD	-5
#define ERR_PIDFILE	   	-6
#define ERR_FORK		-7
#define ERR_CONF_MIX	-8
#define ERR_MALLOC		-9
#define ERR_CONF_LOWHIGH -11
#define ERR_CONF_LEVEL	-12
#define ERR_CONF_ORDERLOW -13
#define ERR_CONF_ORDERHIGH -14
#define ERR_CONF_OVERLAP	-15

#ifndef DUMMYRUN
#define PID_FILE "/var/run/thinkfan.pid"
#define IBM_TEMP "/proc/acpi/ibm/thermal"
#define IBM_FAN "/proc/acpi/ibm/fan"
#else
#define PID_FILE "/tmp/thinkfan.pid"
#define IBM_TEMP "/tmp/thermal"
#define IBM_FAN "/tmp/fan"
#endif //DUMMYRUN

// Stolen from the gurus
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

struct limit {
	int level; // this is written to the fan control file.
	int low;   // lower temperature for stepping down to previous level.
	int high;  // upper temperature determines when to step to the next level.
};

struct sensor {
	char *path;
	int bias[16];
};

struct tf_config {
	struct sensor *sensors;
	int num_sensors;
	char *fan;
	struct limit *limits;
	int num_limits;
	int (*get_temp)();
	void (*setfan)();
	int (*init_fan)();
	void (*uninit_fan)();
};


struct tf_config *config;
int errcnt, cur_lvl;
unsigned int chk_sanity, watchdog_timeout;
char *config_file, *prefix, *rbuf,
	quiet, nodaemon, resume_is_safe,
	*oldpwm; // old contents of pwm*_enable, used for uninit_fan()
float bias_level, depulse_tmp;
struct timespec *depulse;
#define DEPULSE_MIN_LVL 1
#define DEPULSE_MAX_LVL 4
#define FALSE 0
#define TRUE !FALSE

#endif
