
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_FRAME_SIZE 120
#define COUNT_SIZE 3

int sendAll(int socket, char data[], int length)
{
    int sent = 0;

    while (sent < length)
    {
        int n = send(socket, data + sent, length - sent, 0);

        if (n <= 0)
            return 0;

        sent += n;
    }

    return 1;
}

/* Read exactly the requested number of bytes. */
int receiveExact(int socket, char data[], int length)
{
    int received = 0;

    while (received < length)
    {
        int n = recv(socket, data + received, length - received, 0);

        if (n < 0)
            return -1;

        if (n == 0)
        {
            if (received == 0)
                return 0;

            return -1;
        }

        received += n;
    }

    return 1;
}

/* Read frames one by one using their byte-count fields. */
int receiveFrames(int clientSocket)
{
    int frameNumber = 1;

    while (1)
    {
        char count[COUNT_SIZE];
        int result = receiveExact(clientSocket, count, COUNT_SIZE);

        /* Client finished sending all frames. */
        if (result == 0)
            break;

        if (result < 0)
        {
            printf("Incomplete count field received.\n");
            return 0;
        }

        int frameLength = 0;

        for (int i = 0; i < COUNT_SIZE; i++)
        {
            if (count[i] < '0' || count[i] > '9')
            {
                printf("Invalid byte-count field.\n");
                return 0;
            }

            frameLength = frameLength * 10 + (count[i] - '0');
        }

        if (frameLength < COUNT_SIZE ||
            frameLength >= MAX_FRAME_SIZE)
        {
            printf("Invalid frame length: %d\n", frameLength);
            return 0;
        }

        int messageLength = frameLength - COUNT_SIZE;
        char msg[MAX_FRAME_SIZE];

        result = receiveExact(clientSocket,
                              msg, messageLength);

        if (result != 1)
        {
            printf("Incomplete frame received.\n");
            return 0;
        }

        msg[messageLength] = '\0';

        printf("\nFrame %d\n", frameNumber);
        printf("Byte count: %d\n", frameLength);
        printf("Received message: %s\n", msg);

        frameNumber++;
    }

    if (frameNumber == 1)
    {
        printf("No frames received.\n");
        return 0;
    }

    return 1;
}

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    if (bind(serverSocket, (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        perror("Binding failed");
        close(serverSocket);
        return 1;
    }

    printf("Binding successful\n");

    if (listen(serverSocket, 3) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    printf("Waiting for client connection...\n");

    clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    printf("Client connected successfully.\n");

    int valid = receiveFrames(clientSocket);

    if (valid)
    {
        char ack[] = "OK";
        sendAll(clientSocket, ack, sizeof(ack));
        printf("\nAll frames received successfully.\n");
    }
    else
    {
        char error[] = "ERROR";
        sendAll(clientSocket, error, sizeof(error));
        printf("\nFrame reception failed.\n");
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}
