#include "net.h"

int s_read(struct transmission_s *tsm)
{
    bzero(tsm->iobuffer,IO_BUFFER_SIZE);
    return read(tsm->session_fd, tsm->iobuffer,IO_BUFFER_SIZE);
}

int s_write(struct transmission_s *tsm, char *message, int length)
{
    return write(tsm->session_fd, message, length);
}