
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
    int clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientsocket < 0)
    {
        perror("Error creating the socket");
        return 1;
    }

    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8080);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(clientsocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Error connecting the socket");
        close(clientsocket);
        return 1;
    }

    int tt, tp, no_of_packets;

    printf("Enter Tt : ");
    scanf("%d", &tt);

    printf("Enter Tp : ");
    scanf("%d", &tp);

    printf("Enter no of packets : ");
    scanf("%d", &no_of_packets);

    if(send(clientsocket, &tt, sizeof(tt), 0) < 0)
    {
        perror("Error sending Tt");
        close(clientsocket);
        return 1;
    }

    if(send(clientsocket, &tp, sizeof(tp), 0) < 0)
    {
        perror("Error sending Tp");
        close(clientsocket);
        return 1;
    }

    if(send(clientsocket, &no_of_packets, sizeof(no_of_packets), 0) < 0)
    {
        perror("Error sending number of packets");
        close(clientsocket);
        return 1;
    }

    int rtt_calculated = 2 * (tt + tp);
    int to_calculated = 3 * rtt_calculated;

    printf("Transmission time = %d\n", tt);
    printf("Propagation time = %d\n", tp);
    printf("Round trip time 2(Tt + Tp) = %d\n", rtt_calculated);
    printf("Timeout time 3(RTT) = %d\n", to_calculated);

    int curr_time = 0;
    int curr_packet = 1;

    while(curr_packet <= no_of_packets)
    {
        curr_time += tt;

        printf("--------------------------------------------------------------------\n");
        printf("Packet %d sent at t = %d\n", curr_packet, curr_time);

        packet p = {curr_packet, curr_time + tp};

        int send_status = send(clientsocket, &p, sizeof(p), 0);

        if(send_status < 0)
        {
            perror("Error sending the packet");
            close(clientsocket);
            return 1;
        }

        ack a;

        int received_status = recv(clientsocket, &a, sizeof(a), MSG_WAITALL);

        if(received_status != sizeof(a))
        {
            perror("Error receiving the ACK");
            close(clientsocket);
            return 1;
        }

        printf("ACK %d received at t = %d\n", a.ack_no, a.rtt);

        curr_time += a.rtt;

        curr_packet++;
    }

    printf("Transmission Completed\n");
    printf("Total time taken n(2(Tt + Tp)) = %d\n",
           no_of_packets * rtt_calculated);

    close(clientsocket);

    return 0;
}
