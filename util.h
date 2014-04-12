#ifndef __RTCL_UTIL_H
#define __RTCL_UTIL_H

#include <stdio.h>
#include <stdlib.h>

void read_file(char **buffer, const char *filename);
void NOT_IMPLEMENTED(const char *filename, int line, const char *function);
void SUCCESSED(const char *function);
void INFUNC(const char *function);

#endif
