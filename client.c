#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <getopt.h>

int getopt_long(int argc,
                char * const argv[],
                const char *optstring,
                const struct option *longopts,
                int *longindex);

struct option *long_options;

int main(int argc, char* argv[])
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    const char* short_options = "-:a:p:";
    char *ipv4_addr_str;
    char *port_str;
    int ip_flag = 0;
    int port_flag = 0;
    char buffer[256];
	

    int rez;
    int option_index = -1;
    while ((rez = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        switch (rez)
        {
            case 'a':
            {
                if (optarg)
                {
                    ipv4_addr_str = optarg;
                }
                else
                {
                    printf("Error: option -a required argument\n");
                    exit(1);
                }
                ip_flag++;
                break;
            };

            case 'p':
            {
                if (optarg)
                {
                    port_str = optarg;
                }
                else
                {
                    printf("Error: option -p required argument\n");
                    exit(1);
                }
                port_flag++;
                break;
            }
            case ':':
            {
                printf("Error: missing argument\n");
                exit(1);
            }
            case '?':
            {
                printf("Error: unknown option\n");
                exit(1);
            }
            default:
            {
                break;
            };
        };
    };
	
    if (ip_flag == 0)
    {
        ipv4_addr_str = getenv("L2ADDR");
    }
    if (port_flag == 0)
    {
        port_str = getenv("L2PORT");
    }
    if (ipv4_addr_str == NULL)
    {
    	printf("IP address not found\n");
    	exit(1);
    }
    if (port_str == NULL)
    {
    	printf("Port not found\n");
    	exit(1);
    }
    

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("Error wile creating socket\n");
        exit(1);
    }
	
    int port = atoi(port_str);
    if (port == 0)
    {
        printf("Error: wrong format of port number\n");
        exit(1);
    }
    server = gethostbyname(ipv4_addr_str);
    
    if (server == NULL)
    {
        printf("Error: no such host\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ipv4_addr_str);
    serv_addr.sin_port = htons(port);

    printf("Please enter the file name to copy: ");
    char *str;
    str = malloc(sizeof(char) * 255);
    bzero(buffer, 256);
	
    fgets(str, 255, stdin);
	
	char *fileName = malloc(sizeof(char) * 255);
	strncpy(fileName, str, strlen(str) - 1);
	
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error while connecting\n");
        close(sock);
        exit(1);
    }
    
	FILE *fileToCopy = fopen(fileName, "rb");
	if (fileToCopy == NULL)
	{
		printf("File doesn't exist\n");
		close(sock);
		exit(1);
	}
	
	
    int n;
    n = write(sock, str, strlen(str));
    if (n < 0)
    {
        printf("Error: writing to socket\n");
        close(sock);
        exit(1);
    }
	int bytes = 0;
	char by;
	
	
	while (!feof(fileToCopy))
	{
		fread(&by, sizeof(char), 1, fileToCopy);
		bytes++;
	}
	printf("bytes = %d\n", bytes);
	int byteC = htonl(bytes);
	
    n = write(sock, &byteC, sizeof(int));
    if (n < 0)
    {
        printf("Error: writing to socket\n");
        close(sock);
        exit(1);
    }
	fseek(fileToCopy, 0, SEEK_SET);
	
	for (int i = 0; i < bytes; i++)
	{
		fread(&by, sizeof(char), 1, fileToCopy);
		n = send(sock, &by, sizeof(char), 0);
		if (n < 0)
		    {
		        printf("Error: writing to socket YYY\n");
		        close(sock);
		        exit(1);
		    }
	}
	
	
	
	
	
	//bytes = 324834;
	n = write(sock, -1, sizeof(char));

    char *string_out = malloc(sizeof(char) * 255);

    //recv(sock, buffer, strlen(buffer), 0);
    n = read(sock, string_out, 255);

    if (n < 0)
    {
        printf("Error: writing to socket\n");
        close(sock);
        exit(1);
    }
    printf(" out = %s\n", string_out);
    bzero(buffer, 256);
    close(sock);
	close(fileToCopy);




    return 0;
}