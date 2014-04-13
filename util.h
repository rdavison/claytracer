#ifndef __RTCL_UTIL_H
#define __RTCL_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void read_file(char **buffer, const char *filename);

// for debugging
int time_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1);
void time_print(struct timeval *tv);
void NOT_IMPLEMENTED(const char *filename, int line, const char *function);
void SUCCESSED(const char *function);
void INFUNC(const char *function);

#endif
