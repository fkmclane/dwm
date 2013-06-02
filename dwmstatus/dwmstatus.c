#define _BSD_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xlib.h>

#include <alsa/asoundlib.h>

const char *tz = "America/New_York";

const char *card = "default";
const char *selement = "Master";

typedef struct {
	const char *name;
	unsigned long long user, userLow, sys, idle;
} core;

core *cpu = &(core){"cpu"};

Display *dpy;

const char *prefixes[4] = {"kB", "MB", "GB", "TB"};

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

int
getcore(core *cpu) {
	float percent;
	FILE *file;
	char name[10];
	unsigned long long user, userLow, sys, idle, total;
	int result;

	file = fopen("/proc/stat", "r");
	do {
		result = fscanf(file, "%s %llu %llu %llu %llu\n", name, &user, &userLow, &sys, &idle);
	} while(strcmp(name, cpu->name) != 0 && result != EOF);
	fclose(file);

	if (user < cpu->user || userLow < cpu->userLow || sys < cpu->sys || idle < cpu->idle){
		return -1;
	}
	else{
		total = (user - cpu->user) + (userLow - cpu->userLow) + (sys - cpu->sys);
		percent = total;
		total += (idle - cpu->idle);
		percent /= total;
		percent *= 100;
	}

	cpu->user = user;
	cpu->userLow = userLow;
	cpu->sys = sys;
	cpu->idle = idle;

	return (int)percent;
}

int
gettemp() {
	FILE *file;
	int temp;

	file = fopen("/sys/devices/virtual/thermal/thermal_zone0/temp", "r");
	fscanf(file, "%d\n", &temp);
	fclose(file);

	return temp / 1000;
}

char *
getmem() {
	FILE *file;
	float kilobytes;
	int total, free, buffers, cache;
	int prefix = 0;

	file = fopen("/proc/meminfo", "r");
	fscanf(file, "MemTotal: %d kB\nMemFree: %d kB\nBuffers: %d kB\nCached: %d kB\n", &total, &free, &buffers, &cache);
	fclose(file);

	kilobytes = total - free - buffers - cache;

	while(kilobytes > 1024) {
		kilobytes /= 1024;
		prefix++;
	}

	return smprintf("%.1f %s", kilobytes, prefixes[prefix]);
}

int
getbatt() {
	FILE *file;
	int full, now;

	file = fopen("/sys/bus/acpi/drivers/battery/PNP0C0A:00/power_supply/BAT0/charge_full", "r");
	fscanf(file, "%d\n", &full);
	fclose(file);

	file = fopen("/sys/bus/acpi/drivers/battery/PNP0C0A:00/power_supply/BAT0/charge_now", "r");
	fscanf(file, "%d\n", &now);
	fclose(file);

	return now * 100 / full;
}

char *
getvol(const char *card, const char *selement) {
	snd_mixer_t *mixer;
	snd_mixer_selem_id_t *sid;
	snd_mixer_elem_t *element;
	long min, max;

	long volume;
	int muted;

	snd_mixer_open(&mixer, 1);
	snd_mixer_attach(mixer, card);
	snd_mixer_selem_register(mixer, NULL, NULL);
	snd_mixer_load(mixer);
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selement);
	element = snd_mixer_find_selem(mixer, sid);
	snd_mixer_selem_get_playback_volume_range(element, &min, &max);

	snd_mixer_selem_get_playback_volume(element, SND_MIXER_SCHN_FRONT_LEFT, &volume);
	snd_mixer_selem_get_playback_switch(element, SND_MIXER_SCHN_FRONT_LEFT, &muted);

	snd_mixer_close(mixer);

	if (muted == 0)
		return smprintf("---");

	return smprintf("%d%%", 100 * volume / max);
}

void
settz(const char *tzname) {
	setenv("TZ", tzname, 1);
}

char *
mktimes(const char *fmt, const char *tzname) {
	char buf[129];
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		perror("localtime");
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	return smprintf("%s", buf);
}

void
setstatus(char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

int
main(void) {
	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(2)) {
		int cpuload = getcore(cpu);
		int cputemp = gettemp();
		char *memused = getmem();
		int battery = getbatt();
		char *volume = getvol(card, selement);
		char *time = mktimes("%A %d %B %I:%M %p", tz);
		char *status = smprintf("\x05[\x01 CPU:\x06 %d%%\x05 ] [\x01 TEMP:\x06 %d" "\xB0" "C\x05 ] [\x01 MEM:\x06 %s\x05 ] [\x01 %sBATT:%s %d%%\x05 ] [\x01 VOL:\x06 %s\x05 ] [\x01 %s\x05 ]\n", cpuload, cputemp, memused, battery <= 5 ? "\x04" : "", battery <= 5 ? "" : "\x06", battery, volume, time);
		setstatus(status);
		free(memused);
		free(time);
		free(volume);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}
