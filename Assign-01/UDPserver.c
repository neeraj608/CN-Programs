#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>

int main()
{
    int serversocket;
    struct sockaddr_in serveraddress, clientaddress;
    socklen_t clientlen = sizeof(clientaddress);

    serversocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(serversocket < 0)
    {
        printf("Socket creation failed!\n");
        return 1;
    }

    printf("Socket successful!\n");

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(bind(serversocket, (struct sockaddr*)&serveraddress,
            sizeof(serveraddress)) < 0)
    {
        printf("Binding failed!\n");
        close(serversocket);
        return 1;
    }

    printf("Binding successful!\n");
    printf("Waiting for message...\n");

    char message[256];

    int rec = recvfrom(serversocket, message, sizeof(message)-1, 0,
                       (struct sockaddr*)&clientaddress, &clientlen);

    if(rec > 0)
    {
        message[rec] = '\0';
        printf("Message received: %s\n", message);
    }
    else if(rec == 0)
    {
        printf("Empty message received\n");
    }
    else
    {
        printf("Message not received\n");
    }

    close(serversocket);
    return 0;
}