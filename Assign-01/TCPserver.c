
#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>

int main()
{
    int serversocket,clientsocket;

    struct sockaddr_in serveraddress;

    serversocket=socket(AF_INET,SOCK_STREAM,0);

    if(serversocket<0)
    {
        printf("Socket creation failed!\n");
        return 1;
    }

    printf("Socket successful!\n");

    serveraddress.sin_family=AF_INET;
    serveraddress.sin_port=htons(9000);
    serveraddress.sin_addr.s_addr=htonl(INADDR_LOOPBACK);

    if(bind(serversocket,(struct sockaddr*)&serveraddress,sizeof(serveraddress))<0)
    {
        printf("Binding failed!\n");
        close(serversocket);
        return 1;
    }

    printf("Binding successful!\n");

    if(listen(serversocket,5)<0)
    {
        printf("Listen failed!\n");
        close(serversocket);
        return 1;
    }

    printf("Waiting for client...\n");

    clientsocket=accept(serversocket,NULL,NULL);

    if(clientsocket<0)
    {
        printf("Accept failed!\n");
        close(serversocket);
        return 1;
    }

    printf("Client connected!\n");
    printf("Type EXIT to stop chatting.\n");

    char message[256];
    char reply[256];

    while(1)
    {
        int rec=recv(clientsocket,message,sizeof(message)-1,0);

        if(rec<=0)
        {
            printf("Client disconnected!\n");
            break;
        }

        message[rec]='\0';

        printf("Client: %s\n",message);

        if(strcmp(message,"EXIT")==0)
        {
            printf("Chat ended by client!\n");
            break;
        }

        printf("Server: ");
        fgets(reply,sizeof(reply),stdin);
        reply[strcspn(reply,"\n")]='\0';

        if(send(clientsocket,reply,strlen(reply),0)<0)
        {
            printf("Message sending failed!\n");
            break;
        }

        if(strcmp(reply,"EXIT")==0)
        {
            printf("Chat ended by server!\n");
            break;
        }
    }

    close(clientsocket);
    close(serversocket);

    return 0;
}
