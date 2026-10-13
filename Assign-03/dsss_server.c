#include "dsss_common.h"

// XOR two binary strings
void xorBits(char a[], char b[], char result[])
{
    for(int i = 0; i < 4; i++)
    {
        if(a[i] == b[i])
            result[i] = '0';
        else
            result[i] = '1';
    }

    result[4] = '\0';
}

int main()
{
    int sock;

    struct sockaddr_in server;
    struct sockaddr_in client;

    socklen_t clientLen = sizeof(client);

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind
    if(bind(
        sock,
        (struct sockaddr *)&server,
        sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(sock);
        return 1;
    }

    printf("DSSS Server listening on port %d...\n", PORT);
    printf("PN Code: %s\n\n", pnCode);

    Packet p = {0};

    // Receive the complete combined data
    int bytes = recvfrom(
        sock,
        &p,
        sizeof(p),
        0,
        (struct sockaddr *)&client,
        &clientLen
    );

    if(bytes != sizeof(p))
    {
        printf("Invalid or incomplete packet received.\n");
        close(sock);
        return 1;
    }

    p.data[400] = '\0';

    printf("Combined Spread Data Received: %s\n", p.data);

    int len = strlen(p.data);

    // Validate spread-data length
    if(len == 0 || len % 4 != 0)
    {
        printf("Invalid spread data length.\n");
        close(sock);
        return 1;
    }

    char original[101] = "";
    int originalIndex = 0;

    // Despread each 4-bit group
    for(int i = 0; i < len; i += 4)
    {
        char received[5];
        char result[5];

        memcpy(received, p.data + i, 4);
        received[4] = '\0';

        // Validate binary characters
        for(int j = 0; j < 4; j++)
        {
            if(received[j] != '0' && received[j] != '1')
            {
                printf("Invalid binary data received.\n");
                close(sock);
                return 1;
            }
        }

        // XOR with the same PN code
        xorBits(received, pnCode, result);

        printf("Received = %s | After XOR = %s\n",
               received, result);

        if(strcmp(result, "1111") == 0)
            original[originalIndex++] = '1';
        else if(strcmp(result, "0000") == 0)
            original[originalIndex++] = '0';
        else
        {
            printf("Invalid spread group.\n");
            close(sock);
            return 1;
        }
    }

    original[originalIndex] = '\0';

    printf("\nRecovered Original Data: %s\n", original);

    close(sock);

    return 0;
}