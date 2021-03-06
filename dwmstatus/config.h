/* General */
static unsigned int interval    = 2000;

/* CPU */
static Core cpu[]               = {
	/* name    id */
	{ "CPU",   "cpu" },
};

/* Temperature */
static const Temp thermal[]     = {
	/* name    id */
	//{ "TEMP",  "thermal_zone1" },
	{ NULL },
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
static const TZ tz[]          = {
	/* name    timezone */
	{ "",      "America/New_York" },
};
