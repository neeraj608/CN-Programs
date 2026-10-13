
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MSG_SIZE 100
#define MAX_FRAME_SIZE 120
#define MAX_FINAL_FRAME 1000
#define MAX_FRAMES 20
#define COUNT_SIZE 3

struct frames
{
    char frame[MAX_FRAME_SIZE];
};

int stringLength(char str[])
{
    int length = 0;

    while (str[length] != '\0')
        length++;

    return length;
}

void removeNewline(char str[])
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
            return;
        }
    }
}

int createFrame(char msg[], char frame[])
{
    int msgLength = stringLength(msg);
    int totalLength = COUNT_SIZE + msgLength;

    if (totalLength >= MAX_FRAME_SIZE || totalLength > 999)
        return 0;

    /* First 3 characters contain the total frame length. */
    frame[0] = '0' + totalLength / 100;
    frame[1] = '0' + (totalLength / 10) % 10;
    frame[2] = '0' + totalLength % 10;

    for (int i = 0; i < msgLength; i++)
        frame[COUNT_SIZE + i] = msg[i];

    frame[totalLength] = '\0';

    return 1;
}

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

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        printf("Socket is not created\n");
        return 1;
    }

    printf("Socket is created successfully\n");

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(clientSocket, (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        close(clientSocket);
        return 1;
    }

    printf("Connection is established.\n");

    int n;

    printf("Enter the number of frames: ");

    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_FRAMES)
    {
        printf("Invalid number of frames.\n");
        close(clientSocket);
        return 1;
    }

    struct frames f[MAX_FRAMES];
    char final_frame[MAX_FINAL_FRAME];
    int finalIndex = 0;

    getchar();

    for (int i = 0; i < n; i++)
    {
        char msg[MAX_MSG_SIZE];

        printf("Enter message or bits: ");

        if (fgets(msg, sizeof(msg), stdin) == NULL)
        {
            printf("Failed to read message\n");
            close(clientSocket);
            return 1;
        }

        removeNewline(msg);

        if (!createFrame(msg, f[i].frame))
        {
            printf("Frame is too large.\n");
            close(clientSocket);
            return 1;
        }

        int frameLength = stringLength(f[i].frame);

        if (finalIndex + frameLength >= MAX_FINAL_FRAME)
        {
            printf("Combined frames are too large.\n");
            close(clientSocket);
            return 1;
        }

        for (int j = 0; j < frameLength; j++)
            final_frame[finalIndex++] = f[i].frame[j];

        printf("Original message: %s\n", msg);
        printf("Message length: %d\n", stringLength(msg));
        printf("Total frame length: %d\n", frameLength);
        printf("Frame: %s\n\n", f[i].frame);
    }

    final_frame[finalIndex] = '\0';

    printf("Final combined frames: %s\n", final_frame);

    if (!sendAll(clientSocket, final_frame, finalIndex))
    {
        perror("Send failed");
        close(clientSocket);
        return 1;
    }

    /* Tell the server that no more data will be sent. */
    shutdown(clientSocket, SHUT_WR);

    char ack[10];

    int receivedBits = recv(clientSocket, ack, sizeof(ack) - 1, 0);

    if (receivedBits > 0)
    {
        ack[receivedBits] = '\0';

        if (strcmp(ack, "OK") == 0)
            printf("Acknowledgment received.\n");
        else
            printf("Server response: %s\n", ack);
    }
    else
    {
        printf("Acknowledgment not received.\n");
    }

    close(clientSocket);

    return 0;
}
