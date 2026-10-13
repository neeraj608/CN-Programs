
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

struct packet
{
    int seqNo;
    int freq;
    char data[2];
};

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    printf("Socket created successfully\n");

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(clientSocket,
        (struct sockaddr *)&serverAddress,
        sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        close(clientSocket);
        return 1;
    }

    printf("Connection established.\n");

    char bits[101];
    char ack[10];

    printf("Enter binary data: ");
    scanf("%100s", bits);

    // Validate input
    for(int i = 0; bits[i] != '\0'; i++)
    {
        if(bits[i] != '0' && bits[i] != '1')
        {
            printf("Invalid input. Enter only 0 and 1.\n");
            close(clientSocket);
            return 1;
        }
    }

    printf("\nOriginal Data: %s\n\n", bits);

    // Send each bit using its assigned frequency
    for(int i = 0; bits[i] != '\0'; i++)
    {
        struct packet p;

        p.seqNo = i;

        if(bits[i] == '1')
            p.freq = 100;
        else
            p.freq = 200;

        p.data[0] = bits[i];
        p.data[1] = '\0';

        printf("Hop %d | Bit = %c | Frequency = %d Hz\n",
               i + 1, p.data[0], p.freq);

        if(send(clientSocket, &p, sizeof(p), 0) <= 0)
        {
            perror("Send failed");
            close(clientSocket);
            return 1;
        }

        int n = recv(clientSocket, ack, sizeof(ack) - 1, 0);

        if(n <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        ack[n] = '\0';
        printf("ACK: %s\n\n", ack);
    }

    close(clientSocket);

    return 0;
}
