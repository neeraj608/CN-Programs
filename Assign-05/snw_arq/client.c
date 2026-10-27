
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

    int tt, tp, no_of_packets, num_lost;

    printf("Enter Tt : ");
    scanf("%d", &tt);

    printf("Enter Tp : ");
    scanf("%d", &tp);

    printf("Enter no of packets : ");
    scanf("%d", &no_of_packets);

    printf("Enter no of packets to lost : ");
    scanf("%d", &num_lost);

    if(no_of_packets < 1 || no_of_packets > 1000 ||
       num_lost < 0 || num_lost > 100 ||
       num_lost > no_of_packets || tt <= 0 || tp < 0)
    {
        printf("Invalid input\n");
        close(clientsocket);
        return 1;
    }

    int lost_nos[100] = {0};

    if(num_lost > 0)
    {
        printf("Enter the lost nos (space separated) : ");

        for(int i = 0; i < num_lost; i++)
        {
            scanf("%d", &lost_nos[i]);

            if(lost_nos[i] < 1 || lost_nos[i] > no_of_packets)
            {
                printf("Invalid packet number\n");
                close(clientsocket);
                return 1;
            }
        }
    }

    if(send(clientsocket, &tt, sizeof(tt), 0) < 0 ||
       send(clientsocket, &tp, sizeof(tp), 0) < 0 ||
       send(clientsocket, &no_of_packets, sizeof(no_of_packets), 0) < 0 ||
       send(clientsocket, &num_lost, sizeof(num_lost), 0) < 0)
    {
        perror("Error sending simulation data");
        close(clientsocket);
        return 1;
    }

    if(num_lost > 0)
    {
        if(send(clientsocket, lost_nos, sizeof(int) * num_lost, 0) < 0)
        {
            perror("Error sending lost packet list");
            close(clientsocket);
            return 1;
        }
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

        if(send(clientsocket, &p, sizeof(p), 0) < 0)
        {
            perror("Error sending packet");
            close(clientsocket);
            return 1;
        }

        if(is_lost(curr_packet, lost_nos, num_lost))
        {
            curr_time += to_calculated;

            printf("Timeout occurred! Retransmitting packet...\n");

            curr_time += tt;

            printf("Packet %d re-sent at t = %d\n", curr_packet, curr_time);

            p.prop_time = curr_time + tp;

            if(send(clientsocket, &p, sizeof(p), 0) < 0)
            {
                perror("Error retransmitting packet");
                close(clientsocket);
                return 1;
            }
        }

        ack a;

        int received_status = recv(clientsocket, &a, sizeof(a), MSG_WAITALL);

        if(received_status != sizeof(a))
        {
            perror("Error receiving ACK");
            close(clientsocket);
            return 1;
        }

        printf("ACK %d received at t = %d\n",
               a.ack_no, curr_time + a.rtt);

        curr_time += a.rtt;

        curr_packet++;
    }

    printf("--------------------------------------------------------------------\n");
    printf("Transmission Completed\n");

    int n1 = no_of_packets - num_lost;
    int n2 = num_lost;

    int formula_time = n1 * (2 * (tt + tp))
                     + n2 * (4 * rtt_calculated + 2 * tt);

    printf("Total simulated time = %d\n", curr_time);

    printf("Verified formula time n1(2(Tt + Tp)) + n2(4RTT + 2Tt) = %d\n",
           formula_time);

    close(clientsocket);

    return 0;
}
