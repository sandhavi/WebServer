#ifndef HTTPRequest_h
#define HTTPRequest_h

#include <ws2tcpip.h>
#include <stdio.h>

//Numeration of HTTP methods
enum HTTPMethod  // Headers that available for http(RESTFUL API) 
{
    GET,
    POST,
    PUT,
    HTTP_DELETE,
    PATCH,
    HEAD,
    OPTIONS,
    TRACE,
    CONNECT
};

struct HTTPRequest
{
    int Method;
    char *URI;
    float HTTPVersion;
};

struct HTTPRequest http_request_constructor(
    char *request_string);


#endif // HTTPRequest_h