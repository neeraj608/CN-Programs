
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int o[4];
    char buf[1024];
    int n;

    printf("Enter IP address: ");
    scanf("%d.%d.%d.%d", &o[0], &o[1], &o[2], &o[3]);

    int clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientsocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8087);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientsocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Connection failed");
        close(clientsocket);
        return 1;
    }

    send(clientsocket, o, sizeof(o), 0);

    printf("\n");

    while ((n = recv(clientsocket, buf, sizeof(buf) - 1, 0)) > 0)
    {
        buf[n] = '\0';
        printf("%s", buf);
    }

    close(clientsocket);

    return 0;
}
