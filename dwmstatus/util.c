#include <stdio.h>
#include <stdlib.h>

#include "util.h"

char *
smprintf(char *fmt, ...) {
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

void
addstatus(char *status, const char *str) {
	char *temp = status;
	status = smprintf("%s [\x01 %s\x05 ]", temp, str);
	free(temp);
}

void
setstatus(Display *dpy, char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}
