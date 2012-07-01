#include "globaldefs.h"
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fandriver.h"
#include "message.h"
#include "config.h" // The system interface is part of the config now...


#define set_fan cur_lvl = config->limits[lvl_idx].level; \
	if (!quiet && nodaemon) \
	config->setfan();


int fancontrol() {//风扇控制函数，监控温度和转速
	int last_temp=0, temp, lvl_idx=0, bias=0, diff=0, b_temp;
	int wt = watchdog_timeout, st = sleeptime;

	prefix = "\n"; // makes the output more readable
	if(config->init_fan()) return ERR_FAN_INIT;//tf_config -->globaldefs.h 调用init_fan()函数指针，如果返回一个正数说明出错

	prefix = "\n"; // It is set to "" by the output macros

	// Set initial fan level...
	lvl_idx = config->num_limits - 1;
	b_temp = temp = config->get_temp();//获取温度

	if (errcnt) return errcnt;

	while ((temp <= config->limits[lvl_idx].low) \
	 && (lvl_idx > 0)) lvl_idx--;
	set_fan;//设置温度

	/**********************************************
	 * Main loop. This is the actual fan control.
	 *没有中断 没有错误就循环
	 **********************************************/
	while(likely(!interrupted && !errcnt)) {
		last_temp = temp; // detect temperature jumps
		sleep(st); // st is the state-dependant sleeptime

		// depending on the command line, this might also call depulse()
		temp = config->get_temp();

		// Write current fan level to IBM_FAN one cycle before the watchdog
		// timeout ends, to let it know we're alive.
		if (unlikely((wt -= st) <= sleeptime)) {
#ifdef DEBUG
			message(LOG_DEBUG, MSG_DBG_T_STAT);
#endif
			config->setfan();
			wt = watchdog_timeout;
		}

		// If temperature increased by more than 2 °C since the
		// last cycle, we try to react quickly.
		diff = temp - last_temp;
		if (unlikely(diff >= 2)) {
			bias = (st * (diff-1)) * bias_level;
			if (st > 2) st = 2;
		}
		else if (unlikely(st < sleeptime)) st++;
		b_temp = temp + bias;

		if (unlikely(b_temp >= config->limits[lvl_idx].high)) {
			while (likely((b_temp >= config->limits[lvl_idx].high) \
			 && (lvl_idx < config->num_limits-1))) lvl_idx++;
			set_fan;
		}
		if (unlikely(b_temp <= config->limits[lvl_idx].low)) {
			while (likely((b_temp <= config->limits[lvl_idx].low) \
			 && (lvl_idx > 0))) lvl_idx--;
			set_fan;
			st = sleeptime;
		}

		// slowly reduce the bias
		if (unlikely(bias != 0)) {
			bias -= (bias_level + 0.1f) * 4;
			if (unlikely(bias < 0)) bias = 0;
		}
	}
	return errcnt;
}



void sigHandler(int signum) {
	switch(signum) {
	case SIGHUP:
		interrupted = signum;
		break;
	case SIGINT:
	case SIGTERM:
		interrupted = signum;
		break;
	}
}



