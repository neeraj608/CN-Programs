
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct
{
    int o[4];
    int prefix;
    int need;
} Req;

Req r;
char result[8192], t1[20], t2[20], t3[20], t4[20], sb_msk[20];

int maskOf(int p)
{
    if (p == 0)
        return 0;
    else
        return (-1 << (32 - p));
}

void toIP(unsigned int ip_bin, char *ip_formated)
{
    sprintf(ip_formated, "%u.%u.%u.%u",
            (ip_bin >> 24) & 255,
            (ip_bin >> 16) & 255,
            (ip_bin >> 8) & 255,
            ip_bin & 255);
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
    serveraddress.sin_port = htons(8088);
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

    /* RECEIVE STRUCT */
    int bytes = recv(clientsocket, &r, sizeof(r), MSG_WAITALL);

    if (bytes != sizeof(r))
    {
        printf("Failed to receive request.\n");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    printf("Received: %d.%d.%d.%d /%d, subnets=%d\n",
           r.o[0], r.o[1], r.o[2], r.o[3], r.prefix, r.need);

    /* BORROWED BITS */
    int borrowed = 0, count = 1;

    while (count < r.need && borrowed < 30)
    {
        count *= 2;
        borrowed++;
    }

    int newp = r.prefix + borrowed;

    /* CALCULATE NETWORK */
    unsigned int ip = ((unsigned int)r.o[0] << 24) |
                      ((unsigned int)r.o[1] << 16) |
                      ((unsigned int)r.o[2] << 8) |
                      (unsigned int)r.o[3];

    unsigned int network = ip & (unsigned int)maskOf(r.prefix);
    unsigned int subnet_mask = (unsigned int)maskOf(newp);

    unsigned int n = 1U << (32 - newp);

    toIP(subnet_mask, sb_msk);
    toIP(network, t1);

    /* HEADER */
    sprintf(result,
            "Base Network       : %s/%d\n"
            "Subnets Requested  : %d\n"
            "Subnets Created    : %d\n"
            "Borrowed Bits      : %d\n"
            "New Prefix         : /%d\n"
            "Subnet Mask        : %s\n"
            "Hosts per Subnet   : %u\n"
            "------------------------------\n",
            t1, r.prefix, r.need, count, borrowed, newp, sb_msk, n - 2);

    printf("%s", result);
    send(clientsocket, result, strlen(result), 0);

    /* GENERATE SUBNETS */
    for (int i = 0; i < count; i++)
    {
        unsigned int temp = network + i * n;

        toIP(temp, t1);
        toIP(temp + 1, t2);
        toIP(temp + n - 2, t3);
        toIP(temp + n - 1, t4);

        sprintf(result,
                "Subnet %d:\n"
                "  Network ID : %s/%d\n"
                "  First Host : %s\n"
                "  Last Host  : %s\n"
                "  Broadcast  : %s\n\n",
                i + 1, t1, newp, t2, t3, t4);

        printf("%s", result);
        send(clientsocket, result, strlen(result), 0);
    }

    close(clientsocket);
    close(serversocket);

    return 0;
}
