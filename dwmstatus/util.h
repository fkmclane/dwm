#include <stdarg.h>

#include <X11/Xlib.h>

#define LENGTH(X)   (sizeof X / sizeof X[0])

char *smprintf(char *fmt, ...);
void addstatus(char *status, const char *str);
void setstatus(Display *dpy, char *str);
