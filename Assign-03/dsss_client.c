
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
    char bits[101];
    char combined[401] = "";

    printf("Enter binary data: ");
    scanf("%100s", bits);

    // Validate input
    for(int i = 0; bits[i] != '\0'; i++)
    {
        if(bits[i] != '0' && bits[i] != '1')
        {
            printf("Invalid input. Enter only 0 and 1.\n");
            return 1;
        }
    }

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    printf("\nOriginal Data: %s\n", bits);
    printf("PN Code: %s\n\n", pnCode);

    // Spread all bits and combine
    for(int i = 0; bits[i] != '\0'; i++)
    {
        char repeatedBit[5];
        char spreadData[5];

        // Convert one bit into 4 bits
        if(bits[i] == '1')
            strcpy(repeatedBit, "1111");
        else
            strcpy(repeatedBit, "0000");

        // XOR with PN sequence
        xorBits(repeatedBit, pnCode, spreadData);

        // Combine all spread bits
        strcat(combined, spreadData);

        printf("Bit = %c | XOR Result = %s\n",
               bits[i], spreadData);
    }

    printf("\nCombined Spread Data: %s\n", combined);

    // Send the entire combined data in one packet
    Packet p = {0};

    strcpy(p.data, combined);

    int bytes = sendto(
        sock,
        &p,
        sizeof(p),
        0,
        (struct sockaddr *)&server,
        sizeof(server)
    );

    if(bytes < 0)
    {
        perror("sendto failed");
        close(sock);
        return 1;
    }

    printf("\nComplete spread data sent in one UDP packet.\n");

    close(sock);

    return 0;
}
