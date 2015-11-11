#include "net.h"

int s_read(struct session_info *s_info)
{
    bzero(s_info->iobuffer,SESSION_INFO_IO_BUFFER_SIZE);
    return read(s_info->session_fd, s_info->iobuffer,255);
}

int s_write(struct session_info *s_info, char *message, int length)
{
    return write(s_info->session_fd, message, length);
}

int protocol_send(struct session_info *s_info, char *message)
{
    return s_write(s_info, message, sizeof(message));
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
int parse_input(struct session_info *s_info)
{
    /* Input too big */
    if(strlen(s_info->iobuffer) > (MAX_PARAMETER_SIZE + MAX_COMMAND_SIZE) + 2) {
        return -1;
    }

    /* Input too small */
    if(strlen(s_info->iobuffer) < (MIN_PARAMETER_SIZE + MIN_COMMAND_SIZE) + 2) {
        return -1;
    }

    /* ZEROING */
    bzero(&s_info->cm, sizeof(struct command_code));

    /* IDENTIFY */
    if(strstr((char*)s_info->iobuffer, PROTOCOL_IDENTIFY ) == (s_info->iobuffer)) {
        int length = (strlen(s_info->iobuffer) - strlen(PROTOCOL_IDENTIFY) ) - 2; //2 = escape characters
        memcpy(&s_info->cm.parameter, (char*)(s_info->iobuffer)+(strlen(PROTOCOL_IDENTIFY)*(sizeof(char))), length);
        s_info->cm.code = CODE_IDENTIFY;
        return 1;
    }

    /* DISCONNECT */
    if(strstr((char*)s_info->iobuffer, PROTOCOL_DISCONNECT ) == (s_info->iobuffer)) {
        s_info->cm.code = CODE_DISCONNECT;
        return 1;
    }

    return 0;
}