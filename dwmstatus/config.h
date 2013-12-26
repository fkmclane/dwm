/* General */
static unsigned int interval    = 2000;

/* CPU */
static Core cpu[]               = {
	/* name   id */
	{ "CPU", "core" },
};

/* Temperature */
static const Temp thermal[]     = {
	{ "TEMP", "thermal_zone1" },
};

/* Memory */
static const char *units[4]  = { "kB", "MB", "GB", "TB" };

/* Volume */
static const char card[]        = "default";
static const char selement[]    = "Master";

/* Battery */
static const char batt[]        = "BAT1";

/* Time */
static const char fmt[]         = "%A %d %B %H:%M";
static const char tz[]          = "America/New_York";
