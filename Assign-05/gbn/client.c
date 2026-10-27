
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void print_window(int base, int window_size, int no_of_packets)
{
    printf("Window: [ ");

    for(int i = base; i < base + window_size && i <= no_of_packets; i++)
    {
        printf("%d ", i);
    }

    printf("]\n");
}

int main()
{
    int window_size, no_of_packets, nth_lost;
    int base = 1;
    int next_to_send = 1;
    int packets_sent;
    int check_packet;
    int ack_no;

    printf("Enter window size: ");
    scanf("%d", &window_size);

    printf("Enter no of packets: ");
    scanf("%d", &no_of_packets);

    printf("Every nth packet lost (0 = none): ");
    scanf("%d", &nth_lost);

    if(window_size <= 0 || no_of_packets <= 0 || nth_lost < 0)
    {
        printf("Invalid input\n");
        return 1;
    }

    int clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientsocket < 0)
    {
        perror("Error creating socket");
        return 1;
    }

    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8080);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(clientsocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Error connecting");
        close(clientsocket);
        return 1;
    }

    if(send(clientsocket, &window_size, sizeof(window_size), 0) < 0 ||
       send(clientsocket, &no_of_packets, sizeof(no_of_packets), 0) < 0 ||
       send(clientsocket, &nth_lost, sizeof(nth_lost), 0) < 0)
    {
        perror("Error sending parameters");
        close(clientsocket);
        return 1;
    }

    while(base <= no_of_packets)
    {
        printf("\n");
        print_window(base, window_size, no_of_packets);

        packets_sent = 0;

        while(next_to_send < base + window_size && next_to_send <= no_of_packets)
        {
            printf("Sending packet %d\n", next_to_send);

            if(send(clientsocket, &next_to_send, sizeof(int), 0) < 0)
            {
                perror("Error sending packet");
                close(clientsocket);
                return 1;
            }

            next_to_send++;
            packets_sent++;
        }

        check_packet = base;

        for(int i = 0; i < packets_sent; i++)
        {
            int received_status = recv(clientsocket, &ack_no, sizeof(ack_no), MSG_WAITALL);

            if(received_status != sizeof(ack_no))
            {
                perror("Error receiving ACK");
                close(clientsocket);
                return 1;
            }

            if(ack_no != -1)
            {
                base = ack_no + 1;

                printf("ACK %d received, new base = %d\n", ack_no, base);

                if(base <= no_of_packets)
                {
                    print_window(base, window_size, no_of_packets);
                }

                if(next_to_send < base + window_size && next_to_send <= no_of_packets)
                {
                    printf("Sending packet %d\n", next_to_send);

                    if(send(clientsocket, &next_to_send, sizeof(int), 0) < 0)
                    {
                        perror("Error sending packet");
                        close(clientsocket);
                        return 1;
                    }

                    next_to_send++;
                    packets_sent++;
                }
            }
            else
            {
                printf("Packet %d not ACKed (lost or out of order)\n", check_packet);
            }

            check_packet++;
        }

        if(base <= no_of_packets)
        {
            printf("Go back to packet (new base): %d\n", base);
            next_to_send = base;
        }
    }

    printf("\nAll packets sent and acknowledged!\n");

    close(clientsocket);

    return 0;
}
