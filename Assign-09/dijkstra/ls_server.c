
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_VERTICES 100

struct LS_Request {
    int vertices;
    int graph[MAX_VERTICES][MAX_VERTICES];
    int source_router;
};

void print_path(int parent[], int vertex)
{
    if(parent[vertex] == -1)
    {
        printf("%d", vertex);
        return;
    }

    print_path(parent, parent[vertex]);
    printf(" -> %d", vertex);
}

void handle_client(int clientsocket)
{
    struct LS_Request req;
    char ack = 'K';
    size_t received = 0;

    while(received < sizeof(req))
    {
        ssize_t n = recv(clientsocket, (char *)&req + received,
                         sizeof(req) - received, 0);

        if(n <= 0)
        {
            printf("Client disconnected or receive failed.\n");
            close(clientsocket);
            return;
        }

        received += (size_t)n;
    }

    if(req.vertices < 1 || req.vertices > MAX_VERTICES ||
       req.source_router < 0 ||
       req.source_router >= req.vertices)
    {
        printf("Invalid request received.\n");
        close(clientsocket);
        return;
    }

    printf("\n--- Link State Routing ---\n");
    printf("Number of routers: %d\n", req.vertices);
    printf("Source router: %d\n\n", req.source_router);

    int distance[MAX_VERTICES];
    int parent[MAX_VERTICES];
    int visited[MAX_VERTICES] = {0};

    for(int i = 0; i < req.vertices; i++)
    {
        distance[i] = INT_MAX;
        parent[i] = -1;
    }

    distance[req.source_router] = 0;

    for(int step = 0; step < req.vertices; step++)
    {
        int current = -1;

        for(int i = 0; i < req.vertices; i++)
        {
            if(!visited[i] && distance[i] != INT_MAX &&
               (current == -1 || distance[i] < distance[current]))
            {
                current = i;
            }
        }

        if(current == -1)
        {
            break;
        }

        visited[current] = 1;

        for(int neighbor = 0; neighbor < req.vertices; neighbor++)
        {
            int cost = req.graph[current][neighbor];

            if(!visited[neighbor] && cost >= 0 &&
               distance[current] <= INT_MAX - cost &&
               distance[current] + cost < distance[neighbor])
            {
                distance[neighbor] = distance[current] + cost;
                parent[neighbor] = current;
            }
        }
    }

    printf("Routing Table (Dijkstra / OSPF Model)\n");
    printf("Destination\tCost\tPath\n");

    for(int i = 0; i < req.vertices; i++)
    {
        printf("%d\t\t", i);

        if(distance[i] == INT_MAX)
        {
            printf("INF\tUnreachable\n");
        }
        else
        {
            printf("%d\t", distance[i]);
            print_path(parent, i);
            printf("\n");
        }
    }

    printf("\n--- Processing Complete ---\n\n");

    if(send(clientsocket, &ack, sizeof(ack), 0) < 0)
    {
        perror("Acknowledgment failed");
    }

    close(clientsocket);
}

int main()
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

    printf("Link State Server listening on port %d...\n", PORT);

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
