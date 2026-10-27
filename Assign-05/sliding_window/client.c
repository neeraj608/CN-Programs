
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket,
                (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        close(client_socket);
        return 1;
    }

    int window_size, no_of_packets;

    printf("Enter window size: ");
    scanf("%d", &window_size);

    printf("Enter no of packets: ");
    scanf("%d", &no_of_packets);

    if (window_size <= 0 || no_of_packets <= 0)
    {
        printf("Invalid input\n");
        close(client_socket);
        return 1;
    }

    send(client_socket, &window_size, sizeof(int), 0);
    send(client_socket, &no_of_packets, sizeof(int), 0);

    int base = 1;
    int next_to_send = 1;

    while (base <= no_of_packets)
    {
        printf("\nWindow: [ ");

        for (int i = base;
             i < base + window_size && i <= no_of_packets;
             i++)
        {
            printf("%d ", i);
        }

        printf("]\n");

        while (next_to_send < base + window_size &&
               next_to_send <= no_of_packets)
        {
            printf("Sending packet %d\n", next_to_send);

            if (send(client_socket, &next_to_send,
                     sizeof(int), 0) != sizeof(int))
            {
                perror("Send failed");
                close(client_socket);
                return 1;
            }

            next_to_send++;
        }

        for (int i = base;
             i < next_to_send;
             i++)
        {
            int ack_no;

            if (recv(client_socket, &ack_no,
                     sizeof(int), MSG_WAITALL) != sizeof(int))
            {
                printf("ACK receive failed\n");
                close(client_socket);
                return 1;
            }

            printf("ACK %d received\n", ack_no);
        }

        base = next_to_send;
    }

    printf("\nTransmission completed\n");

    close(client_socket);
    return 0;
}
