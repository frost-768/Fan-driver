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


int get_temp_ibm() {
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


void skip_space(char **input) {
	char *tmp = char_cat(input, space, 0);
	free(tmp);
}