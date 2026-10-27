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
    int count;
    int hosts[20];     // hosts needed for each subnet (already in order)
} Req;


int maskOf(int p)
{
    return (p == 0) ? 0 : (-1 << (32 - p));
}

void toIP(int ip_bin, char *ip_formated)
{
    sprintf(ip_formated, "%d.%d.%d.%d", (ip_bin >> 24) & 255, (ip_bin >> 16) & 255, (ip_bin >> 8) & 255, ip_bin & 255);
}

Req r;
char result[8192], t1[20], t2[20], t3[20], t4[20], sb_msk[20];
int main()
{
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(8089);
    a.sin_addr.s_addr = INADDR_ANY;

    bind(srv, (struct sockaddr *)&a, sizeof(a));
    listen(srv, 3);
    printf("Waiting for client...\n");

    int c = accept(srv, NULL, NULL);

    // 1. receive the struct
    recv(c, &r, sizeof(r), MSG_WAITALL);
    printf("Received: %d.%d.%d.%d /%d, subnets=%d\n",
           r.o[0], r.o[1], r.o[2], r.o[3], r.prefix, r.count);

    // 2. validate
    // int ok = 1;
    // for (int i = 0; i < 4; i++)
    //     if (r.o[i] < 0 || r.o[i] > 255)
    //         ok = 0;
    // if (r.prefix < 8 || r.prefix > 30 || r.count < 1 || r.count > 20)
    //     ok = 0;
    // for (int i = 0; ok && i < r.count; i++)
    //     if (r.hosts[i] < 1 || r.hosts[i] > 16777214)
    //         ok = 0;

    // if (!ok)
    // {
    //     strcpy(result, "Error: Invalid request\n");
    //     send(c, result, strlen(result), 0);
    //     close(c);
    //     close(srv);
    //     return 1;
    // }

       // 3. network and base size
    int ip = (r.o[0] << 24) | (r.o[1] << 16) | (r.o[2] << 8) | r.o[3];
    int network = ip & maskOf(r.prefix);
    int base_size = 1 << (32 - r.prefix);

    // header: build and send
    toIP(network, t1);
    sprintf(result,
            "Base Network       : %s/%d\n"
            "Subnets Requested  : %d\n"
            "------------------------------\n",
            t1, r.prefix, r.count);
    printf("%s", result);
    send(c, result, strlen(result), 0);

    // 4. one loop: calculate and send each subnet
    int cur = network, used = 0;
    for (int i = 0; i < r.count; i++)
    {
        // block size = smallest power of 2 >= hosts + 2
        int size = 1, bits = 0;
        while (size < r.hosts[i] + 2 && bits < 30)
        {
            size *= 2;
            bits++;
        }
        int p = 32 - bits;

        // does it still fit in the base network?
        used += size;
        if (used > base_size)
        {
            sprintf(result, "Error: Subnet %d does not fit in /%d network\n", i + 1, r.prefix);
            send(c, result, strlen(result), 0);
            break;
        }

        toIP(maskOf(p), sb_msk);
        toIP(cur, t1);
        toIP(cur + 1, t2);
        toIP(cur + size - 2, t3);
        toIP(cur + size - 1, t4);

        sprintf(result,
                "Subnet %d:\n"
                "  Required Hosts  : %d\n"
                "  Allocated Hosts : %d\n"
                "  Network ID      : %s/%d\n"
                "  Subnet Mask     : %s\n"
                "  First Host      : %s\n"
                "  Last Host       : %s\n"
                "  Broadcast       : %s\n\n",
                i + 1, r.hosts[i], size - 2,
                t1, p, sb_msk, t2, t3, t4);

        printf("%s", result);
        send(c, result, strlen(result), 0);

        cur += size;            // next subnet starts right after this one
    }

    close(c);
    close(srv);
    return 0;
}