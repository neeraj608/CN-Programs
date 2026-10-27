
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

int main()
{
    /* CREATE SERVER SOCKET */
    int serversocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serversocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    /* SERVER ADDRESS */
    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8080);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    /* BIND SOCKET */
    int bind_status = bind(serversocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress));

    if (bind_status < 0)
    {
        perror("Error binding socket");
        close(serversocket);
        return 1;
    }

    /* LISTEN */
    if (listen(serversocket, 5) < 0)
    {
        perror("Listen failed");
        close(serversocket);
        return 1;
    }

    /* ACCEPT CLIENT */
    printf("Waiting for the client...\n");

    int clientsocket = accept(serversocket, NULL, NULL);

    if (clientsocket < 0)
    {
        perror("Error connecting to client");
        close(serversocket);
        return 1;
    }
    else
    {
        printf("Client connected successfully...\n\n");
    }

    /* RECEIVE FILENAME AND FILE SIZE */
    size_t file_size;
    char filename[256];

    int bytes = recv(clientsocket, filename, sizeof(filename), MSG_WAITALL);

    if (bytes != sizeof(filename))
    {
        perror("Error receiving filename");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    filename[sizeof(filename) - 1] = '\0';

    bytes = recv(clientsocket, &file_size, sizeof(file_size), MSG_WAITALL);

    if (bytes != sizeof(file_size))
    {
        perror("Error receiving file size");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    printf("Filename: %s\n", filename);
    printf("File size: %zu bytes\n", file_size);

    /* CREATE OUTPUT FILE */
    char output_path[512];

    snprintf(output_path, sizeof(output_path), "received_files/%s", filename);

    FILE *fp = fopen(output_path, "wb");

    if (fp == NULL)
    {
        perror("File open failed");
        close(clientsocket);
        close(serversocket);
        return 1;
    }

    /* RECEIVE FILE DATA */
    char buffer[BUFFER_SIZE];
    size_t total_size_received = 0;

    while (total_size_received < file_size)
    {
        size_t remaining_size = file_size - total_size_received;
        size_t chunk_size;

        if (remaining_size < sizeof(buffer))
        {
            chunk_size = remaining_size;
        }
        else
        {
            chunk_size = sizeof(buffer);
        }

        ssize_t received_status = recv(clientsocket, buffer, chunk_size, 0);

        if (received_status <= 0)
        {
            perror("Error receiving file chunk");
            fclose(fp);
            close(clientsocket);
            close(serversocket);
            return 1;
        }

        size_t written = fwrite(buffer, 1, received_status, fp);

        if (written != (size_t)received_status)
        {
            perror("Error writing file");
            fclose(fp);
            close(clientsocket);
            close(serversocket);
            return 1;
        }

        total_size_received += received_status;
    }

    fclose(fp);

    printf("\nFile received successfully.\n");
    printf("Total bytes received: %zu\n", total_size_received);

    close(clientsocket);
    close(serversocket);

    return 0;
}
