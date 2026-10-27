
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        perror("Socket creation failed");
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
        perror("Bind failed");
        close(server_socket);
        return 1;
    }

    listen(server_socket, 3);

    printf("Waiting for client...\n");

    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket < 0)
    {
        perror("Accept failed");
        close(server_socket);
        return 1;
    }

    int window_size, no_of_packets;

    if (recv(client_socket, &window_size,
             sizeof(int), MSG_WAITALL) != sizeof(int) ||
        recv(client_socket, &no_of_packets,
             sizeof(int), MSG_WAITALL) != sizeof(int))
    {
        printf("Failed to receive parameters\n");
        close(client_socket);
        close(server_socket);
        return 1;
    }

    if (window_size <= 0 || no_of_packets <= 0)
    {
        printf("Invalid parameters\n");
        close(client_socket);
        close(server_socket);
        return 1;
    }

    for (int i = 1; i <= no_of_packets; i++)
    {
        int packet_no;

        if (recv(client_socket, &packet_no,
                 sizeof(int), MSG_WAITALL) != sizeof(int))
        {
            break;
        }

        printf("Packet %d received\n", packet_no);

        int ack_no = packet_no;

        send(client_socket, &ack_no, sizeof(int), 0);
    }

    close(client_socket);
    close(server_socket);

    return 0;
}
