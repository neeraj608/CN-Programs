
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

int main()
{
    /* CREATE SOCKET */
    int clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientsocket < 0)
    {
        perror("Error creating socket");
        return 1;
    }

    /* SERVER ADDRESS */
    struct sockaddr_in serveraddress = {0};

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8080);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* CONNECT TO SERVER */
    printf("Connecting to server...\n");

    if (connect(clientsocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) < 0)
    {
        perror("Error connecting to server");
        close(clientsocket);
        return 1;
    }
    else
    {
        printf("Server connected successfully...\n");
    }

    /* INPUT FILE PATH */
    char filepath[256];

    printf("Enter the file path: ");
    fgets(filepath, sizeof(filepath), stdin);

    filepath[strcspn(filepath, "\n")] = '\0';

    FILE *fp = fopen(filepath, "rb");

    if (fp == NULL)
    {
        perror("File open failed");
        close(clientsocket);
        return 1;
    }

    /* EXTRACT FILENAME */
    char filename[256];
    int pathlen = strlen(filepath);
    int i = pathlen - 1;

    while (i >= 0 && filepath[i] != '/')
    {
        i--;
    }

    int ptr = i + 1;
    int nameptr = 0;

    while (filepath[ptr] != '\0')
    {
        filename[nameptr++] = filepath[ptr++];
    }

    filename[nameptr] = '\0';

    printf("Filename: %s\n", filename);

    /* EXTRACT FILE SIZE */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    /* SEND FILENAME AND FILE SIZE */
    if (send(clientsocket, filename, sizeof(filename), 0) < 0)
    {
        perror("Error sending filename");
        fclose(fp);
        close(clientsocket);
        return 1;
    }

    if (send(clientsocket, &file_size, sizeof(file_size), 0) < 0)
    {
        perror("Error sending file size");
        fclose(fp);
        close(clientsocket);
        return 1;
    }

    /* SEND FILE DATA USING CHUNKS */
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t size_total_sent = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        if (send(clientsocket, buffer, bytes_read, 0) < 0)
        {
            perror("Error sending file");
            fclose(fp);
            close(clientsocket);
            return 1;
        }

        size_total_sent += bytes_read;
    }

    printf("File sent successfully.\n");
    printf("Total bytes sent: %zu\n", size_total_sent);

    fclose(fp);
    close(clientsocket);

    return 0;
}
