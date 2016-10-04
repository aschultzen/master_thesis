#include "utils.h"

/* These are also in action.c, duplicates are no solution */
#define ERROR_FCLOSE "Failed to close file, out of space?\n"
#define ERROR_FWRITE "Failed to write to file, aborting.\n"
#define ERROR_FREAD "Failed to read file, aborting.\n"
#define ERROR_FOPEN "Failed to open file, aborting.\n"

#define MJD_SCRIPT_PATH "./get_mjd.py"

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
* Extracts IP address from sockaddr struct.
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
* Returns: 0 fail | 1 success
*/
int load_config(struct config_map_entry *cme, char *path, int entries)
{
    FILE *config_file;
    int file_len;
    char *input_buffer;

    int status = 0;

    config_file=fopen(path, "r");
    if(!config_file) {
        t_print("config_loader(): Failed to load config file, aborting.\n");
        return 0;
    }

    fseek(config_file , 0L , SEEK_END);
    file_len = ftell(config_file);
    rewind(config_file);

    char temp_buffer[file_len];

    /* Alocating memory for the file buffer */
    input_buffer = calloc( file_len, sizeof(char));
    if(!input_buffer) {
        fclose(config_file);
        t_print("config_loader(): Memory allocation failed, aborting.\n");
        return 0;
    }

    /* Get the file into the buffer */
    if(fread( input_buffer , file_len, 1 , config_file) != 1) {
        fclose(config_file);
        free(input_buffer);
        t_print("config_loader(): Read failed, aborting\n");
        return 0;
    }

    int counter = 0;
    while(counter < entries) {
        memset(temp_buffer, '\0',file_len);
        char *search_ptr = strstr(input_buffer,cme->entry_name);
        if(search_ptr != NULL) {
            int length = strlen(search_ptr) - strlen(cme->entry_name);
            memcpy(temp_buffer, search_ptr+(strlen(cme->entry_name)*(sizeof(char))),
                   length);
            status = sscanf(temp_buffer, cme->modifier, cme->destination);
            if(status == EOF || status == 0) {
                fclose(config_file);
                free(input_buffer);
                return -1;
            }
        }
        counter++;
        cme++;
    }

    fclose(config_file);
    free(input_buffer);
    return 1;
}

int calculate_nmea_checksum(char *nmea)
{
    char checksum = 0;
    int i;
    int received_checksum = 0;
    int calculated_checksum = 0;


    /* Substring to iterate over */
    char substring[100] = {0};

    /* Finding end (*) and calculate length */
    char *substring_end = strstr(nmea, "*");
    int length = substring_end - (nmea+1);

    /* Copying the substring */
    memcpy(substring, nmea+1, length);

    /* Calculating checksum */
    for(i = 0; i < length; i++) {
        checksum = checksum ^ substring[i];
    }

    /* Reusing substring buffer */
    sprintf(substring, "%x\n", checksum);

    /* Converting calculated checksum to int */
    sscanf(substring, "%d", &calculated_checksum);

    /* Fetching received checksum */
    memcpy(substring, substring_end+1, strlen(nmea));

    /* Converting received checksum to int*/
    sscanf(substring, "%d", &received_checksum);

    /* Comparing checksum */
    if(received_checksum == calculated_checksum) {
        return 1;
    } else {
        return 0;
    }

}

/*
* Used to extract words from between two delimiters
* delim_num_1 -> The number of the first delimiter, ex.3
* delim_num_2 -> The number of the second delimiter, ex.5
* delimiter -> The character to be used as a delimiter
* string -> Input
* buffer -> To transport the string
*/
int substring_extractor(int start, int end, char delimiter, char *buffer,
                        int buffsize, char *string, int str_len)
{
    int i;
    int delim_counter = 0;
    int buffer_index = 0;

    const int carriage_return = 13;

    bzero(buffer, buffsize);

    for(i = 0; i < str_len; i++) {
        /* Second delim (end) reached, stopping. */
        if(delim_counter == end || (int)string[i] == carriage_return) {
            return 1;
        }

        if(string[i] == delimiter) {
            delim_counter++;
        } else {
            /* The first delim is reached */
            if(delim_counter >= start) {
                buffer[buffer_index] = string[i];
                buffer_index++;
            }
        }
    }
    /* Reached end of string without encountering end delimit */
    return 0;
}

int str_len_u(char *buffer, int buf_len)
{
    int i;
    char prev = 'X';
    for(i = 0; i < buf_len; i++) {
        if(buffer[i] == 0x0a && prev == 0x0a) {
            return i;
        }
        prev = buffer[i];
    }
    return -1;
}

/* Mega hackish code for getting MJD */
int get_today_mjd(char *buffer)
{
    int status = run_command(MJD_SCRIPT_PATH, buffer);
    /* Removing newline */
    buffer[strcspn(buffer, "\n")] = 0;
    return status;
}

int run_command(char *path, char *output)
{
    FILE *fp;
    int buffer_size = 1000;
    char buffer[buffer_size];
    memset(buffer, '\0', buffer_size);

    /* Open the command for reading. */
    fp = popen(path, "r");
    if (fp == NULL) {
        t_print("Failed to run command\n");
        return 0;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
        strcat(output,buffer);
    }

    /* close */
    pclose(fp);
    return strlen(output);
}

int log_to_file(char *path, char *content, int stamp_switch)
{
    FILE *log_file;
    log_file = fopen(path, "a+");

    /* Open file */
    if(!log_file) {
        t_print(ERROR_FOPEN);
        return 0;
    }

    /* Add MJD timestamp */
    if(stamp_switch == 1) {
        int timestamp_size = 50;
        char timestamp[timestamp_size];
        memset(timestamp,'\0', timestamp_size);

        get_today_mjd(timestamp);
        if(!fprintf(log_file,"%s,",timestamp)) {
            t_print(ERROR_FWRITE);
            return 0;
        }
    }

    /* Just stamp with regular time */
    if(stamp_switch == 2){
        char timestamp[100];
        memset(timestamp, '\0', 100);
        time_t rawtime;
        struct tm *info;
        time(&rawtime);
        info = gmtime(&rawtime);
        strftime(timestamp,80,"[%x - %X] ", info); 

        if(!fprintf(log_file,"%s,",timestamp)) {
            t_print(ERROR_FWRITE);
            return 0;
        }
    }

    /* Write content to file */
    if(!(fprintf(log_file,"%s",content))) {
        t_print(ERROR_FWRITE);
        return 0;
    }

    /* Close file */
    if(fclose(log_file)) {
        t_print(ERROR_FCLOSE);
    }
    return 1;
}
