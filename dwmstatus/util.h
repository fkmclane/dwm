#include <stdarg.h>

#include <X11/Xlib.h>

#define LENGTH(X)   (sizeof X / sizeof X[0])

void initstatus(char *status);
void addstatus(char *status, const char *fmt, ...);
void splitstatus(char *status);
void setstatus(Display *dpy, char *str);
