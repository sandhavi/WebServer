#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>

struct Server {
    int domain;
    int service;
    int protocol;
    u_long iface;
    int port;
    int backlog;
    
    SOCKET socket;
    struct sockaddr_in address;
    
    void (*launch)(struct Server *server);
};

// Function declarations
void log_event(const char *message);
void handle_error(SOCKET client_socket, const char *error_message, int status_code);
void handle_connection(SOCKET client_socket);
struct Server *server_constructor(int domain, int service, int protocol, u_long iface, int port, int backlog, void (*launch)(struct Server *server));

#endif