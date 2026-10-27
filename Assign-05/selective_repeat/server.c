#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int window_size, no_of_packets, nth_lost;
    int packet_no, ack_no;
    int received_count = 0;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        perror("socket");
        return 1;
    }

    int opt = 1;

    setsockopt(server_socket, SOL_SOCKET,
               SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket,
             (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0)
    {
        perror("bind");
        close(server_socket);
        return 1;
    }

    listen(server_socket, 3);

    printf("Waiting for client...\n");

    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket < 0)
    {
        perror("accept");
        close(server_socket);
        return 1;
    }

    // Receive parameters
    if (recv(client_socket, &window_size,
              sizeof(int), MSG_WAITALL) != sizeof(int) ||
        recv(client_socket, &no_of_packets,
              sizeof(int), MSG_WAITALL) != sizeof(int) ||
        recv(client_socket, &nth_lost,
              sizeof(int), MSG_WAITALL) != sizeof(int))
    {
        printf("Failed to receive parameters\n");
        close(client_socket);
        close(server_socket);
        return 1;
    }

    if (window_size <= 0 || no_of_packets <= 0 || nth_lost < 0)
    {
        printf("Invalid parameters\n");
        close(client_socket);
        close(server_socket);
        return 1;
    }

    int *received = calloc(no_of_packets + 1, sizeof(int));
    int *lost_once = calloc(no_of_packets + 1, sizeof(int));

    if (received == NULL || lost_once == NULL)
    {
        perror("calloc");
        free(received);
        free(lost_once);
        close(client_socket);
        close(server_socket);
        return 1;
    }

    printf("Window = %d, Packets = %d, Lost every = %d\n\n",
           window_size, no_of_packets, nth_lost);

    while (received_count < no_of_packets)
    {
        if (recv(client_socket, &packet_no,
                 sizeof(int), MSG_WAITALL) != sizeof(int))
        {
            break;
        }

        ack_no = -1;

        if (packet_no >= 1 && packet_no <= no_of_packets)
        {
            // Simulate loss only once for each selected packet
            if (nth_lost != 0 &&
                packet_no % nth_lost == 0 &&
                lost_once[packet_no] == 0)
            {
                printf("Packet %d simulated as lost\n", packet_no);

                lost_once[packet_no] = 1;
            }
            else
            {
                if (received[packet_no] == 0)
                {
                    received[packet_no] = 1;
                    received_count++;

                    printf("Packet %d accepted\n", packet_no);
                }
                else
                {
                    printf("Duplicate packet %d received\n", packet_no);
                }

                // ACK each packet independently
                ack_no = packet_no;

                printf("Sending ACK %d\n", ack_no);
            }
        }
        else
        {
            printf("Invalid packet number: %d\n", packet_no);
        }

        if (send(client_socket, &ack_no,
                 sizeof(int), 0) != sizeof(int))
        {
            perror("send");
            break;
        }
    }

    printf("\nAll packets received successfully!\n");

    free(received);
    free(lost_once);

    close(client_socket);
    close(server_socket);

    return 0;
}