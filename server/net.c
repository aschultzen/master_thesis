#include "net.h"

int s_read(struct session_info *s_info) {
    bzero(s_info->iobuffer,BUFFER_SIZE);
    return read(s_info->session_fd, s_info->iobuffer,255);
}

int s_write(struct session_info *s_info, char *message, int length) {
    return write(s_info->session_fd, message, length);
}

/*
* Parses input
* Returns -1 if size is wrong
* Returns 0 if protocol is not followed
* Returns 1 if all is ok
*/
int parse_input(struct session_info *s_info){
	/* Input too big */
	if(strlen(s_info->iobuffer) > (MAX_PARAMETER_SIZE + MAX_COMMAND_SIZE) + 2){
		return -1;
	}

	/* Input too small */
	if(strlen(s_info->iobuffer) < (MIN_PARAMETER_SIZE + MIN_COMMAND_SIZE) + 2){
		return -1;
	}

	/* ZEROING */
	bzero(&s_info->cm, sizeof(struct comm));

	/* IDENTIFY */
	if(strstr((char*)s_info->iobuffer, P_IDENTIFY ) == (s_info->iobuffer)){
		int length = (strlen(s_info->iobuffer) - strlen(P_IDENTIFY) ) - 2; //2 = escape characters
        memcpy(&s_info->cm.parameter, (s_info->iobuffer)+(strlen(P_IDENTIFY)*(sizeof(char))), length);
        s_info->cm.command_code = C_IDENTIFY;
        return 1;
    }

    /* DISCONNECT */
	if(strstr((char*)s_info->iobuffer, P_DISCONNECT ) == (s_info->iobuffer)){
        s_info->cm.command_code = C_DISCONNECT;
        return 1;
    }

    return 0;
}