#include <stdio.h>

#include "util.h"

void
initstatus(char *status) {
	sprintf(status, "\x05");
}

void
addstatus(char *status, const char *fmt, ...) {
	va_list fmtargs;
	char str[64];

	va_start(fmtargs, fmt);
	vsnprintf(str, sizeof(str), fmt, fmtargs);
	va_end(fmtargs);

	sprintf(status, "%s [\x01 %s\x05 ]", status, str);
}

void
splitstatus(char *status) {
	sprintf(status, "%s\n", status);
}

void
setstatus(Display *dpy, char *status) {
	XStoreName(dpy, DefaultRootWindow(dpy), status);
	XSync(dpy, False);
}
