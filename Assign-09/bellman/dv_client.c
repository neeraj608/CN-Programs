
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_VERTICES 100
#define MAX_EDGES 500

struct Edge
{
    int source;
    int destination;
    int cost;
};

struct DV_Request
{
    int vertices;
    int edge_count;
    int source_router;
    struct Edge edges[MAX_EDGES];
};

int main(void)
{
    int clientsocket;
    struct sockaddr_in serveraddress = {0};
    struct DV_Request req = {0};
    char ack;

    clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientsocket < 0)
    {
        perror("Socket creation error");
        return 1;
    }

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(PORT);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Enter number of routers (1-%d): ", MAX_VERTICES);

    if(scanf("%d", &req.vertices) != 1 ||
       req.vertices < 1 || req.vertices > MAX_VERTICES)
    {
        printf("Invalid number of routers.\n");
        close(clientsocket);
        return 1;
    }

    printf("Enter number of directed links (0-%d): ", MAX_EDGES);

    if(scanf("%d", &req.edge_count) != 1 ||
       req.edge_count < 0 || req.edge_count > MAX_EDGES)
    {
        printf("Invalid number of links.\n");
        close(clientsocket);
        return 1;
    }

    printf("Enter each link as: source destination cost\n");

    for(int i = 0; i < req.edge_count; i++)
    {
        if(scanf("%d %d %d",
                 &req.edges[i].source,
                 &req.edges[i].destination,
                 &req.edges[i].cost) != 3 ||
           req.edges[i].source < 0 ||
           req.edges[i].source >= req.vertices ||
           req.edges[i].destination < 0 ||
           req.edges[i].destination >= req.vertices)
        {
            printf("Invalid link. Router IDs must be between 0 and %d.\n",
                   req.vertices - 1);
            close(clientsocket);
            return 1;
        }
    }

    printf("Enter source router (0-%d): ", req.vertices - 1);

    if(scanf("%d", &req.source_router) != 1 ||
       req.source_router < 0 ||
       req.source_router >= req.vertices)
    {
        printf("Invalid source router.\n");
        close(clientsocket);
        return 1;
    }

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
            perror("Sending failed");
            close(clientsocket);
            return 1;
        }

        sent += (size_t)n;
    }

    int received_status = recv(clientsocket, &ack, sizeof(ack), MSG_WAITALL);

    if(received_status == sizeof(ack) && ack == 'K')
    {
        printf("\nNetwork topology sent successfully!\n");
        printf("Check the server console for routing table updates.\n");
    }
    else
    {
        printf("Failed to receive acknowledgment.\n");
    }

    close(clientsocket);

    return 0;
}
