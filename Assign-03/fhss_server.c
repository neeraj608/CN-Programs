
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct packet
{
    int seqNo;
    int freq;
    char data[2];
};

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(9000);

    if(bind(serverSocket,
        (struct sockaddr *)&serverAddress,
        sizeof(serverAddress)) < 0)
    {
        perror("Binding failed");
        close(serverSocket);
        return 1;
    }

    printf("Binding successful\n");

    if(listen(serverSocket, 3) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    printf("Waiting for client connection...\n");

    clientSocket = accept(serverSocket, NULL, NULL);

    if(clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    printf("Client connected successfully.\n\n");

    char original[101] = "";
    char ack[] = "OK";
    struct packet p;

    while(1)
    {
        int n = recv(clientSocket, &p, sizeof(p), 0);

        if(n <= 0)
            break;

        p.data[1] = '\0';

        int expectedFreq;

        if(p.data[0] == '1')
            expectedFreq = 100;
        else if(p.data[0] == '0')
            expectedFreq = 200;
        else
        {
            printf("Invalid bit received.\n");
            continue;
        }

        printf("Hop %d | Receiver frequency = %d Hz\n",
               p.seqNo + 1, p.freq);

        if(p.freq == expectedFreq)
        {
            printf("Frequency matched | Received bit = %c\n",
                   p.data[0]);

            int len = strlen(original);

            if(len < 100)
            {
                original[len] = p.data[0];
                original[len + 1] = '\0';
            }

            send(clientSocket, ack, strlen(ack) + 1, 0);
        }
        else
        {
            printf("Frequency mismatch. Data discarded.\n");
        }

        printf("\n");
    }

    printf("Recovered Data: %s\n", original);

    close(clientSocket);
    close(serverSocket);

    return 0;
}
