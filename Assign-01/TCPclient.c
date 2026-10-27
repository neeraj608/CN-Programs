#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>

int main()
{
    int clientsocket;
    struct sockaddr_in serveraddress;

    clientsocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(clientsocket < 0)
    {
        printf("Socket creation failed!\n");
        return 1;
    }

    printf("Socket successful!\n");

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    char hello[100];

    printf("Enter the Message: ");
    scanf("%99s", hello);

    int sent = sendto(clientsocket, hello, strlen(hello), 0,
                      (struct sockaddr*)&serveraddress,
                      sizeof(serveraddress));

    if(sent < 0)
    {
        printf("Message not sent\n");
        close(clientsocket);
        return 1;
    }

    printf("Message sent!\n");

    close(clientsocket);
    return 0;
}