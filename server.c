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
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_SIZE 1024

int sock;
struct sockaddr_in serv_addr;
struct hostent *server;

const char* short_options = "-:a:p:";
struct option *long_options;
char *ipv4_addr_str;
char *port_str;
int ip_flag = 0;
int port_flag = 0;
int daemon_mode = 0;

void* threadFunc(void* thread_data);
int daemon_func(void);

typedef struct {
    char *file_name;
	FILE *fout;
    int out_socket;
    struct sockaddr_in client;
} thread_dataS;


static void SIG_handler(int signum, siginfo_t *s, void *c)
{
    if (signum == SIGINT)
    {
        close(sock);
        exit(0);
    }
    if (signum == SIGHUP)
    {
        close(sock);
        exit(0);
    }
}


int main(int argc, char* argv[])
{
    struct sigaction sa = {0};
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGHUP);

    sa.sa_sigaction = SIG_handler;
    sa.sa_flags |= (SA_SIGINFO | SA_RESTART);

    //check(sigaction(SIGINT, &sa, NULL));
    //check(sigaction(SIGHUP, &sa, NULL));

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

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0)
	{
	    printf("Error wile creating socket\n");
	    exit(1);
	}

	int port = atoi(port_str);
	if (port == 0)
	{
	    printf("Error: wrong format of port number\n");
	    close(sock);
	    exit(1);
	}

	server = gethostbyname(ipv4_addr_str);
	if (server == NULL)
	{
	    printf("Error: no such host\n");
	    close(sock);
	    exit(1);
	}
	int res;
	res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
	if (res != 0)
	{
	    printf("Error: wrong sock options\n");
	    exit(1);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ipv4_addr_str);
	serv_addr.sin_port = htons(port);

	int x;
	x = bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (x != 0)
	{
	    printf("Error: while binding\n");
	    exit(1);
	}



	listen(sock, 10);

	pid_t parpid;
	//daemon_func();

	if ((parpid = fork()) < 0)
	{
	    printf("Error: creating fork with daemon\n");
	    exit(1);
	}
	else
	{
	    if (parpid != 0)
		{
	        exit(0);
	    }
	    setsid();
	    daemon_func();
	    chdir("/tmp");
	    close(stdin);
	    close(stdout);
	    close(stderr);
	 }
}

int daemon_func(void)
{
    int connfd, n;
    char *str_in;

    while(1)
    {
        str_in = malloc(sizeof(char) * 255);
        if (str_in == NULL)
        {
            exit(1);
        }
        connfd = accept(sock, (struct sockaddr*)NULL, NULL);
        if (connfd == -1)
        {
            exit(1);
        }

        n = read(connfd, str_in, 255);

        if (n == -1)
        {
            exit(1);
        }

        int length = 0;
        for (int i = 0; i < strlen(str_in); i++)
        {
            if (str_in[i] == '\n')
            {
                break;
            }
            else
            {
                length++;
            }
        }

        pthread_t *new_thread = malloc(sizeof(pthread_t));
        if (new_thread == NULL)
        {
            exit(1);
        }
        thread_dataS *td_in = malloc(sizeof(thread_dataS));
        if (td_in == NULL)
        {
            exit(1);
        }
        td_in->file_name = malloc(sizeof(char) * (length + 10));
        if (td_in->file_name == NULL)
        {
            exit(1);
        }
        td_in->out_socket = connfd;
		char *xx = malloc(sizeof(char) * 100);
		snprintf(xx, sizeof(char) * 100, "%s%s", "CP", str_in);
		strncpy(td_in->file_name, xx, length + 2);

        int rr;
        rr = pthread_create(&(new_thread), NULL, threadFunc, td_in);
        if (rr != 0)
        {
            exit(1);
        }
        free(str_in);

    }
    close(sock);
    return 0;
}


void* threadFunc(void* thread_data)
{
    thread_dataS *data = (thread_dataS *) thread_data;
	int ncheck;
	int buffX;
	int bytes;
	int byteC;
	char by;
	read(data->out_socket, &byteC, sizeof(int));
	bytes = ntohl(byteC);

	FILE *res = fopen(data->file_name, "w");
	for (int i = 0; i < bytes - 1; i++)
	{
		ncheck = recv(data->out_socket, &by, sizeof(char), 0);

		if (ncheck == -1)
		{
			close(res);
			break;
		}
		if (by == -1)
		{
			close(res);
			break;
		}
    fprintf(res, "%c", by);
		//putc(by, res);
	}

	int n = write(data->out_socket, "Successful", strlen("Successful"));

    close(data->out_socket);
	  fclose(res);
	//free(data);
    free(thread_data);
    pthread_detach(pthread_self());
    pthread_exit(0);
}
