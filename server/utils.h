#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

void die (int line_number, const char * format, ...);
char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);
void print_timestamp(const char* format, ...);

#endif /* !UTILS_H */