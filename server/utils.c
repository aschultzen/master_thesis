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
void extract_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
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
    }
}

/* 
* Extracts IP from session file descriptor
*/
void get_ip_str(int session_fd, char *ip)
{
    struct sockaddr addr;
    addr.sa_family = AF_INET;
    socklen_t addr_len = sizeof(addr);
    if(getpeername(session_fd, (struct sockaddr *) &addr, &addr_len)) {
        die(44,"getsocketname failed\n");
    }
    extract_ip_str(&addr,ip, addr_len);
}

/* 
* Print with timestamp:
* Example : [01.01.01 - 10:10:10] [<Some string>]
*/
void t_print(const char* format, ...)
{
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

/* 
* Loads config. 
* Returns: -1 fail | 0 success
*/
int load_config(struct config *cfg, char *path)
{
    FILE *config_file;
    long file_size;
    char *input_buffer;
    char temp_buffer[100];
    int status = 0;

    config_file=fopen(path, "r");
    if(!config_file){
        t_print("config_loader(): Failed to load config file, aborting.\n");
        return -1;
    }

    fseek(config_file , 0L , SEEK_END);
    file_size = ftell(config_file);
    rewind(config_file);

    /* Alocating memory for the file buffer */
    input_buffer = calloc( 1, file_size+1 );
    if(!input_buffer){
      fclose(config_file);
      t_print("config_loader(): Memory allocation failed, aborting.\n");
      return -1; 
    } 

    /* Get the file into the buffer */
    if(fread( input_buffer , file_size, 1 , config_file) != 1){
        fclose(config_file);
        free(input_buffer);
        t_print("config_loader(): Read failed, aborting\n");
        return -1;
    }   

    /* Retrieving max connections from config */
    char *ptr = strstr(input_buffer,CONFIG_SERVER_MAX_CONNECTIONS);
    if(ptr != NULL){
        int length = strlen(ptr) - strlen(CONFIG_SERVER_MAX_CONNECTIONS);
        memcpy(temp_buffer, ptr+(strlen(CONFIG_SERVER_MAX_CONNECTIONS)*(sizeof(char))), length);
        status = sscanf(temp_buffer, "%d", &cfg->max_clients);
    }

    if(status == EOF || status == 0){
        fclose(config_file);
        free(input_buffer);
        return -1;
    }

    /* Retrieving warm_up from config */
    ptr = strstr(input_buffer,CONFIG_SERVER_WARM_UP);
    if(ptr != NULL){
        int length = strlen(ptr) - strlen(CONFIG_SERVER_WARM_UP);
        memcpy(temp_buffer, ptr+(strlen(CONFIG_SERVER_WARM_UP)*(sizeof(char))), length);
        status = sscanf(temp_buffer, "%d", &cfg->warm_up_seconds);
    }

    if(status == EOF || status == 0){
        fclose(config_file);
        free(input_buffer);
        return -1;
    }

    fclose(config_file);
    free(input_buffer);
    return 0;
}

int calc_nmea_checksum(char *s) {
    char checksum = 0;
    int i;
    int received_checksum = 0;
    int calculated_checksum = 0;


    /* Substring to iterate over */
    char substring[100] = {0};

    /* Finding end (*) and calculate length */
    char *substring_end = strstr(s, "*");
    int length = substring_end - (s+1);

    /* Copying the substring */
    memcpy(substring, s+1, length);

    /* Calculating checksum */
    for(i = 0; i < length; i++){
        checksum = checksum ^ substring[i];
    }

    /* Reusing substring buffer */
    sprintf(substring, "%x\n", checksum);

    /* Converting calculated checksum to int */
    sscanf(substring, "%d", &calculated_checksum);

    /* Fetching received checksum */
    memcpy(substring, substring_end+1, strlen(s));

    /* Converting received checksum to int*/
    sscanf(substring, "%d", &received_checksum);

    /* Comparing checksum */
    if(received_checksum == calculated_checksum){
        return 0;    
    }
    else{
        return -1;
    }
    
}


