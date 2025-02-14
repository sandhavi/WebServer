#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Server.h"

#pragma comment(lib, "ws2_32.lib") // Link Winsock Library

void launch(struct Server *server)
{
    while (1)
    {
        char buffer[30000];
        printf("==== Waiting for connection ====\n");

        int address_length = sizeof(server->address);
        SOCKET new_socket = accept(server->socket, (struct sockaddr *)&server->address, &address_length);

        if (new_socket == INVALID_SOCKET)
        {
            printf("Accept failed: %d\n", WSAGetLastError());
            return;
        }

        recv(new_socket, buffer, sizeof(buffer) - 1, 0);
        buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination

        printf("Received request:\n%s\n", buffer);

        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><h1>Hello, World!</h1></body></html>";

        send(new_socket, response, strlen(response), 0);
        printf("==== Response sent ====\n");

        closesocket(new_socket);
    }
}

int main()
{
    struct Server *server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8081, 10, launch);

    if (!server)
    {
        printf("Server creation failed.\n");
        return 1;
    }

    server->launch(server);

    // Cleanup
    closesocket(server->socket);
    WSACleanup();
    free(server);

    return 0;
}
