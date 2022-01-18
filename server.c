#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  //inet_addr

#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1000

unsigned int connectedClientsArray[MAX_CLIENTS];
unsigned int clientsCounter = 0;

pthread_mutex_t aMutex = PTHREAD_MUTEX_INITIALIZER;

void *Handle_Client(void *arg);
void Send_Message(char msg[], int x, int recentClient);

int main(int argc, char *argv[])
{
        //argument fail if not execute with port
        if(argc < 2)
        {
                printf("\n[*]No port provided. Program end...\n\n*****NOTES*****\n");
                printf("[*]Usage: %s 'PORT' \n\n ", argv[0]);
                return 1;
        }

        pthread_mutex_init(&aMutex, NULL);


        //declaration
        int createSocket , clientSocket ,port, x, n;
        struct sockaddr_in server , client;
        socklen_t clien;

        //Create server socket
        createSocket = socket(AF_INET , SOCK_STREAM , 0);
        if (createSocket < 0 )
        {
                printf("[*]Could not create socket");
        }
        else
                printf("\n[*]Success create socket \n");

        //Prepare the sockaddr_in structure
        bzero((char *)&server, sizeof(server));
        port = atoi(argv[1]);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons( port );

        //Bind server
        if( bind(createSocket,(struct sockaddr *)&server , sizeof(server)) < 0 )
        {
               puts("[*]bind failed\n");
                return 1;
        }
        else
        {
                printf("[*]bind done at port %d\n",port);
        }

        //Listen
        if(listen(createSocket , MAX_CLIENTS) != -1)
        {
                printf("====WECLOME TO CHATTING ROOM====");
        }
        else
        {
                printf("Can't Listen\n");
                exit(1);
        }

        pthread_t threadID;

        //Client incoming connection
        puts("[*]Waiting for client connections...");

        //Client connected or not
        while(1)
        {
                memset(&client, 0, sizeof(client));
                clien=sizeof(client);
                clientSocket = accept(createSocket, (struct sockaddr *)&client, &clien);
                if (clientSocket<0)
                {
                        perror("[*]accept failed");
                }
                else
                {
                        pthread_mutex_lock(&aMutex);
                        connectedClientsArray[clientsCounter++] = clientSocket;
                        pthread_mutex_unlock(&aMutex);

                        pthread_create(&threadID, NULL, Handle_Client, (void*)&clientSocket);
                        pthread_detach(threadID);

                        printf("connected client IP: %s \n", inet_ntoa(client.sin_addr));
                }
        }
        close(createSocket);
        return 0;

}

void *Handle_Client(void *arg)
{
        int connectedClient = *((int*)arg);

        char message[BUFFER_SIZE];

        int strLen = 0;
        while((strLen = read(connectedClient, message, BUFFER_SIZE-1)) != 0)
        {
                Send_Message(message, strLen, connectedClient);
        }

        //remove client
        int i = 0;

        pthread_mutex_lock(&aMutex);

        for(i=0; i<clientsCounter; i++)
        {
                if(connectedClient == connectedClientsArray[i])
                {
                        while(i < clientsCounter && connectedClientsArray[i] !=0)
                        {
                                connectedClientsArray[i] = connectedClientsArray[i+1];
                                i++;
                        }
                        connectedClientsArray[i-1]=0;
                        break;
                }
                else
                {
                        puts("Error Array");
                }
        }
        --clientsCounter;
        printf("Disconnected Client\n");
        pthread_mutex_unlock(&aMutex);

        close(connectedClient);
        return NULL;
}

void Send_Message(char msg[], int x, int recentClient)
{
        int i;

        pthread_mutex_lock(&aMutex);

        for(i=0; i<clientsCounter; i++)
        {
                if(connectedClientsArray[i] == recentClient)
                        continue;
                        write(connectedClientsArray[i], msg, x);
        }
        pthread_mutex_unlock(&aMutex);
}


