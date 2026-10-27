
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int FREQ[8] = {2400, 2410, 2420, 2430, 2440, 2450, 2460, 2470};

typedef struct
{
    int t, seq, freq;
} Packet;

int main()
{
    /* CREATE SERVER SOCKET */
    int serversocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serversocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(5000);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serversocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Bind failed");
        close(serversocket);
        return 1;
    }

    if (listen(serversocket, 1) < 0)
    {
        perror("Listen failed");
        close(serversocket);
        return 1;
    }

    printf("Waiting for client...\n");

    int clientsocket = accept(serversocket, NULL, NULL);

    if (clientsocket < 0)
    {
        perror("Accept failed");
        close(serversocket);
        return 1;
    }

    /* RECEIVE PACKETS */
    char data[100][4];
    int n = 0;
    Packet p;

    while (recv(clientsocket, &p, sizeof(p), MSG_WAITALL) == sizeof(p))
    {
        if (p.t == -1)
        {
            n = p.seq;
            break;
        }

        int v;

        for (v = 0; v < 8; v++)
        {
            if (FREQ[v] == p.freq)
                break;
        }

        if (v == 8 || p.seq < 0 || p.seq >= 100)
            continue;

        data[p.seq][0] = ((v >> 2) & 1) + '0';
        data[p.seq][1] = ((v >> 1) & 1) + '0';
        data[p.seq][2] = (v & 1) + '0';
        data[p.seq][3] = '\0';

        printf("t%d: got seq=%d freq=%d -> %s\n",
               p.t, p.seq, p.freq, data[p.seq]);
    }

    /* RECONSTRUCT ORIGINAL DATA */
    printf("\nOriginal data: ");

    for (int i = 0; i < n; i++)
        printf("%s", data[i]);

    printf("\n");

    close(clientsocket);
    close(serversocket);

    return 0;
}
