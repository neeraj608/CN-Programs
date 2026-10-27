#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define N 11
 
int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;

    char bits[100];
    char pn[] = "10110111000";
    char packet[N];
    char ack[2];

    printf("Enter data bits: ");
    scanf("%99s", bits);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        perror("Socket failed");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        close(clientSocket);
        return 1;
    }

    printf("Connected to server.\n");

    for (int i = 0; bits[i] != '\0'; i++)
    {
        if (bits[i] != '0' && bits[i] != '1')
        {
            printf("Invalid input.\n");
            close(clientSocket);
            return 1;
        }

        for (int j = 0; j < N; j++)
        {
            packet[j] = ((bits[i] - '0') ^ (pn[j] - '0')) + '0';
        }

        printf("Bit %c -> Sent: %.*s\n", bits[i], N, packet);

        if (send(clientSocket, packet, N, 0) != N)
        {
            perror("Send failed");
            close(clientSocket);
            return 1;
        }

        if (recv(clientSocket, ack, 2, MSG_WAITALL) != 2 ||
            memcmp(ack, "OK", 2) != 0)
        {
            printf("ACK failed.\n");
            close(clientSocket);
            return 1;
        }
    }

    close(clientSocket);
    return 0;
}

