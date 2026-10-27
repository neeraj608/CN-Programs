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
        printf("Socket failed\n");
        close(serversocket);
    }

    printf("Socket connected!\n");

    serveraddress.sin_family=AF_INET;
    serveraddress.sin_port=htons(9000);
    serveraddress.sin_addr.s_addr=htonl(INADDR_LOOPBACK);

if(bind(serversocket,(struct sockaddr*)&serveraddress,sizeof(serveraddress))<0)    {

        printf("Binding failed!\n");
        close(serversocket);
    }

    printf("Binding successfull\n");

    listen(serversocket,5);

    clientsocket=accept(serversocket,NULL,NULL);

    char message[256];
int rec=recv(clientsocket,message,sizeof(message)-1,0);

if(rec>0)
{
    message[rec]='\0';
    printf("Message received: %s\n",message);
}
else
{
    printf("Message not received\n");
}

    close(serversocket);
    close(clientsocket);


}