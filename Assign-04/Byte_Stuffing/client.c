
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_MSG_SIZE 200
#define MAX_STUFFED_SIZE 500
#define MAX_FINAL_FRAME 10000
#define MAX_FRAMES 10

#define FRAME_SEPARATOR '#'
#define FLAG "FLAG"
#define ESC "ESC"
#define FLAG_LEN 4
#define ESC_LEN 3

struct frames
{
    char frame[MAX_STUFFED_SIZE];
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

int matchesAt(char str[], int start, char pattern[], int patternLength)
{
    for (int i = 0; i < patternLength; i++)
    {
        if (str[start + i] != pattern[i])
            return 0;
    }

    return 1;
}

int appendCharacters(char destination[], int *index,
                     char source[], int length, int capacity)
{
    if (*index + length >= capacity)
        return 0;

    for (int i = 0; i < length; i++)
        destination[(*index)++] = source[i];

    return 1;
}

int byteStuff(char msg[], char stuffedMsg[])
{
    int msgLength = stringLength(msg);
    int stuffedIndex = 0;

    if (!appendCharacters(stuffedMsg, &stuffedIndex,
                          FLAG, FLAG_LEN, MAX_STUFFED_SIZE))
        return 0;

    for (int i = 0; i < msgLength;)
    {
        if (matchesAt(msg, i, ESC, ESC_LEN))
        {
            if (!appendCharacters(stuffedMsg, &stuffedIndex,
                                  ESC, ESC_LEN, MAX_STUFFED_SIZE) ||
                !appendCharacters(stuffedMsg, &stuffedIndex,
                                  ESC, ESC_LEN, MAX_STUFFED_SIZE))
                return 0;

            i += ESC_LEN;
        }
        else if (matchesAt(msg, i, FLAG, FLAG_LEN))
        {
            if (!appendCharacters(stuffedMsg, &stuffedIndex,
                                  ESC, ESC_LEN, MAX_STUFFED_SIZE) ||
                !appendCharacters(stuffedMsg, &stuffedIndex,
                                  FLAG, FLAG_LEN, MAX_STUFFED_SIZE))
                return 0;

            i += FLAG_LEN;
        }
        else
        {
            if (!appendCharacters(stuffedMsg, &stuffedIndex,
                                  &msg[i], 1, MAX_STUFFED_SIZE))
                return 0;

            i++;
        }
    }

    if (!appendCharacters(stuffedMsg, &stuffedIndex,
                          FLAG, FLAG_LEN, MAX_STUFFED_SIZE))
        return 0;

    stuffedMsg[stuffedIndex] = '\0';

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
        perror("Socket creation failed");
        return 1;
    }

    printf("Socket created successfully\n");

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

    printf("Connection established.\n");

    int frameCount;

    printf("Enter the number of frames: ");

    if (scanf("%d", &frameCount) != 1 ||
        frameCount < 1 || frameCount > MAX_FRAMES)
    {
        printf("Enter a frame count between 1 and %d.\n", MAX_FRAMES);
        close(clientSocket);
        return 1;
    }

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    struct frames f[MAX_FRAMES];

    for (int i = 0; i < frameCount; i++)
    {
        char msg[MAX_MSG_SIZE];
        char stuffedMsg[MAX_STUFFED_SIZE];

        printf("Enter message for frame %d: ", i + 1);

        if (fgets(msg, sizeof(msg), stdin) == NULL)
        {
            printf("Failed to read message.\n");
            close(clientSocket);
            return 1;
        }

        if (strchr(msg, '\n') == NULL && !feof(stdin))
        {
            printf("Message is too long.\n");
            close(clientSocket);
            return 1;
        }

        removeNewline(msg);

        if (!byteStuff(msg, stuffedMsg))
        {
            printf("Stuffed message is too large.\n");
            close(clientSocket);
            return 1;
        }

        strcpy(f[i].frame, stuffedMsg);

        printf("Original message: %s\n", msg);
        printf("Stuffed message: %s\n\n", stuffedMsg);
    }

    char finalFrame[MAX_FINAL_FRAME];
    int finalIndex = 0;

    for (int i = 0; i < frameCount; i++)
    {
        int length = stringLength(f[i].frame);

        if (finalIndex + length >= MAX_FINAL_FRAME)
        {
            printf("Combined frames are too large.\n");
            close(clientSocket);
            return 1;
        }

        for (int j = 0; j < length; j++)
            finalFrame[finalIndex++] = f[i].frame[j];
    }

    finalFrame[finalIndex] = '\0';

    printf("Final frame: %s\n", finalFrame);

    if (!sendAll(clientSocket, finalFrame, finalIndex))
    {
        perror("Sending failed");
        close(clientSocket);
        return 1;
    }

    shutdown(clientSocket, SHUT_WR);

    char ack[10];
    int n = recv(clientSocket, ack, sizeof(ack) - 1, 0);

    if (n > 0)
    {
        ack[n] = '\0';

        if (strcmp(ack, "OK") == 0)
            printf("Acknowledgment received.\n");
        else
            printf("Invalid acknowledgment.\n");
    }
    else
    {
        printf("Acknowledgment not received.\n");
        close(clientSocket);
        return 1;
    }

    close(clientSocket);

    return 0;
}
