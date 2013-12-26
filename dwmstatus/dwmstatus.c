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

typedef struct {
	const char *name;
	unsigned long long user, userLow, sys, idle;
} core;

//CPU
core *cpu = &(core){"cpu"};

//Temperature
#define THERMAL thermal_zone1

//Memory
const char *prefixes[4] = {"kB", "MB", "GB", "TB"};

//Volume
#define CARD "default"
#define SELEMENT "Master"

//Battery
#define BATTERY "BAT1"

//Time
#define TZ "America/New_York"

Display *dpy;

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
gettemp(const char *thermal) {
	FILE *file;
	int temp;

	char *file_thermal = smprintf("/sys/class/thermal/%s/temp", thermal);
	file = fopen(file_thermal, "r");
	fscanf(file, "%d\n", &temp);
	fclose(file);
	free(file_thermal);

	return temp / 1000;
}

int
getmem() {
	FILE *file;
	int total, free, buffers, cache;

	file = fopen("/proc/meminfo", "r");
	fscanf(file, "MemTotal: %d kB\nMemFree: %d kB\nBuffers: %d kB\nCached: %d kB\n", &total, &free, &buffers, &cache);
	fclose(file);

	return total - free - buffers - cache;
}

int
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
		return -1;

	return volume * 100 / max;
}

int
getbatt(const char *battery) {
	FILE *file;
	int full, now;

	char *file_full = smprintf("/sys/class/power_supply/%s/energy_full", battery);
	file = fopen(file_full, "r");
	fscanf(file, "%d\n", &full);
	fclose(file);
	free(file_full);

	char *file_now = smprintf("/sys/class/power_supply/%s/energy_now", battery);
	file = fopen(file_now, "r");
	fscanf(file, "%d\n", &now);
	fclose(file);
	free(file_now);

	return now * 100 / full;
}

char *
mktimes(const char *fmt, const char *tzname) {
	char buf[129];
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
	setenv("TZ", tzname, 1);
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
		int load = getcore(cpu);
		char *cpu = smprintf("CPU:\x06 %d%%", load);

		int temperature = gettemp(THERMAL);
		char *temp = smprintf("TEMP:\x06 %d" "\xB0" "C", temperature);

		float memused = getmem();
		int memprefix = 0;
		while(memused > 1024) {
			memused /= 1024;
			memprefix++;
		}
		char *mem = smprintf("MEM:\x06 %.1f %s", memused, prefixes[memprefix]);

		int volume = getvol(CARD, SELEMENT);
		char *vol = volume < 0 ? smprintf("VOL:\x06 ---") : smprintf("VOL:\x06 %d%%", volume);

#ifdef BATTERY
		int battery = getbatt(BATTERY);
		char *batt = smprintf("%sBATT:%s %d%%", battery <= 5 ? "\x04" : "", battery <= 5 ? "" : "\x06", battery);
#endif

		char *time = mktimes("%A %d %B %H:%M", TZ);

#ifdef BATTERY
		char *status = smprintf("\x05[\x01 %s\x05 ] [\x01 %s\x05 ] [\x01 %s\x05 ] [\x01 %s\x05 ] [\x01 %s\x05 ] [\x01 %s\x05 ]", cpu, temp, mem, vol, batt, time);
#else
		char *status = smprintf("\x05[\x01 %s\x05 ] [\x01 %s\x05 ] [\x01 %s\x05 ] [\x01 %s\x05 ] [\x01 %s\x05 ]", cpu, temp, mem, vol, time);
#endif
		setstatus(status);

		free(cpu);
		free(temp);
		free(mem);
		free(vol);
#ifdef BATTERY
		free(batt);
#endif
		free(time);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}
