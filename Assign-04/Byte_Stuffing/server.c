
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MSG_SIZE 200
#define MAX_FINAL_FRAME 1000
#define FLAG "FLAG"
#define ESC "ESC"
#define FLAG_LEN 4
#define ESC_LEN 3

int stringLength(char str[])
{
    int length = 0;

    while (str[length] != '\0')
        length++;

    return length;
}

void clearMemory(void *address, int size)
{
    char *bytes = (char *)address;

    for (int i = 0; i < size; i++)
        bytes[i] = 0;
}

int matchesAt(char str[], int start, char pattern[], int patternLength)
{
    for (int i = 0; i < patternLength; i++)
    {
        if (str[start + i] != pattern[i])
            return 0;
    }

    return 1;
}

int receiveAllData(int socket, char data[], int capacity)
{
    int received = 0;

    while (received < capacity - 1)
    {
        int n = recv(socket, data + received,
                     capacity - 1 - received, 0);

        if (n < 0)
            return -1;

        if (n == 0)
            break;

        received += n;
    }

    data[received] = '\0';

    return received;
}

int removeFlagPatterns(char msg[], char originalMsg[])
{
    int msgLength = stringLength(msg);
    int originalIndex = 0;

    if (msgLength < 2 * FLAG_LEN ||
        !matchesAt(msg, 0, FLAG, FLAG_LEN) ||
        !matchesAt(msg, msgLength - FLAG_LEN, FLAG, FLAG_LEN))
    {
        printf("Invalid frame flags.\n");
        return 0;
    }

    int start = FLAG_LEN;
    int end = msgLength - FLAG_LEN;

    for (int i = start; i < end;)
    {
        if (matchesAt(msg, i, ESC, ESC_LEN))
        {
            i += ESC_LEN;

            if (i + FLAG_LEN <= end &&
                matchesAt(msg, i, FLAG, FLAG_LEN))
            {
                for (int j = 0; j < FLAG_LEN; j++)
                    originalMsg[originalIndex++] = FLAG[j];

                i += FLAG_LEN;
            }
            else if (i + ESC_LEN <= end &&
                     matchesAt(msg, i, ESC, ESC_LEN))
            {
                for (int j = 0; j < ESC_LEN; j++)
                    originalMsg[originalIndex++] = ESC[j];

                i += ESC_LEN;
            }
            else
            {
                printf("Invalid escape sequence.\n");
                return 0;
            }
        }
        else if (matchesAt(msg, i, FLAG, FLAG_LEN))
        {
            printf("Unexpected FLAG inside data.\n");
            return 0;
        }
        else
        {
            originalMsg[originalIndex++] = msg[i++];
        }

        if (originalIndex >= MAX_MSG_SIZE - 1)
        {
            printf("Original message is too large.\n");
            return 0;
        }
    }

    originalMsg[originalIndex] = '\0';

    return 1;
}

int separateFinalFrame(char finalFrame[])
{
    int finalFrameLength = stringLength(finalFrame);
    int currentIndex = 0;
    int frameNumber = 1;

    while (currentIndex < finalFrameLength)
    {
        char msg[MAX_MSG_SIZE];
        char originalMsg[MAX_MSG_SIZE];
        int msgIndex = 0;

        if (!matchesAt(finalFrame, currentIndex, FLAG, FLAG_LEN))
        {
            printf("Invalid frame: starting FLAG not found.\n");
            return 0;
        }

        for (int i = 0; i < FLAG_LEN; i++)
            msg[msgIndex++] = finalFrame[currentIndex++];

        int endFound = 0;

        while (currentIndex < finalFrameLength)
        {
            if (matchesAt(finalFrame, currentIndex, ESC, ESC_LEN))
            {
                if (msgIndex + ESC_LEN >= MAX_MSG_SIZE)
                {
                    printf("Frame is too large.\n");
                    return 0;
                }

                for (int i = 0; i < ESC_LEN; i++)
                    msg[msgIndex++] = finalFrame[currentIndex++];

                if (matchesAt(finalFrame, currentIndex, FLAG, FLAG_LEN))
                {
                    if (msgIndex + FLAG_LEN >= MAX_MSG_SIZE)
                    {
                        printf("Frame is too large.\n");
                        return 0;
                    }

                    for (int i = 0; i < FLAG_LEN; i++)
                        msg[msgIndex++] = finalFrame[currentIndex++];
                }
                else if (matchesAt(finalFrame, currentIndex, ESC, ESC_LEN))
                {
                    if (msgIndex + ESC_LEN >= MAX_MSG_SIZE)
                    {
                        printf("Frame is too large.\n");
                        return 0;
                    }

                    for (int i = 0; i < ESC_LEN; i++)
                        msg[msgIndex++] = finalFrame[currentIndex++];
                }
                else
                {
                    printf("Invalid escape sequence.\n");
                    return 0;
                }
            }
            else if (matchesAt(finalFrame, currentIndex, FLAG, FLAG_LEN))
            {
                for (int i = 0; i < FLAG_LEN; i++)
                    msg[msgIndex++] = finalFrame[currentIndex++];

                endFound = 1;
                break;
            }
            else
            {
                if (msgIndex >= MAX_MSG_SIZE - 1)
                {
                    printf("Frame is too large.\n");
                    return 0;
                }

                msg[msgIndex++] = finalFrame[currentIndex++];
            }
        }

        msg[msgIndex] = '\0';

        if (!endFound)
        {
            printf("Invalid frame: ending FLAG not found.\n");
            return 0;
        }

        printf("\nFrame %d\n", frameNumber);
        printf("Received Msg: %s\n", msg);

        if (!removeFlagPatterns(msg, originalMsg))
        {
            printf("Failed to recover original message.\n");
            return 0;
        }

        printf("Original message: %s\n", originalMsg);

        frameNumber++;
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

    clearMemory(&serverAddress, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

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

    char finalFrame[MAX_FINAL_FRAME];

    int n = receiveAllData(clientSocket, finalFrame,
                          sizeof(finalFrame));

    if (n <= 0)
    {
        if (n < 0)
            perror("Receive failed");
        else
            printf("No data received.\n");

        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    printf("Received final frame: %s\n", finalFrame);

    int valid = separateFinalFrame(finalFrame);

    char ack[] = "OK";
    char error[] = "ERROR";

    if (valid)
        send(clientSocket, ack, stringLength(ack) + 1, 0);
    else
        send(clientSocket, error, stringLength(error) + 1, 0);

    close(clientSocket);
    close(serverSocket);

    return 0;
}
