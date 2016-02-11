#include "net.h"

int s_read(struct client_table_entry *cte)
{
    bzero(cte->iobuffer,IO_BUFFER_SIZE);
    return read(cte->session_fd, cte->iobuffer,IO_BUFFER_SIZE);
}

int s_write(struct client_table_entry *cte, char *message, int length)
{
    return write(cte->session_fd, message, length);
}

int protocol_send(struct client_table_entry *cte, char *message)
{
    return s_write(cte, message, sizeof(message));
}

/*
* Note: This function might be better placed somewhere else than in net.c!
*
* Explanation:
* ------------
* Parses input from clients over IP network. Return value indicates status.
* Uses the command_code struct to convey parameter and command code. 
* The purpose of the parser was to make the server/client code less
* cluttered and to make future protocol implementations easier. 
* 
* Return values:
* ------------  
* Returns -1 if size is wrong
* Returns 0 if protocol is not followed
* Returns 1 if all is ok
*/

int parse_input(struct client_table_entry *cte)
{
    /* INPUT TO BIG */
    if(strlen(cte->iobuffer) > (MAX_PARAMETER_SIZE + MAX_COMMAND_SIZE) + 2) {
        return -1;
    }

    /* INPUT TO SMALL */
    if(strlen(cte->iobuffer) < (MIN_PARAMETER_SIZE + MIN_COMMAND_SIZE) + 2) {
        return -1;
    }

    /* ZEROING COMMAND CODE */
    cte->cm.code = 0;

    /* IDENTIFY */
    if(strstr((char*)cte->iobuffer, PROTOCOL_IDENTIFY ) == (cte->iobuffer)) {
        int length = (strlen(cte->iobuffer) - strlen(PROTOCOL_IDENTIFY) );
        memcpy(cte->cm.parameter, (cte->iobuffer)+(strlen(PROTOCOL_IDENTIFY)*(sizeof(char))), length);
        cte->cm.code = CODE_IDENTIFY;
        return 1;
    }

    /* LISTCLIENTS */
    if(strstr((char*)cte->iobuffer, PROTOCOL_LISTCLIENTS ) == (cte->iobuffer)) {
        cte->cm.code = CODE_LISTCLIENTS;
        return 1;
    }

    /* NMEA */
    if(strstr((char*)cte->iobuffer, PROTOCOL_NMEA ) == (cte->iobuffer)) {
        cte->cm.code = CODE_NMEA;
        
        /* Fetch RMC */
        //if(strstr(cte->iobuffer, RMC ) != NULL){
        //    memcpy(cte->nmea.rmc, cte->iobuffer, strlen(cte->iobuffer));
        //}

        t_print("Length of string: %d\n", strlen(cte->iobuffer));
        printf(cte->iobuffer);
        memset(cte->iobuffer, '\0',MAX_PARAMETER_SIZE);
        return 1;
    }    

    /* DISCONNECT */
    if(strstr((char*)cte->iobuffer, PROTOCOL_DISCONNECT ) == (cte->iobuffer)) {
        cte->cm.code = CODE_DISCONNECT;
        return 1;
    }

    return 0;
}