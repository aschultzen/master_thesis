#include "sensor_client.h"
#include "net.h"
#include "protocol.h"

int identify(int session_fd, int id)
{
    //Converting from int to string
    char id_str[5];
    bzero(id_str, 5);
    sprintf(id_str, " %d", id); //Notice the space in the second parameter.

    //Declaring message string
    char identify_message[sizeof(PROTOCOL_IDENTIFY) + sizeof(id_str) + 1];

    //copying
    memcpy(identify_message, PROTOCOL_IDENTIFY, sizeof(PROTOCOL_IDENTIFY));
    memcpy(&identify_message[8],id_str, sizeof(id_str));

    write(session_fd, identify_message, sizeof(identify_message));
    
    char iobuffer[100];
    while ( (read(session_fd, iobuffer, sizeof(iobuffer)-1)) > 0) {
        if(strstr((char*)iobuffer, PROTOCOL_OK ) == (iobuffer)) 
        {
            t_print("It's ok!\n");
        }
        else
        {
            t_print("it's not\n");
        }
    }
    return 0;    
}

int start_client(int portno, char* ip, int id)
{
    int session_fd = 0;
    int n = 0;
    char iobuffer[1024];
    struct sockaddr_in serv_addr;

    memset(iobuffer, '0',sizeof(iobuffer));
    if((session_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0) {
        printf("inet_pton error occured!\n");
        return 1;
    }

    if( connect(session_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed!\n");
        return 1;
    }

    identify(session_fd, id);

    while ( (n = read(session_fd, iobuffer, sizeof(iobuffer)-1)) > 0) {
        //
    }

    if(n < 0) {
        printf("\n Read error \n");
    }

    return 0;
}

int usage(char *argv[])
{
    printf("Usage: %s -s <SERVER IP> -p <SERVER PORT> -i <CLIENT ID>\n", argv[0]);
    return 0;
}

int main(int argc, char *argv[])
{
    char *ip_adress = NULL;
    char *port_number = NULL;
    int client_id = 0;

    if(argc < 5) {
        usage(argv);
        return 0;
    }

    while (1) {
        char c;

        c = getopt (argc, argv, "s:p:i:");
        if (c == -1) {
            break;
        }
        switch (c) {
        case 's':
            ip_adress = optarg;
            break;
        case 'p':
            port_number = optarg;
            break;
        case 'i':
            client_id = atoi(optarg);
            break;
        default:
            usage(argv);
        }
    }
    if(client_id == 0 || client_id > ID_MAX){
        printf("Client ID can not be less than 1 or more than %d!\n", ID_MAX);
        exit(0);
    }

    if(ip_adress == NULL || port_number == NULL) {
        printf("Missing parameters!\n");
        exit(0);
    }

    start_client(atoi(port_number), ip_adress, client_id);
    return 0;
}
