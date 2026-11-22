#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SOCKET;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define closesocket(s) close(s)
#endif

#include "Router.h"

using namespace std;

// Simple HTTP server listening for requests on specified port
class HttpServer {
private:
    int port;
    Router* router;
    bool isRunning;
    SOCKET serverSocket;

    // Process a single client request
    void handleClient(SOCKET clientSocket);

public:
    // Constructor
    HttpServer(int serverPort, Router* serverRouter);

    // Destructor
    ~HttpServer();

    // Start server loop
    void start();

    // Stop server
    void stop();
};

#endif // HTTP_SERVER_H
