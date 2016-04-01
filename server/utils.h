#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "list.h"
#include "config.h"

void die (int line_number, const char * format, ...);
void get_ip_str(int session_fd, char *ip);
void t_print(const char* format, ...);
int load_config(struct config *cfg, char *path);
int calculate_nmea_checksum(char *s);
int word_extractor(int delim_num_1, int delim_num_2, char delimiter, char *buffer, int buffsize, char *string, int str_len);

#endif /* !UTILS_H */