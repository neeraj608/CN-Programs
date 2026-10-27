
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

int main()
{
    Req r;
    char result[8192];

    printf("Enter network address: ");
    scanf("%d.%d.%d.%d", &r.o[0], &r.o[1], &r.o[2], &r.o[3]);

    printf("Enter prefix (0-30): ");
    scanf("%d", &r.prefix);

    printf("Enter number of subnets: ");
    scanf("%d", &r.need);

    int clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientsocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8088);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientsocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Connection failed");
        close(clientsocket);
        return 1;
    }

    send(clientsocket, &r, sizeof(r), 0);

    char buf[1024];
    int n;

    while ((n = recv(clientsocket, buf, sizeof(buf) - 1, 0)) > 0)
    {
        buf[n] = '\0';
        printf("%s", buf);
    }

    close(clientsocket);

    return 0;
}
