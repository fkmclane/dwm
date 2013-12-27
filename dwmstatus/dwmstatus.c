#define _BSD_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "data.h"
#include "util.h"

#include "config.h"

Display *dpy;

int
main(void) {
	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	char status[512];

	for (;;usleep(interval * 1000)) {
		initstatus(status);

		if (cpu[0].name != NULL) {
			for (int i = 0; i < LENGTH(cpu); i++)
				addstatus(status, "%s:\x06 %d%%", cpu[i].name, getcore(&cpu[i]));
		}

		if (thermal[0].name != NULL) {
			for (int i = 0; i < LENGTH(thermal); i++)
				addstatus(status, "%s:\x06 %d""\xB0""C", thermal[i].name, gettemp(thermal[i].id));
		}

		float mem_v = getmem();
		int mem_u = 0;
		while (mem_v > 512 && mem_u < LENGTH(units)) {
			mem_v /= 1024;
			mem_u++;
		}
		addstatus(status, "MEM:\x06 %.1f %s", mem_v, units[mem_u]);

		if (strlen(card) > 0 && strlen(selement) > 0) {
			int vol_v = getvol(card, selement);
			if (vol_v < 0)
				addstatus(status, "VOL:\x06 ---");
			else
				addstatus(status, "VOL:\x06 %d%%", vol_v);
		}

		if (strlen(batt) > 0) {
			int batt_v = getbatt(batt);
			if (batt_v > 5)
				addstatus(status, "BATT:\x06 %d%%", batt_v);
			else
				addstatus(status, "\x04""BATT: %d%%", batt_v);
		}

		if (tz[0].name != NULL) {
			char buf[64];
			for (int i = 0; i < LENGTH(tz); i++) {
				mktimes(buf, fmt, tz[i].id);
				if(strlen(tz[i].name) > 0)
					addstatus(status, "%s (%s)", buf, tz[i].name);
				else
					addstatus(status, "%s", buf);
			}
		}

		setstatus(dpy, status);
	}

	XCloseDisplay(dpy);

	return 0;
}
