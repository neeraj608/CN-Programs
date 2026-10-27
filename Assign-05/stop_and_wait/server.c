
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
        perror("Error binding the socket");
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

    int tt, tp, no_of_packets;

    if(recv(clientsocket, &tt, sizeof(tt), MSG_WAITALL) != sizeof(tt))
    {
        perror("Error receiving Tt");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    if(recv(clientsocket, &tp, sizeof(tp), MSG_WAITALL) != sizeof(tp))
    {
        perror("Error receiving Tp");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    if(recv(clientsocket, &no_of_packets, sizeof(no_of_packets), MSG_WAITALL) != sizeof(no_of_packets))
    {
        perror("Error receiving number of packets");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    printf("Simulation Parameters Received -> Tt: %d, Tp: %d, Packets: %d\n",
           tt, tp, no_of_packets);

    for(int i = 1; i <= no_of_packets; i++)
    {
        packet p;

        int received_status = recv(clientsocket, &p, sizeof(p), MSG_WAITALL);

        if(received_status != sizeof(p))
        {
            perror("Error receiving packet");
            close(clientsocket);
            close(serversocket);
            return 1;
        }

        printf("--------------------------------------------------------------------\n");
        printf("Packet %d received at server time t = %d\n", p.data, p.prop_time);

        ack a;

        a.ack_no = p.data;
        a.rtt = tt + 2 * tp;

        printf("Sending ACK %d for Packet %d\n", a.ack_no, p.data);

        int send_status = send(clientsocket, &a, sizeof(a), 0);

        if(send_status < 0)
        {
            perror("Error sending ACK");
            close(clientsocket);
            close(serversocket);
            return 1;
        }
    }

    printf("--------------------------------------------------------------------\n");
    printf("All packets received. Transmission Completed.\n");

    close(clientsocket);
    close(serversocket);

    return 0;
}
