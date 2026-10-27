
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

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

    char data[1000], stuffed[2000], frame[2020];

   printf("Enter binary data: ");
   // %999s in scanf() means read a string containing at most 999 characters.
   scanf("%999s", data);

    int j = 0, count = 0;

    /* Validate and perform bit stuffing */
    for(int i = 0; data[i] != '\0'; i++)
    {
        if(data[i] != '0' && data[i] != '1')
        {
            printf("Invalid input! Enter only binary data.\n");
            close(clientsocket);
            return 1;
        }

        stuffed[j] = data[i];
        j++;


        if(data[i] == '1')
            count++;
        else
            count = 0;

        if(count == 5)
        {
            stuffed[j] = '0';
            j++;
            count = 0;
        }
    }

    stuffed[j] = '\0';

    /* Add flags after stuffing the data */
    //snprintf(destination, size, "format", values);
    snprintf(frame, sizeof(frame),
             "01111110%s01111110", stuffed);

    printf("Stuffed data: %s\n", stuffed);
    printf("Frame to send: %s\n", frame);

    
int sent = send(clientsocket, frame, strlen(frame), 0);

if(sent < 0)
{
    perror("Send failed");
    close(clientsocket);
    return 1;
}

    close(clientsocket);

    return 0;
}
