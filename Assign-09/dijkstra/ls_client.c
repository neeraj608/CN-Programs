
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_VERTICES 100

struct LS_Request {
    int vertices;
    int graph[MAX_VERTICES][MAX_VERTICES];
    int source_router;
};

int main()
{
    int clientsocket;
    struct sockaddr_in serveraddress = {0};
    struct LS_Request req;
    char ack;

    printf("Enter number of routers: ");
    scanf("%d", &req.vertices);

    if(req.vertices < 1 || req.vertices > MAX_VERTICES)
    {
        printf("Invalid number of routers.\n");
        return 1;
    }

    for(int i = 0; i < req.vertices; i++)
    {
        for(int j = 0; j < req.vertices; j++)
        {
            if(i == j)
            {
                req.graph[i][j] = 0;
            }
            else
            {
                req.graph[i][j] = -1;
            }
        }
    }

    int links;

    printf("Enter number of links: ");
    scanf("%d", &links);

    if(links < 0 || links > req.vertices * (req.vertices - 1) / 2)
    {
        printf("Invalid number of links.\n");
        return 1;
    }

    printf("Enter source destination cost for each link:\n");

    for(int i = 0; i < links; i++)
    {
        int src, dest, cost;

        printf("Link %d: ", i + 1);

        if(scanf("%d %d %d", &src, &dest, &cost) != 3 ||
           src < 0 || src >= req.vertices ||
           dest < 0 || dest >= req.vertices ||
           src == dest || cost < 0)
        {
            printf("Invalid link details.\n");
            return 1;
        }

        req.graph[src][dest] = cost;
        req.graph[dest][src] = cost;
    }

    printf("Enter source router: ");
    scanf("%d", &req.source_router);

    if(req.source_router < 0 || req.source_router >= req.vertices)
    {
        printf("Invalid source router.\n");
        return 1;
    }

    clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientsocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(PORT);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(clientsocket, (struct sockaddr *)&serveraddress,
               sizeof(serveraddress)) < 0)
    {
        perror("Connection failed");
        close(clientsocket);
        return 1;
    }

    size_t sent = 0;

    while(sent < sizeof(req))
    {
        ssize_t n = send(clientsocket, (char *)&req + sent,
                         sizeof(req) - sent, 0);

        if(n <= 0)
        {
            perror("Send failed");
            close(clientsocket);
            return 1;
        }

        sent += (size_t)n;
    }

    if(recv(clientsocket, &ack, sizeof(ack), MSG_WAITALL) == sizeof(ack)
       && ack == 'K')
    {
        printf("\nNetwork topology sent successfully!\n");
        printf("Check server terminal for routing table.\n");
    }
    else
    {
        printf("Acknowledgment not received.\n");
    }

    close(clientsocket);

    return 0;
}
