#ifndef STRPTIME_H
#define STRPTIME_H


struct tm;

/*
 * Version of "strptime()", for the benefit of OSes that don't have it.
 */
extern char *strptime(const char *, const char *, struct tm *);

#endif
