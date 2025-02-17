#include "Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void log_event(const char *message)
{
    printf("[LOG]: %s\n", message);
}

void handle_error(SOCKET client_socket, const char *error_message, int status_code)
{
    printf("[ERROR %d]: %s\n", status_code, error_message);
    closesocket(client_socket);
}

const char *get_mime_type(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "text/plain";

    // Make sure to return "text/html" with proper headers
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=UTF-8"; // Added charset
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".js") == 0)
        return "application/javascript";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    return "application/octet-stream";
}

void send_file(SOCKET client_socket, const char *filename)
{
    // Handle root path request
    const char *actual_filename = filename;
    if (strlen(filename) == 0)
    {
        actual_filename = "index.html";
    }

    // Silently ignore favicon requests
    if (strcmp(actual_filename, "favicon.ico") == 0)
    {
        const char *empty_response = "HTTP/1.1 204 No Content\r\n\r\n";
        send(client_socket, empty_response, strlen(empty_response), 0);
        return;
    }

    // Basic filename validation (whitelist)
    if (strcmp(actual_filename, "index.html") != 0 &&
        strcmp(actual_filename, "contact.html") != 0 && // Add contact.html to whitelist
        strcmp(actual_filename, "style.css") != 0 &&
        strcmp(actual_filename, "script.js") != 0)
    {
        printf("[ERROR]: Invalid filename requested: %s\n", actual_filename);
        const char *bad_request_response = "HTTP/1.1 400 Bad Request\r\n"
                                           "Content-Type: text/html\r\n"
                                           "Content-Length: 37\r\n"
                                           "Connection: close\r\n"
                                           "\r\n"
                                           "<html><body><h1>400 Bad Request</h1></body></html>";
        send(client_socket, bad_request_response, strlen(bad_request_response), 0);
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "D:/UCSC/Y2S2/Computer_Network/WebServer/html/%s", actual_filename);

    printf("Looking for file at absolute path: %s\n", filepath);

    FILE *file = fopen(filepath, "rb");
    if (file == NULL)
    {
        perror("File open error");
        printf("[ERROR 404]: File not found -> %s\n", filepath);

        const char *not_found_response = "HTTP/1.1 404 Not Found\r\n"
                                         "Content-Type: text/html\r\n"
                                         "Content-Length: 46\r\n"
                                         "Connection: close\r\n"
                                         "\r\n"
                                         "<html><body><h1>404 Not Found</h1></body></html>";
        send(client_socket, not_found_response, strlen(not_found_response), 0);
        return;
    }

    printf("File opened successfully!\n");

    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Determine MIME type
    const char *mime_type = get_mime_type(filename);

    // Construct the HTTP response header
    char response_header[512];
    snprintf(response_header, sizeof(response_header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n",
             mime_type, file_size);

    send(client_socket, response_header, strlen(response_header), 0);

    // Read and send file content
    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        int bytesSent = send(client_socket, buffer, bytesRead, 0);
        if (bytesSent == SOCKET_ERROR)
        {
            perror("Send failed");
            fclose(file);
            closesocket(client_socket);
            return;
        }
    }

    fclose(file);
    printf("File sent successfully!\n");
}

void handle_connection(SOCKET client_socket)
{
    char buffer[2048];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == SOCKET_ERROR)
    {
        perror("Receive failed");
        closesocket(client_socket);
        return;
    }
    if (bytes_received == 0)
    {
        printf("Client disconnected.\n");
        closesocket(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';
    printf("Received request:\n%s\n", buffer);

    // Parse the request
    char *line = strtok(buffer, "\r\n");
    if (line == NULL)
    {
        printf("Invalid request.\n");
        closesocket(client_socket);
        return;
    }

    char method[16], path[256], protocol[16];
    if (sscanf(line, "%s %s %s", method, path, protocol) != 3)
    {
        printf("Invalid request line.\n");
        closesocket(client_socket);
        return;
    }

    printf("Method: %s, Path: %s, Protocol: %s\n", method, path, protocol);

    // Basic request validation
    if (strcmp(method, "GET") != 0)
    {
        printf("Only GET requests are supported.\n");
        closesocket(client_socket);
        return;
    }

    // Handle root path
    char *filename;
    if (strcmp(path, "/") == 0)
    {
        filename = "index.html"; // Default file for root path
    }
    else if (strcmp(path, "/contact") == 0)
    {
        filename = "contact.html"; // Handle /contact path
    }
    else
    {
        // Remove leading slash from the path
        filename = path + 1;
    }

    // Send the file
    send_file(client_socket, filename);

    closesocket(client_socket);
}
struct Server *server_constructor(
    int domain,
    int service,
    int protocol,
    u_long iface,
    int port,
    int backlog,
    void (*launch)(struct Server *server))
{

    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return NULL;
    }

    struct Server *server = (struct Server *)malloc(sizeof(struct Server));
    if (server == NULL)
    {
        perror("Memory allocation for server failed");
        WSACleanup();
        return NULL;
    }

    server->domain = domain;
    server->service = service;
    server->protocol = protocol;
    server->iface = iface;
    server->port = port;
    server->backlog = backlog;

    server->socket = socket(domain, service, protocol);
    if (server->socket == INVALID_SOCKET)
    {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        free(server);
        WSACleanup();
        return NULL;
    }

    server->address.sin_family = AF_INET;
    server->address.sin_addr.s_addr = INADDR_ANY;
    server->address.sin_port = htons(port);

    // Binding the socket
    if (bind(server->socket, (struct sockaddr *)&server->address, sizeof(server->address)) == SOCKET_ERROR)
    {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server->socket);
        free(server);
        WSACleanup();
        return NULL;
    }

    // Listen for incoming connections
    if (listen(server->socket, backlog) == SOCKET_ERROR)
    {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server->socket);
        free(server);
        WSACleanup();
        return NULL;
    }

    server->launch = launch;
    return server;
}