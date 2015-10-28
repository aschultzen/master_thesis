#include "sensor_server.h"
#include "serial.h"

/* Declaration of shared memory variables */
static char *connections;
static char *message;
 
int respond(struct session_info *s_info) {
    int read_status = s_read(s_info);
    if(read_status == -1){
        t_print("Read failed, disconnecting client\n");
        if(s_info->client_id != -1){
          connections[s_info->client_id] = '0';  
        }
        return -1;
    }
    int parse_status = parse_input(s_info);

    if(parse_status == -1){
        s_write(s_info, ILL_SIZE, sizeof(ILL_SIZE));
    }
    if(parse_status == 0){
        s_write(s_info, ILL_COM, sizeof(ILL_COM));
    }
    if(parse_status == 1){
        if(s_info->cm.command_code == C_DISCONNECT){
            t_print("Client %d requested DISCONNECT.\n", s_info->client_id);
            s_write(s_info, OK, sizeof(OK));
            connections[s_info->client_id] = '0';
            return -1;
        }
        if(s_info->cm.command_code == C_IDENTIFY){
            int id = 0;
            if(sscanf(s_info->cm.parameter, "%d", &id) == -1){
                s_write(s_info, ILL_COM, sizeof(ILL_COM));
                return 0;
            }

            if(connections[id] == '1') {
                s_info->client_id = -1;
                s_write(s_info, "ID in use!\n", 11);
                return 0;
            }   

            s_info->client_id = id;
            t_print("%s ID set to: %d\n",s_info->ip, s_info->client_id);
            connections[s_info->client_id] = '1';
            s_write(s_info, OK, sizeof(OK));
            return 0;
        }

        if(s_info->client_id < 0){
            s_write(s_info, NO_ID, sizeof(NO_ID));
            return 0;  
        }
    }
    return 0;
}

/* The session_info struct allocated on stack only */
void handle_session(int session_fd) {
    /* Initializing structure */
    struct session_info *s_info = malloc(sizeof(struct session_info)); 
        s_info->tv.tv_sec = CLIENT_TIMEOUT + 1000; //remove 1000 when testing is done!
        s_info->tv.tv_usec = 0;
        s_info->client_id = -1;
        s_info->session_fd = session_fd;
        s_info->iobuffer = calloc (BUFFER_SIZE, sizeof(void*));
        if(s_info->iobuffer == NULL){
            die(21, "Memory allocation failed!");
        }

    /* Setting socket timeout to default value */
    /* This doesn't always work for some reason, race condition? :/ */
     if (setsockopt (s_info->session_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&s_info->tv, sizeof(struct timeval)) < 0)
        die(36,"setsockopt failed\n");

    /* 
    * Fetching clients IP address and 
    * storing it in s_info
    */
    struct sockaddr addr;
    addr.sa_family = AF_INET;
    socklen_t addr_len = sizeof(addr);
    if(getpeername(session_fd, (struct sockaddr *) &addr, &addr_len)){
        die(93,"getsocketname failed\n");
    }
    get_ip_str(&addr, s_info->ip,addr_len);
    t_print("Client connected from: %s\n", s_info->ip); // prints "192.0.2.33"

    /* 
    * Entering child process main loop 
    * Breaks (disconnects the client) if 
    * respond == 0
    */
    while(1){
        if(respond(s_info) < 0){
            break;
        }
    }
    /* Freeing resources */
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
int start_server(int portno, char *usb) {
    /* Initializing variables */
    //char *USB = "/dev/ttyACM0";
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

    /* Initializing the server address struct:
    AF_INET = IPV4 Internet protocol
    INADDR_ANY = Accept connections to all IPs of the machine
    htons(portno) = Endianess: network to host long(port number).*/
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

    /* Forking out proc for serial com */
    if(usb != NULL){
        /* Shared memory used for "IPC"*/
        connections = mmap(NULL, sizeof MAX_CONNECTIONS*(sizeof(char)), PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        message = mmap(NULL, sizeof DISPLAY_SIZE*(sizeof(char)), PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        pid_t pid=fork();
        if (pid==-1) {
            die(94, "failed to create child process (errno=%d)",errno);
        } else if (pid==0) {
            t_print("Serial COM started (PID: %d)\n", getpid());
            open_serial(usb, connections);
            t_print("Serial COM closed (PID: %d)\n", getpid());
            _exit(0);
        }

        int i;
        for(i = 0; i < DISPLAY_SIZE; i++){
            message[i] = '0';
        }
    }
    else
    {
        connections = malloc(sizeof MAX_CONNECTIONS*(sizeof(char)));
        t_print("No serial display defined.\n");
    }

    /* Initializing connection table */
    int i;
        for(i = 0; i < MAX_CONNECTIONS; i++){
        connections[i] = '0';
    }

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
            t_print("Closing up session (PID: %d)\n", getpid());
            close(session_fd);
            _exit(0);
        } else {
            free(message);
            close(session_fd);
        }
    }
}

int main(int argc, char *argv[])
{
    /* Checking parameters */
    t_print("Starting the server...\n");
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    start_server(atoi(argv[1]), argv[2]);
    exit(0);
} 