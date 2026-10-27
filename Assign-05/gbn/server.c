
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
    int window_size, no_of_packets, nth_lost;
    int packet_no, ack_no;
    int expected_packet = 1;
    int count = 0;

    int serversocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serversocket < 0)
    {
        perror("Error creating socket");
        return 1;
    }

    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8080);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    if(bind(serversocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Error binding");
        close(serversocket);
        return 1;
    }

    if(listen(serversocket, 3) < 0)
    {
        perror("Error listening");
        close(serversocket);
        return 1;
    }

    printf("Waiting for client...\n");

    int clientsocket = accept(serversocket, NULL, NULL);

    if(clientsocket < 0)
    {
        perror("Error accepting connection");
        close(serversocket);
        return 1;
    }

    if(recv(clientsocket, &window_size, sizeof(window_size), MSG_WAITALL) != sizeof(window_size) ||
       recv(clientsocket, &no_of_packets, sizeof(no_of_packets), MSG_WAITALL) != sizeof(no_of_packets) ||
       recv(clientsocket, &nth_lost, sizeof(nth_lost), MSG_WAITALL) != sizeof(nth_lost))
    {
        perror("Error receiving parameters");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    if(window_size <= 0 || no_of_packets <= 0 || nth_lost < 0)
    {
        printf("Invalid parameters received\n");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    printf("Window = %d, Packets = %d, Lost every = %d\n\n",
           window_size, no_of_packets, nth_lost);

    while(expected_packet <= no_of_packets)
    {
        if(recv(clientsocket, &packet_no, sizeof(packet_no), MSG_WAITALL) != sizeof(packet_no))
        {
            break;
        }

        ack_no = -1;

        if(packet_no == expected_packet)
        {
            count++;

            if(nth_lost != 0 && count == nth_lost)
            {
                printf("Packet %d simulated as lost\n", packet_no);

                count = 0;
            }
            else
            {
                printf("Packet %d accepted, sending ACK\n", packet_no);

                ack_no = packet_no;
                expected_packet++;
            }
        }
        else
        {
            printf("Packet %d discarded (out of order, expected %d)\n",
                   packet_no, expected_packet);
        }

        if(send(clientsocket, &ack_no, sizeof(ack_no), 0) < 0)
        {
            perror("Error sending ACK");
            break;
        }
    }

    close(clientsocket);
    close(serversocket);

    return 0;
}
