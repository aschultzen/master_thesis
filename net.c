#include "net.h"

int s_read(struct session_info *s_info) {
    bzero(s_info->iobuffer,BUFFER_SIZE);
    return read(s_info->session_fd, s_info->iobuffer,255);
}

int s_write(int session_fd) {
    return 0;
}

int respond(struct session_info *s_info) {
    int status = s_read(s_info);
    if(strstr((char*)s_info->iobuffer, DISCONNECT) != NULL){
        printf("Client %d requested DISCONNECT.", s_info->client_id);
        return -1;
    }

    printf("%s", (char*)s_info->iobuffer);
    return status;
}

/* The session_info struct allocated on stack only */
void handle_session(int session_fd) {
    /* Initializing structure */

    struct session_info *s_info = malloc(sizeof(struct session_info)); 
        s_info->tv.tv_sec = TIME_OUT;
        s_info->tv.tv_usec = 0;
        s_info->client_id = -1;
        s_info->session_fd = session_fd;
        s_info->iobuffer = calloc (BUFFER_SIZE, sizeof(char));
        if(s_info->iobuffer == NULL){
            die(21, "Memory allocation failed!");
        }

    /* Setting socket timeout to default value */
    /* This doesn't always work for some reason, race condition? :/ */
     if (setsockopt (s_info->session_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&s_info->tv, sizeof(struct timeval)) < 0)
        die(36,"setsockopt failed\n");

    while(1){
        if(respond(s_info) < 0){
            break;
        }
    }
    free(s_info->iobuffer);
    free(s_info);
}

void handle_sigchld(int sig) {
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
}

/* 
* Main loop and everything.
* Variables to watch out for:
*   session_fd
*/
int start_server(int portno) {
    /* Initializing variables */
    int server_sockfd;          
    struct sockaddr_in serv_addr;

    /* Initialize socket */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0){
       die(62,"ERROR opening socket\n");
    }

    /* Registering the SIGCHLD handler */
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
      perror(0);
      exit(1);
    }

    /* 
    Initializing the server address struct:
    AF_INET = IPV4 Internet protocol
    INADDR_ANY = Accept connections to all IPs of the machine
    htons(portno) = Endianess: network to host long(port number). 
    */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Assigns the address (serv_addr) to the socket
    referred to by server_sockfd. */
    if (bind(server_sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0){
        die(80,"ERROR on binding");
    } 

    /* Marking the connection for listening*/
    listen(server_sockfd,SOMAXCONN);

    while (1) {
        int session_fd = accept(server_sockfd,0,0);
        if (session_fd==-1) {
            if (errno==EINTR) continue;
            die(90,"failed to accept connection (errno=%d)",errno);
        }
        pid_t pid=fork();
        if (pid==-1) {
            die(94, "failed to create child process (errno=%d)",errno);
        } else if (pid==0) {
            close(server_sockfd);
            handle_session(session_fd);
            printf("Closing up session (PID: %d)\n", getpid());
            close(session_fd);
            _exit(0);
        } else {
            close(session_fd);
        }
    }
}
