#include "net.h"

int s_read(struct session_info *s_info) {
    bzero(s_info->iobuffer,BUFFER_SIZE);
    return read(s_info->session_fd, s_info->iobuffer,255);
}

int s_write(struct session_info *s_info, char *message, int length) {
    return write(s_info->session_fd, message, length);
}