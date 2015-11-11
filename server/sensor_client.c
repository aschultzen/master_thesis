#include "sensor_client.h"
#include "serial.h"

int start_client(int portno, char* ip)
{
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr;

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0) {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0) {
        recvBuff[n] = 0;
        if(fputs(recvBuff, stdout) == EOF) {
            printf("\n Error : Fputs error\n");
        }
    }

    if(n < 0) {
        printf("\n Read error \n");
    }

    return 0;
}

int usage(char *argv[])
{
    printf("Usage: %s -s <SERVER IP> -p <SERVER PORT>\n", argv[0]);
    return 0;
}

int main(int argc, char *argv[])
{
    char *ip_adress = NULL;
    char *port_number = NULL;

    if(argc < 4) {
        usage(argv);
        return 0;
    }

    while (1) {
        char c;

        c = getopt (argc, argv, "s:p:");
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
        default:
            usage(argv);
        }
    }

    if(ip_adress == NULL || port_number == NULL) {
        printf("Missing parameters!\n");
    }

    start_client(atoi(port_number), ip_adress);
    return 0;
}
