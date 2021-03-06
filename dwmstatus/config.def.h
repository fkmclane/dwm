/* General */
static unsigned int interval    = 2000;

/* CPU */
static Core cpu[]               = {
	/* name    id */
	{ "CPU",   "core" },
};

/* Temperature */
static const Temp thermal[]     = {
	/* name    id */
	{ "TEMP",  "thermal_zone0" },
};

/* Memory */
static const char *units[4]  = { "kB", "MB", "GB", "TB" };

/* Volume */
static const char card[]        = "default";
static const char selement[]    = "Master";

/* Battery */
static const char batt[]        = "BAT0";

/* Time */
static const char fmt[]         = "%A %d %B %I:%M %p";
static const TZ tz[]          = {
	/* name    timezone */
	{ "",      "America/New_York" },
};