int main(int argc, char **argv) {
	char ourlvl ;
	int opt, ret;
	char *invalid = "";
	struct sigaction handler;
       char *input = NULL,*fname = "fan";
	void *map_start;
	struct stat sb;
        int fd,i;
        int temp1;

	rbuf = NULL;
	depulse_tmp = 0;
	sleeptime = 5;
	quiet = 0;
	chk_sanity = 1;
	bias_level = 0.5f;
	ret = 0;
	config_file = "/etc/thinkfan.conf";
	nodaemon = 0;
	errcnt = 0;
	resume_is_safe = 0;
	depulse = NULL;
	prefix = "\n";
	oldpwm = NULL;
	cur_lvl = -1;

	openlog("fandriver", LOG_CONS, LOG_USER);
	syslog(LOG_INFO, "fandriver " VERSION " starting...");

	interrupted = 0;
	memset(&handler, 0, sizeof(handler));//将handler设置为0
	handler.sa_handler = sigHandler;
	if (sigaction(SIGHUP, &handler, NULL) \
	 || sigaction(SIGINT, &handler, NULL) \
	 || sigaction(SIGTERM, &handler, NULL)) perror("sigaction");

	while ((opt = getopt(argc, argv, "c:s:b:p::hnqDzd")) != -1) {
		switch(opt) {
		case 'h':
			fprintf(stderr, MSG_USAGE);
			return 0;
			break;
		case 'n':
			nodaemon = 1;//设置参数nodaemon
			break;
		case 'c':
			config_file = optarg;
			break;
		case 'q':
			quiet = 1;
			break;
		case 'D':
			chk_sanity = 0;
			break;
		case 's':
			sleeptime = (unsigned int) strtol(optarg, &invalid, 0);
			if (*invalid != 0) {
				message_fg(LOG_ERR, MSG_ERR_OPT_S);
				message_fg(LOG_INFO, MSG_USAGE);
				return 1;
			}
			break;
		case 'b':			
			temp1 = (unsigned int)strtol(optarg, &invalid, 0);
			ourlvl = (char)(temp1+48);
			if (*invalid != 0) {
				//report(LOG_ERR, LOG_ERR, MSG_ERR_OPT_S);
				//report(LOG_ERR, LOG_INFO, MSG_USAGE);
				//ret = 1;
				return 0;
			}
	    		//setfan_ibm();
			fd = open(fname, O_RDWR,S_IRWXG|S_IRWXO|S_IRWXU);
			if (fd < 0) {
				//report(LOG_ERR, LOG_ERR, "%s: %s\n", fname, strerror(errno));

				return 0;
			}
			if (fstat(fd, &sb) < 0) {
				//report(LOG_ERR, LOG_ERR, "%s: %s\n", fname, strerror(errno));
				return 0;
			}
			map_start = (char *) mmap(NULL, sb.st_size,
					PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
			if (map_start == MAP_FAILED) {
				//report(LOG_ERR, LOG_ERR, "%s: %s\n", fname, strerror(errno));
				printf("122");
				return 0;
			}
			input = (char *) map_start;
			for(i = 0;i < strlen(input);i++){
				if(input[i] == 'l' && input[i+1] == 'e' && input[i+2] == 'v'){
					input[i+8] = ourlvl;
					break;			
				}
		
			}
			cur_lvl = input;
			setfan_ibm();
			int ibm_fan,l;
			umask(0);
			l = strlen(input);
			if (unlikely((ibm_fan = open(IBM_FAN, O_RDWR, O_TRUNC,S_IRWXG|S_IRWXO|S_IRWXU)) < 0)) {
				//report(LOG_ERR, LOG_ERR, IBM_FAN ": %s\n", strerror(errno));
				//errcnt |= ERR_FAN_SET;
				return 0;
			}
			else {
				if (unlikely(write(ibm_fan, input, l) < l)) {
					//report(LOG_ERR, LOG_ERR, MSG_ERR_FANCTRL);
					//errcnt |= ERR_FAN_SET;
					printf("123");
					return 0;
				}
				close(ibm_fan);
			}
			printf("%s\n",input);
		break;
		default:
			fprintf(stderr, MSG_USAGE);
			return 1;
		}
	}

	if (sleeptime > 15) {
		if (chk_sanity) {
			message_fg(LOG_ERR, MSG_WRN_SLEEPTIME_15);
			message_fg(LOG_ERR, MSG_INF_SANITY);
			return 1;
		}
		else {
			message_fg(LOG_WARNING, MSG_WRN_SLEEPTIME_15);
			message_fg(LOG_WARNING, MSG_INF_INSANITY);
		}
	}
	else if (sleeptime < 1) {
		if (chk_sanity) {
			message_fg(LOG_ERR, MSG_WRN_SLEEPTIME_1);
			message_fg(LOG_ERR, MSG_INF_SANITY);
			return 1;
		}
		else {
			message_fg(LOG_WARNING, MSG_WRN_SLEEPTIME_1);
			message_fg(LOG_WARNING, MSG_INF_INSANITY);
		}
	}
	watchdog_timeout = sleeptime * 6;

	ret = run();

	free(rbuf);
	free(depulse);
	free(oldpwm);
	return ret;
}


int run() {
	int ret = 0, childpid;
	struct tf_config *newconfig=NULL;
	FILE *pidfile;
	rbuf = malloc(sizeof(char) * 128);

	prefix = "\n";

	if ((config = readconfig(config_file)) == NULL) {
		message(LOG_ERR, MSG_ERR_CONF_NOFILE);
		return ERR_CONF_NOFILE;
	}

	if (config->init_fan()) {
		ret = ERR_FAN_INIT;
		goto bail;
	}

	if (config->get_temp() == ERR_T_GET) {
		ret = ERR_T_GET;
		goto bail;
	}

	if (chk_sanity && ((pidfile = fopen(PID_FILE, "r")) != NULL)) {
		fclose(pidfile);
		message_fg(LOG_ERR, MSG_ERR_RUNNING);
		ret = ERR_PIDFILE;
		goto bail;
	}

	if (depulse) message(LOG_INFO, MSG_INF_DEPULSE(sleeptime, depulse_tmp));

	// So we try to detect most errors before forking...

	if (!nodaemon) {  //如果本程序可以作为一个后台应用程序
		if ((childpid = fork()) != 0) {
			if (!quiet) fprintf(stderr, "Daemon PID: %d\n", childpid);
			ret = 0;
			goto bail;
		}
		if (childpid < 0) {
			perror("fork()");
			ret = ERR_FORK;
			goto bail;
		}
	}

	if ((pidfile = fopen(PID_FILE, "w+")) == NULL) {
		showerr(PID_FILE);
		ret = ERR_PIDFILE;
		goto bail;
	}
	fprintf(pidfile, "%d\n", getpid());
	fclose(pidfile);

	while (1) {
		interrupted = 0;
		if ((ret = fancontrol())) break;
		else if (interrupted == SIGHUP) {
			message(LOG_DEBUG, MSG_DBG_CONF_RELOAD);
			if ((newconfig = readconfig(config_file)) != NULL) {
				free_config(config);
				config = newconfig;
			}
			else message(LOG_ERR, MSG_ERR_CONF_RELOAD);
		}
		else if (SIGINT <= interrupted && interrupted <= SIGTERM) {
			message(LOG_WARNING, "\nCaught deadly signal. ");
			break;
		}
	}

	message(LOG_WARNING, MSG_INF_TERM);
	config->uninit_fan();

	unlink(PID_FILE);

bail:
	free_config(config);
	return ret;
}

