typedef struct {
	const char *name;
	const char *id;
	unsigned long long user, userlow, sys, idle;
} Core;

typedef struct {
	const char *name;
	const char *id;
} Temp;

typedef struct {
	const char *name;
	const char *id;
} TZ;

int getcore(Core *cpu);
int gettemp(const char *thermal);
int getmem();
int getvol(const char *card, const char *selement);
int getbatt(const char *batt);
char *mktimes(const char *fmt, const char *tz);
