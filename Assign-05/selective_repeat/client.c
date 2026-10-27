#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void print_window(int base, int window_size, int no_of_packets)
{
    printf("Window: [ ");

    for (int i = base;
         i < base + window_size && i <= no_of_packets;
         i++)
    {
        printf("%d ", i);
    }

    printf("]\n");
}

int main()
{
    int window_size, no_of_packets, nth_lost;
    int base = 1;
    int packet_no, ack_no;
    int packets_sent;

    printf("Enter window size: ");
    scanf("%d", &window_size);

    printf("Enter no of packets: ");
    scanf("%d", &no_of_packets);

    printf("Every nth packet lost (0 = none): ");
    scanf("%d", &nth_lost);

    if (window_size <= 0 || no_of_packets <= 0 || nth_lost < 0)
    {
        printf("Invalid input\n");
        return 1;
    }

    int *acked = calloc(no_of_packets + 1, sizeof(int));

    if (acked == NULL)
    {
        perror("calloc");
        return 1;
    }

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (client_socket < 0)
    {
        perror("socket");
        free(acked);
        return 1;
    }

    if (connect(client_socket,
                (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0)
    {
        perror("connect");
        close(client_socket);
        free(acked);
        return 1;
    }

    send(client_socket, &window_size, sizeof(int), 0);
    send(client_socket, &no_of_packets, sizeof(int), 0);
    send(client_socket, &nth_lost, sizeof(int), 0);

    while (base <= no_of_packets)
    {
        printf("\n");
        print_window(base, window_size, no_of_packets);

        packets_sent = 0;

        // Send only unacknowledged packets in the window
        for (int i = base;
             i < base + window_size && i <= no_of_packets;
             i++)
        {
            if (acked[i] == 0)
            {
                packet_no = i;

                printf("Sending packet %d\n", packet_no);

                if (send(client_socket, &packet_no,
                         sizeof(int), 0) != sizeof(int))
                {
                    perror("send");
                    close(client_socket);
                    free(acked);
                    return 1;
                }

                packets_sent++;
            }
        }

        // Receive ACKs for packets sent in this round
        for (int i = 0; i < packets_sent; i++)
        {
            if (recv(client_socket, &ack_no,
                     sizeof(int), MSG_WAITALL) != sizeof(int))
            {
                printf("Connection closed or receive failed\n");
                close(client_socket);
                free(acked);
                return 1;
            }

            if (ack_no != -1 &&
                ack_no >= 1 && ack_no <= no_of_packets)
            {
                acked[ack_no] = 1;

                printf("ACK %d received\n", ack_no);
            }
            else
            {
                printf("Packet not acknowledged, will retransmit\n");
            }
        }

        // Slide base over consecutively acknowledged packets
        while (base <= no_of_packets && acked[base] == 1)
        {
            base++;
        }

        if (base <= no_of_packets)
        {
            printf("Next base = %d\n", base);
        }
    }

    printf("\nAll packets sent and acknowledged!\n");

    close(client_socket);
    free(acked);

    return 0;
}