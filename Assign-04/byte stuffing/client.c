
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int clientsocket;
    struct sockaddr_in serveraddress = {0};

    clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientsocket < 0)
    {
        perror("Socket failed");
        return 1;
    }

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(connect(clientsocket, (struct sockaddr *)&serveraddress,
               sizeof(serveraddress)) < 0)
    {
        perror("Connection failed");
        close(clientsocket);
        return 1;
    }

    printf("Connected to server\n");

    char data[100], stuffed[200];

    printf("Enter data: ");
    scanf("%99s", data);

    int j = 0;

    /* Add starting flag */
    stuffed[j] = '$';
    j++;

    /* Perform byte stuffing */
    for(int i = 0; data[i] != '\0'; i++)
    {
        if(data[i] == '$' || data[i] == '/')
        {
            stuffed[j] = '/';
            j++;
        }

        stuffed[j] = data[i];
        j++;
    }

    /* Add ending flag */
    stuffed[j] = '$';
    j++;

    stuffed[j] = '\0';

    printf("$ = FLAG\n");
    printf("/ = ESC\n");

    printf("Stuffed data: %s\n", stuffed);

    int sent = send(clientsocket, stuffed, strlen(stuffed) + 1, 0);

    if(sent < 0)
    {
        perror("Send failed");
        close(clientsocket);
        return 1;
    }

    close(clientsocket);

    return 0;
}
