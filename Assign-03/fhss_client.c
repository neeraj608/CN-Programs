
#include <stdio.h>
#include <string.h>
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
    char bits[300];

    printf("Enter bitstream (multiple of 3 bits): ");
    scanf("%299s", bits);

    int len = strlen(bits);

    if (len == 0 || len % 3 != 0)
    {
        printf("Length must be a multiple of 3\n");
        return 1;
    }

    for (int i = 0; i < len; i++)
    {
        if (bits[i] != '0' && bits[i] != '1')
        {
            printf("Only 0 and 1 allowed\n");
            return 1;
        }
    }

    /* CONVERT DATA TO FREQUENCIES */
    int n = len / 3;
    char data[100][4];
    int val[100], freq[100], sent[100] = {0};

    if (n > 100)
    {
        printf("Maximum 100 chunks allowed\n");
        return 1;
    }

    for (int i = 0; i < n; i++)
    {
        strncpy(data[i], bits + i * 3, 3);
        data[i][3] = '\0';

        val[i] = (bits[i * 3] - '0') * 4 +
                 (bits[i * 3 + 1] - '0') * 2 +
                 (bits[i * 3 + 2] - '0');

        freq[i] = FREQ[val[i]];
    }

    printf("Data: ");

    for (int i = 0; i < n; i++)
        printf("%s ", data[i]);

    printf("\nFrequency: ");

    for (int i = 0; i < n; i++)
        printf("%d ", freq[i]);

    printf("\n\n");

    /* CONNECT TO SERVER */
    int clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientsocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(5000);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientsocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Connection failed");
        close(clientsocket);
        return 1;
    }

    /* SEND DATA SLOT BY SLOT */
    int left = n;

    for (int t = 0; left > 0; t++)
    {
        int used[8] = {0};

        for (int i = 0; i < n; i++)
        {
            if (sent[i] || used[val[i]])
                continue;

            used[val[i]] = 1;
            sent[i] = 1;
            left--;

            Packet p = {t, i, freq[i]};

            if (send(clientsocket, &p, sizeof(p), 0) != sizeof(p))
            {
                perror("Packet sending failed");
                close(clientsocket);
                return 1;
            }

            printf("t%d: sent seq=%d freq=%d\n", t, i, freq[i]);
        }

        sleep(1);
    }

    /* END MARKER */
    Packet end = {-1, n, 0};

    send(clientsocket, &end, sizeof(end), 0);

    close(clientsocket);

    return 0;
}
