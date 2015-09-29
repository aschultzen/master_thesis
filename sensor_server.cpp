#include "sensor_server.h"

static void die (int line_number, const char * format, ...)
{
    exit (1);
    va_list vargs;
    va_start (vargs, format);
    fprintf (stderr, "%d: ", line_number);
    vfprintf (stderr, format, vargs);
    fprintf (stderr, ".\n");
}

void handle_sigchld(int sig) {
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
}

void handle_session(int session_fd) {
    printf("New connection (PID:%d)\n", getpid());
    char *buffer = (char*) calloc (BUFFER_SIZE,sizeof(char));
    int status = 0;

     struct timeval timeout;      
            timeout.tv_sec = TIME_OUT;
            timeout.tv_usec = 0;

    //Setting timeout
    if (setsockopt (session_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        die(101,"setsockopt failed\n");
    
    while(1){
        bzero(buffer,512);
        status = read(session_fd,buffer,255);
        if (status < 0){
            printf("ERROR reading from socket, closing socket.\n");
            break;
        }

        printf("Received: %s\n",buffer);
        if(strstr(buffer, "DISCONNECT") != NULL){
            printf("Received DISCONNECT, closing connection.\n");
            free(buffer);   
            break;
        }

        status = write(session_fd,"ACK\n",4);
        if (status < 0){
            printf("ERROR writing to socket\n");
            break;
        } 
    }
}

int main(int argc, char *argv[])
{
    /* Checking parameters */
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
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

    /* Initializing variables */
    int server_sockfd, newsockfd, portno;
    socklen_t clilen;             
    /*char buffer[256];*/
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    portno = atoi(argv[1]);

    /* Initialize socket */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0){
       die(62,"ERROR opening socket\n");
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

    /* Listen for connections */
    listen(server_sockfd,SOMAXCONN); //Not blocking, just marking

    for (;;) {
        printf("Server(PID:%d)\n", getpid());
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