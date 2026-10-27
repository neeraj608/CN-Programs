
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    int serversocket, clientsocket;
    struct sockaddr_in serveraddress = {0};

    serversocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serversocket < 0)
    {
        perror("Socket failed");
        return 1;
    }

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(serversocket, (struct sockaddr *)&serveraddress,
            sizeof(serveraddress)) < 0)
    {
        perror("Bind failed");
        close(serversocket);
        return 1;
    }

    if(listen(serversocket, 5) < 0)
    {
        perror("Listen failed");
        close(serversocket);
        return 1;
    }

    printf("Waiting for client...\n");

    clientsocket = accept(serversocket, NULL, NULL);

    if(clientsocket < 0)
    {
        perror("Accept failed");
        close(serversocket);
        return 1;
    }

    printf("Client connected\n");

    char stuffed[200], data[100];

    int received = recv(clientsocket, stuffed, sizeof(stuffed) - 1, 0);

    if(received <= 0)
    {
        perror("Receive failed");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    stuffed[received] = '\0';

    printf("Received: %s\n", stuffed);

    /* Check starting and ending flags */
    int len = strlen(stuffed);
    int valid = 1;

    if(len < 2 || stuffed[0] != '$' || stuffed[len - 1] != '$')
    {
        valid = 0;
    }

    if(valid == 0)
    {
        printf("Invalid frame!\n");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    /* Destuffing */
    int j = 0;

    for(int i = 1; i < len - 1; i++)
    {
        if(stuffed[i] == '/')
        {
            i++;

            if(i >= len - 1)
            {
                printf("Invalid stuffed data!\n");
                close(clientsocket);
                close(serversocket);
                return 1;
            }

            if(stuffed[i] != '$' && stuffed[i] != '/')
            {
                printf("Invalid stuffed data!\n");
                close(clientsocket);
                close(serversocket);
                return 1;
            }
        }

        data[j] = stuffed[i];
        j++;
    }

    data[j] = '\0';

    printf("Destuffed data: %s\n", data);

    close(clientsocket);
    close(serversocket);

    return 0;
}
