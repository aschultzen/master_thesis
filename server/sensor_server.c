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
        s_write(s_info, ERROR_ILLEGAL_MESSAGE_SIZE, sizeof(ERROR_ILLEGAL_MESSAGE_SIZE));
    }
    if(parse_status == 0){
        s_write(s_info, ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
    }
    if(parse_status == 1){
        if(s_info->cm.code == CODE_DISCONNECT){
            t_print("Client %d requested DISCONNECT.\n", s_info->client_id);
            s_write(s_info, PROTOCOL_OK, sizeof(PROTOCOL_OK));
            connections[s_info->client_id] = '0';
            return -1;
        }
        if(s_info->cm.code == CODE_IDENTIFY){
            int id = 0;
            if(sscanf(s_info->cm.parameter, "%d", &id) == -1){
                s_write(s_info, ERROR_ILLEGAL_COMMAND, sizeof(ERROR_ILLEGAL_COMMAND));
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
            s_write(s_info, PROTOCOL_OK, sizeof(PROTOCOL_OK));
            return 0;
        }

        if(s_info->client_id < 0){
            s_write(s_info, ERROR_NO_ID, sizeof(ERROR_NO_ID));
            return 0;  
        }
    }
    return 0;
}

/* The session_info struct allocated on stack only */
void setup_session(int session_fd) {
    /* Initializing structure */
    struct session_info *s_info = malloc(sizeof(struct session_info)); 
        s_info->heartbeat_timeout.tv_sec = CLIENT_TIMEOUT + 1000; //remove 1000 when testing is done!
        s_info->heartbeat_timeout.tv_usec = 0;
        s_info->client_id = -1;
        s_info->session_fd = session_fd;
        s_info->iobuffer = calloc (SESSION_INFO_IO_BUFFER_SIZE, sizeof(void*));
        if(s_info->iobuffer == NULL){
            die(21, "Memory allocation failed!");
        }

    /* Setting socket timeout to default value */
    /* This doesn't always work for some reason, race condition? :/ */
     if (setsockopt (s_info->session_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&s_info->heartbeat_timeout, sizeof(struct timeval)) < 0)
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
* Main loop for the server.
* Forks everytime a client connects 
* and calls setup_session()
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
        connections = mmap(NULL, sizeof SERVER_MAX_CONNECTIONS*(sizeof(char)), PROT_READ | PROT_WRITE, 
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

        int loop_counter;
        for(loop_counter = 0; loop_counter < DISPLAY_SIZE; loop_counter++){
            message[loop_counter] = '0';
        }
    }
    else
    {
        connections = malloc(sizeof SERVER_MAX_CONNECTIONS*(sizeof(char)));
        t_print("No serial display defined.\n");
    }

    /* Initializing connection table */
    int loop_counter;
        for(loop_counter = 0; loop_counter < SERVER_MAX_CONNECTIONS; loop_counter++){
        connections[loop_counter] = '0';
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
            setup_session(session_fd);
            t_print("Closing up session (PID: %d)\n", getpid());
            close(session_fd);
            _exit(0);
        } else {
            free(message);
            close(session_fd);
        }
    }
}

int usage(char *argv[]){
    char description[] ={"Required argument:\n\t -p <PORT NUMBER>\n\nOptional:\n\t -s <PATH TO SERIAL DISPLAY>\n"}; 
    printf("Usage: %s [ARGS]\n\n", argv[0]);
    printf("Sensor_server: Server part of GPS Jamming/Spoofing system\n\n");
    printf("%s\n", description);
    return 0;
}

int main(int argc, char *argv[])
{
    char *serial_display_path = NULL;
    char *port_number = NULL;

    /* getopt silent mode set */
    opterr = 0; 

    if(argc < 3){
        usage(argv);
        return 0;
    }

    while (1) {
        char c;

        c = getopt (argc, argv, "p:");
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'p':
            port_number = optarg;
            break;
        }
    }
    
    if(port_number == NULL){
        printf("Missing parameters!\n");
    }

    if(argc == 5){
        if(argv[3][1] == 's'){
            serial_display_path = argv[4];
        }
    }

    t_print("Sensor server starting...\n");
    start_server(atoi(port_number), serial_display_path);
    exit(0);
} 