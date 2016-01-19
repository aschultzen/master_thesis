#include "sensor_client.h"
#include "net.h"
#include "protocol.h"
#include "utils.h"

char *nmea_string = 
"NMEA $GPRMC,134116.00,A,5957.80516,N,01043.82047,E,0.874,,150116,,,A*72\
$GPVTG,,T,,M,0.874,N,1.619,K,A*27\
$GPGGA,134116.00,5957.80516,N,01043.82047,E,1,06,2.00,194.6,M,38.5,M,,*57\
$GPGSA,A,3,05,02,09,06,23,16,,,,,,,4.36,2.00,3.87*03\
$GPGSV,4,1,13,02,26,240,32,05,48,287,33,06,05,204,12,07,67,144,*7A\
$GPGSV,4,2,13,09,48,097,16,13,10,262,,16,21,032,07,20,05,312,*71\
$GPGSV,4,3,13,23,14,092,14,26,04,012,07,29,07,314,,30,44,190,*73\
$GPGSV,4,4,13,33,18,209,*49\
$GPGLL,5957.80516,N,01043.82047,E,134116.00,A,A*62 \
$GPTXT,01,01,01,NMEA unknown msg*58";
int nmea_string_length = 560;

int identify(int session_fd, int id)
{
    //Converting from int to string
    char id_str[5];
    bzero(id_str, 5);
    sprintf(id_str, " %d", id); //Notice the space in the second parameter.
    int read_status = 0;

    //Declaring message string
    char identify_message[sizeof(PROTOCOL_IDENTIFY) + sizeof(id_str) + 1];

    //copying
    memcpy(identify_message, PROTOCOL_IDENTIFY, sizeof(PROTOCOL_IDENTIFY));
    memcpy(&identify_message[8],id_str, sizeof(id_str));

    write(session_fd, identify_message, sizeof(identify_message));

    char iobuffer[100];
    while ( (read_status = read(session_fd, iobuffer, sizeof(iobuffer)-1)) > 0) {
        if(strstr((char*)iobuffer, PROTOCOL_OK ) == (iobuffer)) {
            // ID not used. Accepting.
            t_print("ID %d accepted by server.\n", id);
            return 0;
        } else {
            // ID in use. Rejected.
            t_print("ID %d rejected by server, already in use.\n", id);
            return -1;
        }
    }
    // Something happened during read.
    // read() returns -1 at error
    return read_status;
}

int create_connection(char *iobuffer, struct sockaddr_in *serv_addr, int *session_fd, char *ip, int portno)
{
    if((*session_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Could not create socket\n");
        return -1;
    }

    memset(serv_addr, '0', sizeof(*serv_addr));

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(portno);

    if(inet_pton(AF_INET, ip, &(serv_addr->sin_addr))<=0) {
        printf("inet_pton error occured!\n");
        return 1;
    }

    if( connect(*session_fd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        t_print("Connection failed\n");
        return 1;
    }

    return 0;
}

int start_client(int portno, char* ip, int id)
{
    int n = 0;
    char iobuffer[1024];
    struct sockaddr_in serv_addr;
    int session_fd = 1;
    int connection_attempts = 1;

    memset(iobuffer, '0',sizeof(iobuffer));

    while(connection_attempts <= CONNECTION_ATTEMPTS_MAX) {
        if(create_connection(iobuffer, &serv_addr, &session_fd, ip, portno) == 0) {
            t_print("Connected to server!\n");
            break;
        }
        t_print("Connection attempt %d failed. Code %d\n", connection_attempts,n);
        sleep(1);
        connection_attempts++;
    }

    if( identify(session_fd, id) == -1 ) {
        exit(0);
    }

    while (1) {
        write(session_fd, nmea_string, nmea_string_length);
        sleep(1);
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
    char *ip_address = NULL;
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
            ip_address = optarg;
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
    if(client_id == 0 || client_id > ID_MAX) {
        printf("Client ID can not be less than 1 or more than %d!\n", ID_MAX);
        exit(0);
    }

    if(ip_address == NULL || port_number == NULL) {
        printf("Missing parameters!\n");
        exit(0);
    }

    start_client(atoi(port_number), ip_address, client_id);
    return 0;
}
