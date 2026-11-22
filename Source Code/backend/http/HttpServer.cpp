#include "HttpServer.h"
#include <iostream>
#include <sstream>

using namespace std;

// Constructor
HttpServer::HttpServer(int serverPort, Router* serverRouter)
    : port(serverPort), router(serverRouter), isRunning(false), serverSocket(INVALID_SOCKET) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

// Destructor
HttpServer::~HttpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

// Handle an incoming HTTP client request
void HttpServer::handleClient(SOCKET clientSocket) {
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        return;
    }

    buffer[bytesReceived] = '\0';
    string rawRequest(buffer, bytesReceived);

    // Read full body if Content-Length is provided in header
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    if (headerEnd != string::npos) {
        string headers = rawRequest.substr(0, headerEnd);
        size_t pos = headers.find("Content-Length:");
        if (pos == string::npos) pos = headers.find("content-length:");

        if (pos != string::npos) {
            size_t valStart = pos + 15;
            size_t valEnd = headers.find("\r\n", valStart);
            int contentLength = stoi(headers.substr(valStart, valEnd - valStart));

            size_t currentBodyLen = rawRequest.length() - (headerEnd + 4);
            while (currentBodyLen < (size_t)contentLength) {
                int more = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (more <= 0) break;
                rawRequest.append(buffer, more);
                currentBodyLen += more;
            }
        }
    }

    // Parse method, endpoint, and body
    istringstream stream(rawRequest);
    string method, endpoint, version;
    stream >> method >> endpoint >> version;

    string body = "";
    if (headerEnd != string::npos) {
        body = rawRequest.substr(headerEnd + 4);
    }

    // Dispatch request through router
    HttpRequest req(method, endpoint, body);
    HttpResponse res = router->handleRequest(req);

    // Send HTTP response
    string responseData = res.serialize();
    send(clientSocket, responseData.c_str(), (int)responseData.length(), 0);

    closesocket(clientSocket);
}

// Start HTTP listening loop
void HttpServer::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "[HttpServer] Error: Failed to create socket." << endl;
        return;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        cerr << "[HttpServer] Error: Failed to bind port " << port << endl;
        closesocket(serverSocket);
        return;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        cerr << "[HttpServer] Error: Failed to listen on socket." << endl;
        closesocket(serverSocket);
        return;
    }

    isRunning = true;
    cout << "==========================================================" << endl;
    cout << "  Vehicle Rental System - C++ Backend Server Running" << endl;
    cout << "  Listening on: http://localhost:" << port << endl;
    cout << "  Static Web:   http://localhost:" << port << "/index.html" << endl;
    cout << "==========================================================" << endl;

    while (isRunning) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            if (!isRunning) break;
            continue;
        }

        handleClient(clientSocket);
    }
}

// Stop server
void HttpServer::stop() {
    if (isRunning) {
        isRunning = false;
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }
        cout << "[HttpServer] Server stopped." << endl;
    }
}
