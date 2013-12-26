#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <alsa/asoundlib.h>

#include "data.h"
#include "util.h"

int
getcore(Core *cpu) {
	int percent;
	FILE *file;
	char name[10];
	unsigned long long user, userlow, sys, idle, total;
	int result;

	file = fopen("/proc/stat", "r");
	do {
		result = fscanf(file, "%s %llu %llu %llu %llu\n", name, &user, &userlow, &sys, &idle);
	}
	while(strcmp(name, cpu->name) != 0 && result != EOF);
	fclose(file);

	total = (user - cpu->user) + (userlow - cpu->userlow) + (sys - cpu->sys);
	percent = total * 100 / (total + idle - cpu->idle);

	cpu->user = user;
	cpu->userlow = userlow;
	cpu->sys = sys;
	cpu->idle = idle;

	return percent;
}

int
gettemp(const char *thermal) {
	FILE *file;
	int temp;

	char *fthermal = smprintf("/sys/class/thermal/%s/temp", thermal);
	file = fopen(fthermal, "r");
	fscanf(file, "%d\n", &temp);
	fclose(file);
	free(fthermal);

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

	long vol;
	int mute;

	snd_mixer_open(&mixer, 1);
	snd_mixer_attach(mixer, card);
	snd_mixer_selem_register(mixer, NULL, NULL);
	snd_mixer_load(mixer);
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selement);
	element = snd_mixer_find_selem(mixer, sid);
	snd_mixer_selem_get_playback_volume_range(element, &min, &max);

	snd_mixer_selem_get_playback_volume(element, SND_MIXER_SCHN_FRONT_LEFT, &vol);
	snd_mixer_selem_get_playback_switch(element, SND_MIXER_SCHN_FRONT_LEFT, &mute);

	snd_mixer_close(mixer);

	if (mute == 0)
		return -1;

	return vol * 100 / max;
}

int
getbatt(const char *batt) {
	FILE *file;
	int full, now;

	char *ffull = smprintf("/sys/class/power_supply/%s/energy_full", batt);
	file = fopen(ffull, "r");
	fscanf(file, "%d\n", &full);
	fclose(file);
	free(ffull);

	char *fnow = smprintf("/sys/class/power_supply/%s/energy_now", batt);
	file = fopen(fnow, "r");
	fscanf(file, "%d\n", &now);
	fclose(file);
	free(fnow);

	return now * 100 / full;
}

char *
mktimes(const char *fmt, const char *tz) {
	char buf[128];
	time_t rtime;
	struct tm *itime;

	setenv("TZ", tz, 1);
	rtime = time(NULL);
	itime = localtime(&rtime);

	strftime(buf, (sizeof buf) - 1, fmt, itime);

	return smprintf("%s", buf);
}
