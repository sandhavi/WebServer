#include "Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

struct Server *server_constructor(
    int domain,
    int service,
    int protocol,
    u_long iface,
    int port,
    int backlog,
    void (*launch)(struct Server *server))
{
    struct Server *server = (struct Server *)malloc(sizeof(struct Server));
    if (!server)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed\n");
        free(server);
        exit(1);
    }

    // Assign properties
    server->domain = domain;
    server->service = service;
    server->protocol = protocol;
    server->iface = iface;
    server->port = port;
    server->backlog = backlog;
    server->launch = launch; // Assign function pointer properly

    // Initialize server address structure
    server->address.sin_family = domain;
    server->address.sin_port = htons(port);
    server->address.sin_addr.s_addr = htonl(iface);

    // Create socket
    server->socket = socket(domain, service, protocol);
    if (server->socket == INVALID_SOCKET)
    {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        free(server);
        WSACleanup();
        exit(1);
    }

    // Bind socket
    if (bind(server->socket, (struct sockaddr *)&server->address, sizeof(server->address)) == SOCKET_ERROR)
    {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server->socket);
        free(server);
        WSACleanup();
        exit(1);
    }

    // Start listening
    if (listen(server->socket, backlog) == SOCKET_ERROR)
    {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(server->socket);
        free(server);
        WSACleanup();
        exit(1);
    }

    return server;
}
