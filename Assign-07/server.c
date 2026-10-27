
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char result[500], t1[20], t2[20];

void toIP(unsigned int num, char *str)
{
    sprintf(str, "%u.%u.%u.%u",
            (num >> 24) & 255,
            (num >> 16) & 255,
            (num >> 8) & 255,
            num & 255);
}

int main()
{
    int serversocket, clientsocket;

    serversocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serversocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8087);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serversocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Bind failed");
        close(serversocket);
        return 1;
    }

    if (listen(serversocket, 3) < 0)
    {
        perror("Listen failed");
        close(serversocket);
        return 1;
    }

    printf("Waiting for client...\n");

    clientsocket = accept(serversocket, NULL, NULL);

    if (clientsocket < 0)
    {
        perror("Accept failed");
        close(serversocket);
        return 1;
    }

    /* RECEIVE 4 OCTETS */
    int o[4], i;

    int bytes = recv(clientsocket, o, sizeof(o), MSG_WAITALL);

    if (bytes != sizeof(o))
    {
        printf("Failed to receive IP address.\n");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    /* VALIDATE */
    for (i = 0; i < 4; i++)
    {
        if (o[i] < 0 || o[i] > 255)
        {
            strcpy(result, "Invalid IP Address\n");
            send(clientsocket, result, strlen(result), 0);
            close(clientsocket);
            close(serversocket);
            return 1;
        }
    }

    /* IP AS ONE NUMBER */
    unsigned int ip = ((unsigned int)o[0] << 24) |
                      ((unsigned int)o[1] << 16) |
                      ((unsigned int)o[2] << 8) |
                      (unsigned int)o[3];

    /* FIND CLASS */
    int ones = 0;

    while (ones < 4 && ((ip >> (31 - ones)) & 1))
    {
        ones++;
    }

    int len = sprintf(result, "Class      : %c\n", 'A' + ones);

    /* NETWORK AND HOST INFORMATION */
    if (ones <= 2)
    {
        int nid = 8 * (ones + 1);
        int hid = 32 - nid;

        int networks = 1 << (nid - (ones + 1));

        if (ones == 0)
        {
            networks -= 2;
        }

        unsigned int hosts = (1U << hid) - 2;

        unsigned int net = ip & (0xFFFFFFFFU << hid);

        toIP(net + 1, t1);
        toIP(net + (1U << hid) - 2, t2);

        sprintf(result + len,
                "NID Bits   : %d\n"
                "HID Bits   : %d\n"
                "Networks   : %d\n"
                "Hosts      : %u\n"
                "LBA        : %s\n"
                "DBA        : %s\n",
                nid, hid, networks, hosts, t1, t2);
    }
    else if (ones == 3)
    {
        sprintf(result + len, "Used for Multicasting\n");
    }
    else
    {
        sprintf(result + len, "Used for Experimental/Reserved purpose\n");
    }

    printf("%s", result);

    send(clientsocket, result, strlen(result), 0);

    close(clientsocket);
    close(serversocket);

    return 0;
}
