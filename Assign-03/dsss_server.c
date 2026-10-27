#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define N 11

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress;

    char pn[] = "10110111000";
    char packet[N];
    char decoded[100];
    int count = 0;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        perror("Socket failed");
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 3) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    printf("Waiting for client...\n");

    clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    printf("Client connected.\n");

    while (count < 99)
    {
        int received = recv(clientSocket, packet, N, MSG_WAITALL);

        if (received < N)
            break;

        int zeros = 0, ones = 0;

        printf("Received: %.*s\n", N, packet);

        for (int j = 0; j < N; j++)
        {
            int bit = (packet[j] - '0') ^ (pn[j] - '0');

            if (bit == 0)
                zeros++;
            else
                ones++;
        }

        if (zeros == N)
            decoded[count] = '0';
        else if (ones == N)
            decoded[count] = '1';
        else
            decoded[count] = '?';

        printf("Decoded bit: %c\n", decoded[count]);

        count++;

        if (send(clientSocket, "OK", 2, 0) != 2)
            break;
    }

    decoded[count] = '\0';

    printf("\nOriginal data: %s\n", decoded);

    close(clientSocket);
    close(serverSocket);

    return 0;
}
