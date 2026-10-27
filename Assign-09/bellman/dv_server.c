
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
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

void print_path(int parent[], int vertex)
{
    int path[MAX_VERTICES];
    int length = 0;
    int current = vertex;

    while(current != -1 && length < MAX_VERTICES)
    {
        path[length++] = current;
        current = parent[current];
    }

    if(current != -1)
    {
        printf("cycle detected in predecessor chain");
        return;
    }

    for(int i = length - 1; i >= 0; i--)
    {
        printf("%d", path[i]);

        if(i > 0)
        {
            printf(" -> ");
        }
    }
}

void handle_client(int clientsocket)
{
    struct DV_Request req;
    char ack = 'K';

    long long distance[MAX_VERTICES];
    int parent[MAX_VERTICES];

    int received_status = recv(clientsocket, &req, sizeof(req), MSG_WAITALL);

    if(received_status != sizeof(req))
    {
        close(clientsocket);
        return;
    }

    if(req.vertices < 1 || req.vertices > MAX_VERTICES ||
       req.edge_count < 0 || req.edge_count > MAX_EDGES ||
       req.source_router < 0 || req.source_router >= req.vertices)
    {
        printf("Invalid request received.\n");
        close(clientsocket);
        return;
    }

    printf("\n--- Processing Distance Vector Routing ---\n");
    printf("Total Routers: %d\n", req.vertices);
    printf("Total Links: %d\n", req.edge_count);
    printf("Source Router: %d\n\n", req.source_router);

    for(int i = 0; i < req.vertices; i++)
    {
        distance[i] = LLONG_MAX;
        parent[i] = -1;
    }

    distance[req.source_router] = 0;

    for(int round = 1; round < req.vertices; round++)
    {
        int changed = 0;

        for(int i = 0; i < req.edge_count; i++)
        {
            struct Edge edge = req.edges[i];

            if(edge.source < 0 || edge.source >= req.vertices ||
               edge.destination < 0 || edge.destination >= req.vertices)
            {
                continue;
            }

            if(distance[edge.source] != LLONG_MAX &&
               distance[edge.source] + edge.cost < distance[edge.destination])
            {
                distance[edge.destination] =
                    distance[edge.source] + edge.cost;

                parent[edge.destination] = edge.source;
                changed = 1;
            }
        }

        printf("Distance-vector update round %d: ", round);

        if(changed == 1)
        {
            printf("table changed\n");
        }
        else
        {
            printf("converged\n");
        }

        if(changed == 0)
        {
            break;
        }
    }

    int has_cycle = 0;

    for(int i = 0; i < req.edge_count; i++)
    {
        struct Edge edge = req.edges[i];

        if(edge.source < 0 || edge.source >= req.vertices ||
           edge.destination < 0 || edge.destination >= req.vertices)
        {
            continue;
        }

        if(distance[edge.source] != LLONG_MAX &&
           distance[edge.source] + edge.cost < distance[edge.destination])
        {
            printf("\nNegative-weight cycle detected reachable from router %d.\n",
                   req.source_router);
            printf("Shortest paths are undefined.\n");

            has_cycle = 1;
            break;
        }
    }

    if(has_cycle == 0)
    {
        printf("\nDistance-vector routing table (Bellman-Ford)\n");
        printf("Destination\tCost\tPath\n");

        for(int i = 0; i < req.vertices; i++)
        {
            printf("%d\t\t", i);

            if(distance[i] == LLONG_MAX)
            {
                printf("INF\tUnreachable\n");
            }
            else
            {
                printf("%lld\t", distance[i]);
                print_path(parent, i);
                printf("\n");
            }
        }
    }

    printf("--- Finished Processing ---\n\n");

    if(send(clientsocket, &ack, sizeof(ack), 0) < 0)
    {
        perror("Acknowledgment failed");
    }

    close(clientsocket);
}

int main(void)
{
    int serversocket, clientsocket;

    struct sockaddr_in serveraddress = {0};
    struct sockaddr_in clientaddress = {0};

    serversocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serversocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(PORT);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    if(bind(serversocket, (struct sockaddr *)&serveraddress,
            sizeof(serveraddress)) < 0)
    {
        perror("Bind failed");
        close(serversocket);
        return 1;
    }

    if(listen(serversocket, 5) < 0)
    {
        perror("Listen failed");
        close(serversocket);
        return 1;
    }

    printf("Distance Vector Server listening on port %d...\n", PORT);

    while(1)
    {
        socklen_t clientlength = sizeof(clientaddress);

        clientsocket = accept(serversocket,
                              (struct sockaddr *)&clientaddress,
                              &clientlength);

        if(clientsocket < 0)
        {
            perror("Accept failed");
            continue;
        }

        handle_client(clientsocket);
    }

    close(serversocket);

    return 0;
}
