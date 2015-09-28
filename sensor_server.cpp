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

void handle_session(int session_fd) {
    char *buffer = (char*) calloc (BUFFER_SIZE,sizeof(char));
    int status = 0;
    
    while(1){
        bzero(buffer,512);
        status = read(session_fd,buffer,255);
        if (status < 0){
            die(21, "ERROR reading from socket");
        }

        printf("Received: %s\n",buffer);
        if(strstr(buffer, "DISCONNECT") != NULL){
            printf("Received DISCONNECT, removing client\n");
            free(buffer);   
            break;
        }

        status = write(session_fd,"ACK\n",4);
        if (status < 0){
            die(32, "ERROR writing to socket");
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

    /* Initializing variables */
    int server_sockfd, newsockfd, portno;
    socklen_t clilen;             
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    portno = atoi(argv[1]);

    /* Initialize socket */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0){
       die(62,"ERROR opening socket");
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
    listen(server_sockfd,SOMAXCONN);
    
    for (;;) {
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
            close(session_fd);
            exit(0);
        } else {
            close(session_fd);
        }
    }
}