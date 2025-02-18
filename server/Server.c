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

    // return "text/html" wit proper headers
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=UTF-8"; 
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
    if (strcmp(dot, ".ico") == 0)
        return "image/x-icon";
    if (strcmp(dot, ".webp") == 0)
        return "image/webp";
    if (strcmp(dot, ".mp4") == 0)
        return "video/mp4";
    return "application/octet-stream";
}

void send_file(SOCKET client_socket, const char *path)
{
    // Remove leading slash if present
    const char *clean_path = path;
    if (path[0] == '/')
    {
        clean_path = path + 1;
    }

    // Handle root path request
    if (strlen(clean_path) == 0)
    {
        clean_path = "index.html";
    }

    // ignore favicon requests
    if (strcmp(clean_path, "favicon.ico") == 0)
    {
        const char *empty_response = "HTTP/1.1 204 No Content\r\n\r\n";
        send(client_socket, empty_response, strlen(empty_response), 0);
        return;
    }

    char filepath[512];
    const char *dot = strrchr(clean_path, '.');

    if (!dot)
    {

        snprintf(filepath, sizeof(filepath), "../src//html/%s.html", clean_path);
    }
    else
    {
        if (strcmp(dot, ".html") == 0)
        {
            snprintf(filepath, sizeof(filepath), "../src/html/%s", clean_path);
        }
        else if (strcmp(dot, ".css") == 0)
        {
            snprintf(filepath, sizeof(filepath), "../src/css/%s", clean_path);
        }
        else if (strcmp(dot, ".js") == 0)
        {
            snprintf(filepath, sizeof(filepath), "../src/js/%s", clean_path);
        }
        else
        {
            snprintf(filepath, sizeof(filepath), "../public/%s", clean_path);
        }
    }

    // Allowed file extensions
    const char *allowed_extensions[] = {
        ".html", ".css", ".js",
        ".jpg", ".jpeg", ".png", ".gif", ".ico", ".webp",
        ".mp4", ".mp3", ".wav",
        NULL};

    // Validate file extension
    int is_allowed = 0;
    const char *file_ext = dot ? dot : ".html"; 

    for (int i = 0; allowed_extensions[i] != NULL; i++)
    {
        if (strcmp(file_ext, allowed_extensions[i]) == 0)
        {
            is_allowed = 1;
            break;
        }
    }

    if (!is_allowed)
    {
        printf("[ERROR]: Invalid file type requested: %s\n", clean_path);
        const char *bad_request_response = "HTTP/1.1 400 Bad Request\r\n"
                                           "Content-Type: text/html\r\n"
                                           "Content-Length: 37\r\n"
                                           "\r\n"
                                           "<html><body><h1>400 Bad Request</h1></body></html>";
        send(client_socket, bad_request_response, strlen(bad_request_response), 0);
        return;
    }

    printf("Looking for file at path: %s\n", filepath);

    FILE *file = fopen(filepath, "rb");
    if (file == NULL)
    {
        printf("[ERROR 404]: File not found -> %s\n", filepath);
        const char *not_found_response = "HTTP/1.1 404 Not Found\r\n"
                                         "Content-Type: text/html\r\n"
                                         "Content-Length: 46\r\n"
                                         "\r\n"
                                         "<html><body><h1>404 Not Found</h1></body></html>";
        send(client_socket, not_found_response, strlen(not_found_response), 0);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    const char *mime_type = get_mime_type(filepath);

    char response_header[512];
    snprintf(response_header, sizeof(response_header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n",
             mime_type, file_size);

    send(client_socket, response_header, strlen(response_header), 0);

    // Send file content
    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        if (send(client_socket, buffer, bytes_read, 0) == SOCKET_ERROR)
        {
            perror("Send failed");
            break;
        }
    }

    fclose(file);
    printf("File sent successfully: %s\n", filepath);
}

void handle_connection(SOCKET client_socket)
{
    char buffer[2048];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0)
    {
        closesocket(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';

    // Parse the request
    char method[16], path[256], protocol[16];
    if (sscanf(buffer, "%s %s %s", method, path, protocol) != 3)
    {
        closesocket(client_socket);
        return;
    }

    printf("Received request: %s %s %s\n", method, path, protocol);

    if (strcmp(method, "GET") != 0)
    {
        const char *method_not_allowed = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        send(client_socket, method_not_allowed, strlen(method_not_allowed), 0);
        closesocket(client_socket);
        return;
    }

    // Handle request
    send_file(client_socket, path);
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

    // Winsock
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