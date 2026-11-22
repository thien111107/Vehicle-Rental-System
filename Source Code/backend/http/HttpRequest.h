#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>

using namespace std;

// Representation of an incoming HTTP request
class HttpRequest {
private:
    string method;
    string endpoint;
    string body;

public:
    // Default constructor
    HttpRequest() : method("GET"), endpoint("/"), body("") {}

    // Parameterized constructor
    HttpRequest(const string& methodVal, const string& endpointVal, const string& bodyVal)
        : method(methodVal), endpoint(endpointVal), body(bodyVal) {}

    // Getters and setters
    string getMethod() const { return method; }
    void setMethod(const string& methodVal) { method = methodVal; }

    string getEndpoint() const { return endpoint; }
    void setEndpoint(const string& endpointVal) { endpoint = endpointVal; }

    string getBody() const { return body; }
    void setBody(const string& bodyVal) { body = bodyVal; }
};

#endif // HTTP_REQUEST_H
