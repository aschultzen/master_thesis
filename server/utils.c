#include "utils.h"

void die (int line_number, const char * format, ...)
{
    va_list vargs;
    va_start (vargs, format);
    fprintf (stderr, "%d: ", line_number);
    vfprintf (stderr, format, vargs);
    fprintf (stderr, ".\n");
    exit(1);
}

/* 
* Extracts IP adress from sockaddr struct.
* Supports both IPV4 and IPV6
*/
char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch(sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                    s, maxlen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                    s, maxlen);
            break;

        default:
            strncpy(s, "Unknown AF", maxlen);
            return NULL;
    }

    return s;
}

/* 
* Print with timestamp:
* Example : [01.01.01 - 10:10:10] [<Some string>]
*/
void t_print(const char* format, ...){
	char buffer[100];
	time_t rawtime;
	struct tm *info;
	time(&rawtime);
	info = gmtime(&rawtime);
	strftime(buffer,80,"[%x - %X] ", info);
	va_list argptr;
    va_start(argptr, format);
    fputs(buffer, stdout);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
}
