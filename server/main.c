#include "Server.h"
#include <stdio.h>

void launch_server(struct Server *server) {
    while (1) {
        printf("Waiting for connections...\n");
        
        SOCKET client_socket;
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        
        client_socket = accept(server->socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }
        
        printf("Client connected!\n");
        handle_connection(client_socket);
    }
}

int main() {
    struct Server *server = server_constructor(
        AF_INET,      // IPv4
        SOCK_STREAM,  // TCP
        0,            // IP Protocol
        INADDR_ANY,   // Interface
        8082,         // Port
        10,           // Backlog
        launch_server // Launch function
    );
    
    if (server == NULL) {
        printf("Server creation failed\n");
        return 1;
    }
    
    printf("Server started successfully on port %d\n", server->port);
    server->launch(server);
    
    // Cleanup
    closesocket(server->socket);
    free(server);
    WSACleanup();
    
    return 0;
}