#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib") // Winsock Library

struct Server
{
    int domain;
    int service;
    int protocol;
    u_long iface;
    int port;
    int backlog;

    struct sockaddr_in address;
    SOCKET socket;
    void (*launch)(struct Server *server); // Function pointer corrected
};

// Function prototype updated to return a pointer
struct Server *server_constructor(
    int domain,
    int service,
    int protocol,
    u_long iface,
    int port,
    int backlog,
    void (*launch)(struct Server *server));

#endif
