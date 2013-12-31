#include <stdio.h>
#include <time.h>

#ifdef ALSA
#include <alsa/asoundlib.h>
#endif

#include "data.h"

int
getcore(Core *cpu) {
	int percent;
	FILE *file;
	char name[10];
	unsigned long long user, userlow, sys, idle, total;
	int ret;

	file = fopen("/proc/stat", "r");
	do {
		ret = fscanf(file, "%s %llu %llu %llu %llu %*[^\n]\n", name, &user, &userlow, &sys, &idle);
	}
	while(strcmp(name, cpu->id) != 0 && ret != EOF);
	fclose(file);
	if(ret < 5)
		return ret;

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
	char path[64];
	int temp;
	int ret;

	snprintf(path, sizeof(path), "/sys/class/thermal/%s/temp", thermal);
	file = fopen(path, "r");
	ret = fscanf(file, "%d\n", &temp);
	fclose(file);
	if(ret < 1)
		return -1;

	return temp / 1000;
}

int
getmem() {
	FILE *file;
	int total, free, buffers, cache;
	int ret;

	file = fopen("/proc/meminfo", "r");
	ret = fscanf(file, "MemTotal: %d kB\nMemFree: %d kB\nBuffers: %d kB\nCached: %d kB\n", &total, &free, &buffers, &cache);
	fclose(file);
	if(ret < 4)
		return -1;

	return total - free - buffers - cache;
}

#ifdef ALSA
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
#endif

int
getbatt(const char *batt) {
	FILE *file;
	char path[64];
	long full, now;
	int ret;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_full", batt);
	file = fopen(path, "r");
	ret = fscanf(file, "%ld\n", &full);
	fclose(file);
	if(ret < 1)
		return -1;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_now", batt);
	file = fopen(path, "r");
	ret = fscanf(file, "%ld\n", &now);
	fclose(file);
	if(ret < 1)
		return -1;

	return now * 100 / full;
}

void
mktimes(char *buf, const char *fmt, const char *tz) {
	time_t rtime;
	struct tm *itime;

	setenv("TZ", tz, 1);
	rtime = time(NULL);
	itime = localtime(&rtime);

	strftime(buf, 64, fmt, itime);
}
