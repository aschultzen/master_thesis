#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "list.h"

struct client_table_entry{ 
	struct list_head list;
	int id;
	pid_t pid;
	char ip[INET_ADDRSTRLEN]; 
} __attribute__ ((packed));

void die (int line_number, const char * format, ...);
void get_ip_str(int session_fd, char *ip);
void t_print(const char* format, ...);

#endif /* !UTILS_H */