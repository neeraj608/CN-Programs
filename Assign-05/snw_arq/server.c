
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>

typedef struct{
    int data;
    int prop_time;
} packet;

typedef struct{
    int ack_no;
    int rtt;
} ack;

int is_lost(int pkt, int* lost_nos, int num_lost)
{
    for(int i = 0; i < num_lost; i++)
    {
        if(lost_nos[i] == pkt)
        {
            return 1;
        }
    }

    return 0;
}

int main()
{
    int serversocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serversocket < 0)
    {
        perror("Error creating the socket");
        return 1;
    }

    struct sockaddr_in serveraddress = {0};
    struct sockaddr_in clientaddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8080);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    if(bind(serversocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Error binding");
        close(serversocket);
        return 1;
    }

    if(listen(serversocket, 5) < 0)
    {
        perror("Error listening");
        close(serversocket);
        return 1;
    }

    printf("Server listening on port 8080...\n");

    socklen_t clientlength = sizeof(clientaddress);

    int clientsocket = accept(serversocket, (struct sockaddr *)&clientaddress, &clientlength);

    if(clientsocket < 0)
    {
        perror("Error accepting connection");
        close(serversocket);
        return 1;
    }

    printf("Client connected successfully.\n");

    int tt, tp, no_of_packets, num_lost;

    if(recv(clientsocket, &tt, sizeof(tt), MSG_WAITALL) != sizeof(tt) ||
       recv(clientsocket, &tp, sizeof(tp), MSG_WAITALL) != sizeof(tp) ||
       recv(clientsocket, &no_of_packets, sizeof(no_of_packets), MSG_WAITALL) != sizeof(no_of_packets) ||
       recv(clientsocket, &num_lost, sizeof(num_lost), MSG_WAITALL) != sizeof(num_lost))
    {
        perror("Error receiving simulation data");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    if(no_of_packets < 1 || no_of_packets > 1000 ||
       num_lost < 0 || num_lost > 100 ||
       num_lost > no_of_packets || tt <= 0 || tp < 0)
    {
        printf("Invalid simulation parameters\n");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    int lost_nos[100] = {0};

    if(num_lost > 0)
    {
        if(recv(clientsocket, lost_nos, sizeof(int) * num_lost, MSG_WAITALL)
           != sizeof(int) * num_lost)
        {
            perror("Error receiving lost packet list");
            close(clientsocket);
            close(serversocket);
            return 1;
        }
    }

    int received_counts[1001] = {0};

    while(1)
    {
        packet p;

        int received_status = recv(clientsocket, &p, sizeof(p), MSG_WAITALL);

        if(received_status == 0)
        {
            break;
        }

        if(received_status != sizeof(p))
        {
            perror("Error receiving packet");
            break;
        }

        if(p.data < 1 || p.data > no_of_packets)
        {
            printf("Invalid packet received\n");
            continue;
        }

        received_counts[p.data]++;

        printf("--------------------------------------------------------------------\n");

        if(is_lost(p.data, lost_nos, num_lost) &&
           received_counts[p.data] == 1)
        {
            printf("Packet %d received at t = %d but dropped (ACK not sent)\n",
                   p.data, p.prop_time);

            continue;
        }

        printf("Packet %d received at t = %d, sending ACK\n",
               p.data, p.prop_time);

        ack a;

        a.ack_no = p.data;
        a.rtt = tt + 2 * tp;

        if(send(clientsocket, &a, sizeof(a), 0) < 0)
        {
            perror("Error sending ACK");
            break;
        }
    }

    close(clientsocket);
    close(serversocket);

    return 0;
}
