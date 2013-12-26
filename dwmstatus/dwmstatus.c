#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "util.h"
#include "data.h"

#include "config.h"

Display *dpy;

int
main(void) {
	if (dpy != XOpenDisplay(NULL)) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	int cpu_v, temp_v, mem_u, vol_v, batt_v;
	float mem_v;
	char *cpu_s, *temp_s, *mem_s, *vol_s, *batt_s, *time_s, *status;

	for (;;usleep(interval * 1000)) {
		status = smprintf("\x05");

		for (int i = 0; i < LENGTH(cpu); i++) {
			cpu_v = getcore(&cpu[i]);
			cpu_s = smprintf("%s:\x06 %d%%", cpu[i].name, cpu_v);
			addstatus(status, cpu_s);
			free(cpu_s);
		}

		for (int i = 0; i < LENGTH(thermal); i++) {
			temp_v = gettemp(thermal[i].id);
			temp_s = smprintf("%s:\x06 %d""\xB0""C", thermal[i].name, temp_v);
			addstatus(status, temp_s);
			free(temp_s);
		}

		mem_v = getmem();
		mem_u = 0;
		while (mem_v > 512 && mem_u < LENGTH(units)) {
			mem_v /= 1024;
			mem_u++;
		}
		mem_s = smprintf("MEM:\x06 %.1f %s", mem_v, units[mem_u]);
		addstatus(status, mem_s);
		free(mem_s);

		if (LENGTH(card) > 0 && LENGTH(selement) > 0) {
			vol_v = getvol(card, selement);
			vol_s = vol_v < 0 ? smprintf("VOL:\x06 ---") : smprintf("VOL:\x06 %d%%", vol_v);
			addstatus(status, vol_s);
			free(vol_s);
		}

		if (LENGTH(batt) > 0) {
			batt_v = getbatt(batt);
			if (batt_v > 5)
				batt_s = smprintf("BATT:\x06 %d%%", batt_v);
			else
				batt_s = smprintf("\x04""BATT: %d%%", batt_v);
			addstatus(status, batt_s);
			free(batt_s);
		}

		time_s = mktimes(fmt, tz);
		addstatus(status, time_s);
		free(time_s);

		setstatus(dpy, status);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}
