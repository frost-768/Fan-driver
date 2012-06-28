#ifndef THINKFAN_H

#define THINKFAN_H
#define _GNU_SOURCE

#include <time.h>

#define set_fan cur_lvl = config->limits[lvl_idx].level; \
		if (!quiet && nodaemon) message(LOG_DEBUG, MSG_DBG_T_STAT); \
		config->setfan();

volatile int interrupted;
unsigned int sleeptime;

int run();
int fancontrol();

#endif
