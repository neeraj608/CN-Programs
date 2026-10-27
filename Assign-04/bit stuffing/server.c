
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

    char frame[2020];
    char stuffed[2000];
    char data[2000];

    int received = recv(clientsocket, frame, sizeof(frame) - 1, 0);

    if(received <= 0)
    {
        perror("Receive failed");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    frame[received] = '\0';

    printf("Received frame: %s\n", frame);

    char flag[] = "01111110";
    int flagLength = 8;
    int frameLength = 0;

    /* Calculate frame length manually */
    while(frame[frameLength] != '\0')
    {
        frameLength++;
    }

    /* Check starting and ending flags */
    int valid = 1;

    if(frameLength < 2 * flagLength)
    {
        valid = 0;
    }

    /* Check starting flag */
    if(valid == 1)
    {
        for(int i = 0; i < flagLength; i++)
        {
            if(frame[i] != flag[i])
            {
                valid = 0;
                break;
            }
        }
    }

    /* Check ending flag */
    if(valid == 1)
    {
        for(int i = 0; i < flagLength; i++)
        {
            if(frame[frameLength - flagLength + i] != flag[i])
            {
                valid = 0;
                break;
            }
        }
    }

    if(valid == 0)
    {
        printf("Invalid frame!\n");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    /* Extract stuffed data between flags */
    int stuffedLength = frameLength - 2 * flagLength;

    for(int i = 0; i < stuffedLength; i++)
    {
        stuffed[i] = frame[flagLength + i];
    }

    stuffed[stuffedLength] = '\0';

    printf("Stuffed data: %s\n", stuffed);

    /* Destuff the data */
    int j = 0;
    int count = 0;

    for(int i = 0; i < stuffedLength; i++)
    {
        data[j++] = stuffed[i];

        if(stuffed[i] == '1')
        {
            count++;
        }
        else
        {
            count = 0;
        }

        if(count == 5)
        {
            if(i + 1 >= stuffedLength || stuffed[i + 1] != '0')
            {
                printf("Invalid stuffed data!\n");
                close(clientsocket);
                close(serversocket);
                return 1;
            }

            i++;       /* Skip the stuffed zero */
            count = 0;
        }
    }

    data[j] = '\0';

    printf("Destuffed data: %s\n", data);

    close(clientsocket);
    close(serversocket);

    return 0;
}
