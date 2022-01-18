#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>

#define BUFFER_SIZE 1000
#define NAME_SIZE 30

void *Send_Message(void *arg);
void *Receive_Message(void *arg);

int main(int argc , char *argv[])
{
        pthread_t sendThreadID, recvThreadID;
        int createSocket, port, n;
        struct sockaddr_in server_addr;
        struct hostent *server;
        char name[NAME_SIZE];

        if(argc<3)
        {
                printf("\nERROR\n");
                printf("[*]Usage: %s 'HOSTNAME' 'PORT'\n\n", argv[0]);
                return 0;
        }

        //Create client socket
        port = atoi(argv[2]);
        createSocket = socket(AF_INET , SOCK_STREAM , 0);
        if ( createSocket < 0)
        {
                printf("\n[*]Could not create socket");
        }
        else
                printf("\n[*]Success create socket\n");

        //search host(server) if available
        server = gethostbyname(argv[1]);
        if(server == NULL)
        {
                printf("Sorry no such host");
        }

        //sockaddr_in structure
        bzero((char *)&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,(char *)&server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons( port );

        //Connect to server
        if (connect(createSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
                printf("[*]connect error \n\n");
                return 1;
        }
        else
        {
                printf("[*]Connected...\n");
                printf("[*]Enter Your Name: ");

                fgets(name, NAME_SIZE, stdin);
                name[strlen(name)-1] = '\0';

                pthread_create(&sendThreadID, NULL, Send_Message,(void*)&createSocket);
                pthread_create(&recvThreadID, NULL, Receive_Message,(void*)&createSocket);
        }

        pthread_join(sendThreadID, NULL);
        pthread_join(recvThreadID, NULL);

        close(createSocket);
        return 0;
}

void *Send_Message(void *arg)
{
        int connectedClient = *((int*)arg);
        char message[BUFFER_SIZE] = {0};
        char NameMessage[BUFFER_SIZE+NAME_SIZE+30] = {0};

        while(1)
        {
                fgets(message,BUFFER_SIZE, stdin);
                if(message[0] == 'x' || message[0] == 'X')
                {
                        close(connectedClient);
                        exit(0);
                }
                else
                {
                        printf(NameMessage, "[%s]; %s", tzname, message);
                        write(connectedClient, NameMessage, strlen(NameMessage));
                }
        }
        return NULL;
}

void *Receive_Message(void *arg)
{
        int connectedClient = *((int*)arg);
        char NameMessage[BUFFER_SIZE+NAME_SIZE+1] = {0};

        int strLen = 0;

        for(;;)
        {
                strLen = read(connectedClient, NameMessage, BUFFER_SIZE+NAME_SIZE);
                if(strLen == 1)
                {
                        exit(1);
                }
                else
                {
                        NameMessage[strLen] = '\0';
                        printf("%s", NameMessage);
                }
        }
        return NULL;
}
