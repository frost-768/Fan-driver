#ifndef MESSAGE_C
#define MESSAGE_C

#define showerr(cause) { \
	int lasterr = errno; \
	if (nodaemon || isatty(fileno(stderr))) { \
		fputs(prefix, stderr); \
		fprintf(stderr, "%s: %s\n", cause, strerror(lasterr)); \
	} \
	syslog(LOG_ERR, "%s: %s\n", cause, strerror(lasterr)); \
	prefix = ""; \
}

#define message(lvl, message) { \
	if ( (nodaemon || isatty(fileno(stderr))) \
			&& (!quiet || lvl > LOG_WARNING) ) { \
		fputs(prefix, stderr); \
		fprintf(stderr, message); \
	} \
	else syslog(lvl, message); \
	prefix = ""; \
}

#define message_fg(lvl, message) \
	if (!quiet || lvl > LOG_WARNING) { \
		fputs(prefix, stderr); \
		fprintf(stderr, message); \
	} \
	prefix = "";

/*
*n命令，使程序不会成为一个后台运行程序
*/

#define MSG_USAGE \
 "\nproject " VERSION ": our curriculum design\n" \
 "\nUsage: *************************************" \
 "\n       *******our project Fan Driver********" \
 "\n       ***made by Shuangzhi Wu 1090310622***" \
 "\n       *******Guokan Shang 1090310213*******" \
 "\n       *************************************" \
 "\n -h  This help message" \
 "\n -s  Maximum cycle time in seconds (Integer. Default: 5)" \
 "\n -c  Load different configuration file (default: /etc/thinkfan.conf)" \
 "\n -n  Do not become a daemon and log to terminal & syslog" \
 "\n -D  DANGEROUS mode: Disable all sanity checks. May damage your " \
 "hardware!!\n\n"

#define MSG_FILE_HDR(file, num, line) "%s:%d:%s\n", file, num, line

#define MSG_DBG_T_STAT "sleeptime=%d, temp=%d, last_temp=%d, biased_temp=%d" \
 " -> level=%d\n", st, temp, last_temp, b_temp, cur_lvl
#define MSG_DBG_CONF_RELOAD "Received SIGHUP: reloading config...\n"

#define MSG_INF_SANITY "Sanity checks are on. Exiting.\n"
#define MSG_INF_INSANITY "Sanity checks are off. Continuing.\n"
#define MSG_INF_CONFIG \
 "Config as read from %s:\nFan level\tLow\tHigh\n", config_file
#define MSG_INF_CONF_ITEM(level, low, high) " %d\t\t%d\t%d\n", level, low, high
#define MSG_INF_TERM \
 "Cleaning up and resetting fan control.\n"
#define MSG_INF_DEPULSE(delay, time) "Disengaging the fan controller for " \
	"%.3f seconds every %d seconds\n", time, delay

#define MSG_WRN_SLEEPTIME_15 "WARNING: %d seconds of not realizing " \
 "rising temperatures may be dangerous!\n", sleeptime
#define MSG_WRN_SLEEPTIME_1 "A sleeptime of %d seconds doesn't make much " \
 "sense.\n", sleeptime
#define MSG_WRN_SYSFS_SAFE "WARNING: Using safe but wasteful way of settin" \
	"g PWM value. Check README to know more.\n"
#define MSG_WRN_SENSOR_DEFAULT "WARNING: Using default temperature inputs in" \
	" /proc/acpi/ibm/thermal.\n"
#define MSG_WRN_CONF_NOBIAS(t) "WARNING: You have not provided any correction" \
	" values for any sensor, and your fan will only start at %d °C. This can " \
	"be dangerous for your hard drive.\n", t

#define MSG_ERR_T_GET "Error getting temperature.\n"
#define MSG_ERR_MODOPTS \
 "Module thinkpad_acpi doesn't seem to support fan_control\n"
#define MSG_ERR_FANCTRL "Error writing to %s.\n", config->fan
#define MSG_ERR_FAN_INIT "Error initializing fan control.\n"
#define MSG_ERR_OPT_S "ERROR: option -s requires an int argument!\n"
#define MSG_ERR_OPT_B "ERROR: option -b requires a float argument!\n"
#define MSG_ERR_OPT_P "ERROR: invalid argument to option -p: %f\n", depulse_tmp

#define MSG_ERR_CONF_NOFILE "Refusing to run without usable config file." \
 " Please read AND UNDERSTAND the documentation!\n"
#define MSG_ERR_CONF_LOST "Lost configuration! This is a bug. Please " \
 "report this to the author.\n"
#define MSG_ERR_CONF_RELOAD "Error reloading config. Keeping old one.\n"
#define MSG_ERR_CONF_NOFAN "Could not find any fan speed settings in" \
	" the config file. Please read AND UNDERSTAND the documentation!\n"
#define MSG_ERR_CONF_LOWHIGH "LOWER limit must be smaller than HIGHER! " \
	"Really, don't mess with this, it could trash your hardware.\n"
#define MSG_ERR_CONF_OVERLAP "LOWER limit doesn't overlap with previous UPPER" \
	" limit.\n"
#define MSG_ERR_CONF_FAN "Thinkfan can't use more than one fan.\n"
#define MSG_ERR_CONF_MIX "Thinkfan can't use sysfs sensors together with " \
	"thinkpad_acpi sensors. Please choose one.\n"
#define MSG_ERR_CONF_LEVEL "Fan levels are not ordered correctly.\n"
#define MSG_ERR_CONF_PARSE "Syntax error.\n"

#define MSG_ERR_RUNNING PID_FILE " already exists. Either thinkfan is " \
	"already running, or it was killed by SIGKILL. If you're sure thinkfan" \
	" is not running, delete " PID_FILE " manually.\n"
#define MSG_ERR_FANFILE_IBM "Error opening " IBM_FAN ". Is this a computer " \
	"really Thinkpad? Is the thinkpad_acpi module loaded? Are you running thi" \
	"nkfan with root privileges?\n"
#define MSG_ERR_T_PARSE(str) "Error parsing temperatures: %s\n", str

#endif
