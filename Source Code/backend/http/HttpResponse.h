#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>

using namespace std;

// Representation of an outgoing HTTP response
class HttpResponse {
private:
    int statusCode;
    string body;
    string contentType;

public:
    // Default constructor
    HttpResponse() : statusCode(200), body(""), contentType("application/json; charset=utf-8") {}

    // Parameterized constructor
    HttpResponse(int status, const string& bodyVal, const string& typeVal = "application/json; charset=utf-8")
        : statusCode(status), body(bodyVal), contentType(typeVal) {}

    // Getters and setters
    int getStatusCode() const { return statusCode; }
    void setStatusCode(int status) { statusCode = status; }

    string getBody() const { return body; }
    void setBody(const string& bodyVal) { body = bodyVal; }

    string getContentType() const { return contentType; }
    void setContentType(const string& typeVal) { contentType = typeVal; }

    // Build raw HTTP response string
    string serialize() const {
        string statusText = "OK";
        if (statusCode == 201) statusText = "Created";
        else if (statusCode == 204) statusText = "No Content";
        else if (statusCode == 400) statusText = "Bad Request";
        else if (statusCode == 401) statusText = "Unauthorized";
        else if (statusCode == 403) statusText = "Forbidden";
        else if (statusCode == 404) statusText = "Not Found";
        else if (statusCode == 500) statusText = "Internal Server Error";

        string rawResponse = "HTTP/1.1 " + to_string(statusCode) + " " + statusText + "\r\n";
        rawResponse += "Content-Type: " + contentType + "\r\n";
        rawResponse += "Content-Length: " + to_string(body.size()) + "\r\n";
        rawResponse += "Access-Control-Allow-Origin: *\r\n";
        rawResponse += "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n";
        rawResponse += "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
        rawResponse += "Connection: close\r\n\r\n";
        rawResponse += body;
        return rawResponse;
    }
};

#endif // HTTP_RESPONSE_H
