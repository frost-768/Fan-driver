#include "message.h"
#include "globaldefs.h"
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

void report(int nlevel, int dlevel, char *format, ...) {
	va_list ap;
	int level = chk_sanity ? nlevel : dlevel;

	va_start(ap, format);
	if (nodaemon && (!quiet || level > LOG_WARNING)) {
		fputs(prefix, stderr);
		vfprintf(stderr, format, ap);
		prefix = "";
	}
	else {
		vsyslog(level, format, ap);
	}
}
