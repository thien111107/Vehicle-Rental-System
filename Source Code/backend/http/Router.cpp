#include "Router.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

// Simple helper to find a string value in JSON
static string readJsonString(const string& json, const string& key) {
    string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == string::npos) return "";

    size_t colon = json.find(':', pos + search.length());
    if (colon == string::npos) return "";

    size_t q1 = json.find('"', colon + 1);
    if (q1 == string::npos) return "";

    size_t q2 = json.find('"', q1 + 1);
    if (q2 == string::npos) return "";

    return json.substr(q1 + 1, q2 - q1 - 1);
}

// Simple helper to find a number in JSON
static double readJsonNumber(const string& json, const string& key, double defaultValue = 0.0) {
    string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == string::npos) return defaultValue;

    size_t colon = json.find(':', pos + search.length());
    if (colon == string::npos) return defaultValue;

    size_t start = colon + 1;
    while (start < json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '"')) {
        start++;
    }

    size_t end = start;
    while (end < json.length() && (isdigit(json[end]) || json[end] == '.' || json[end] == '-' || json[end] == 'e' || json[end] == 'E' || json[end] == '+')) {
        end++;
    }

    if (end > start) {
        try {
            return stod(json.substr(start, end - start));
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

// Simple helper to extract query parameter (?type=Car)
static string readQueryParam(const string& url, const string& key) {
    size_t qPos = url.find('?');
    if (qPos == string::npos) return "";

    string query = url.substr(qPos + 1);
    string search = key + "=";
    size_t pos = query.find(search);
    if (pos == string::npos) return "";

    size_t start = pos + search.length();
    size_t end = query.find('&', start);
    if (end == string::npos) {
        return query.substr(start);
    }
    return query.substr(start, end - start);
}

// Helper to extract integer from query parameter or JSON body
static int readQueryOrJsonInt(const string& url, const string& jsonBody, const string& key, int defaultValue = 0) {
    string qVal = readQueryParam(url, key);
    if (!qVal.empty()) {
        try {
            return stoi(qVal);
        } catch (...) {
            return defaultValue;
        }
    }
    return (int)readJsonNumber(jsonBody, key, defaultValue);
}

// Constructor
Router::Router(AuthController* auth, VehicleController* vehicle, RentalController* rental,
               AdminController* admin, NotificationController* notif,
               const string& staticPath)
    : authCtrl(auth), vehicleCtrl(vehicle), rentalCtrl(rental),
      adminCtrl(admin), notifCtrl(notif), frontendStaticRoot(staticPath) {}

// Serve static frontend files (html, css, js)
HttpResponse Router::serveStaticFile(const string& relativePath) {
    string cleanPath = relativePath;
    if (cleanPath == "/" || cleanPath.empty()) {
        cleanPath = "/index.html";
    }

    string searchRoots[4] = {
        frontendStaticRoot,
        "frontend",
        "../frontend",
        "."
    };

    ifstream file;
    for (int i = 0; i < 4; i++) {
        string fullPath = searchRoots[i] + cleanPath;
        file.open(fullPath, ios::binary);
        if (file.is_open()) break;
    }

    if (!file.is_open()) {
        return HttpResponse(404, "{\"error\": \"File not found\"}", "application/json");
    }

    stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    string mimeType = "text/plain";
    if (cleanPath.find(".html") != string::npos) mimeType = "text/html; charset=utf-8";
    else if (cleanPath.find(".css") != string::npos) mimeType = "text/css; charset=utf-8";
    else if (cleanPath.find(".js") != string::npos) mimeType = "application/javascript; charset=utf-8";
    else if (cleanPath.find(".svg") != string::npos) mimeType = "image/svg+xml";

    return HttpResponse(200, buffer.str(), mimeType);
}

// Route incoming HTTP request to matching controller
HttpResponse Router::handleRequest(const HttpRequest& req) {
    string method = req.getMethod();
    string endpoint = req.getEndpoint();
    string body = req.getBody();

    string pathOnly = endpoint;
    size_t qPos = pathOnly.find('?');
    if (qPos != string::npos) {
        pathOnly = pathOnly.substr(0, qPos);
    }

    // CORS preflight
    if (method == "OPTIONS") {
        return HttpResponse(200, "", "text/plain");
    }

    // 1. Auth endpoints
    if (pathOnly == "/api/auth/login" && method == "POST") {
        string username = readJsonString(body, "username");
        string password = readJsonString(body, "password");
        return HttpResponse(200, authCtrl->login(username, password));
    }

    if (pathOnly == "/api/auth/register" && method == "POST") {
        string username = readJsonString(body, "username");
        string password = readJsonString(body, "password");
        return HttpResponse(200, authCtrl->registerCustomer(username, password));
    }

    // 2. Vehicle endpoints
    if (pathOnly == "/api/vehicles" && method == "GET") {
        string filter = readQueryParam(endpoint, "type");
        return HttpResponse(200, vehicleCtrl->getAvailableVehicles(filter));
    }

    if (pathOnly == "/api/vehicles/all" && method == "GET") {
        string filter = readQueryParam(endpoint, "type");
        return HttpResponse(200, vehicleCtrl->getAllVehicles(filter));
    }

    if (pathOnly == "/api/vehicles/count" && method == "GET") {
        return HttpResponse(200, vehicleCtrl->countVehicles());
    }

    if (pathOnly == "/api/vehicles/add" && method == "POST") {
        string type = readJsonString(body, "type");
        string brand = readJsonString(body, "brand");
        string model = readJsonString(body, "model");
        string plate = readJsonString(body, "licensePlate");
        double price = readJsonNumber(body, "pricePerDay");

        Vehicle* v = nullptr;
        if (type == "Car") {
            int seats = (int)readJsonNumber(body, "numSeats", 5);
            v = new Car(0, brand, model, plate, price, true, seats);
        } else {
            int cc = (int)readJsonNumber(body, "engineCapacity", 150);
            v = new Motobike(0, brand, model, plate, price, true, cc);
        }

        string res = vehicleCtrl->addVehicle(v);
        delete v;
        return HttpResponse(200, res);
    }

    if (pathOnly == "/api/vehicles/update" && method == "POST") {
        int id = (int)readJsonNumber(body, "id");
        string type = readJsonString(body, "type");
        string brand = readJsonString(body, "brand");
        string model = readJsonString(body, "model");
        string plate = readJsonString(body, "licensePlate");
        double price = readJsonNumber(body, "pricePerDay");

        Vehicle* v = nullptr;
        if (type == "Car") {
            int seats = (int)readJsonNumber(body, "numSeats", 5);
            v = new Car(id, brand, model, plate, price, true, seats);
        } else {
            int cc = (int)readJsonNumber(body, "engineCapacity", 150);
            v = new Motobike(id, brand, model, plate, price, true, cc);
        }

        string res = vehicleCtrl->updateVehicle(v);
        delete v;
        return HttpResponse(200, res);
    }

    if (pathOnly == "/api/vehicles/delete" && method == "POST") {
        int id = (int)readJsonNumber(body, "id");
        return HttpResponse(200, vehicleCtrl->deleteVehicle(id));
    }

    // 3. Rental endpoints
    if (pathOnly == "/api/rentals/create" && method == "POST") {
        int customerId = (int)readJsonNumber(body, "customerId");
        int vehicleId = (int)readJsonNumber(body, "vehicleId");
        string cccd = readJsonString(body, "cccd");
        string license = readJsonString(body, "driverLicense");
        string start = readJsonString(body, "startDate");
        string end = readJsonString(body, "endDate");

        return HttpResponse(200, rentalCtrl->createRental(customerId, vehicleId, cccd, license, start, end));
    }

    if (pathOnly == "/api/rentals/pending" && method == "GET") {
        return HttpResponse(200, rentalCtrl->getPendingOrders());
    }

    if (pathOnly == "/api/rentals/approve" && method == "POST") {
        int orderId = (int)readJsonNumber(body, "orderId");
        return HttpResponse(200, rentalCtrl->approveRental(orderId));
    }

    if (pathOnly == "/api/rentals/reject" && method == "POST") {
        int orderId = (int)readJsonNumber(body, "orderId");
        return HttpResponse(200, rentalCtrl->rejectRental(orderId));
    }

    if (pathOnly == "/api/rentals/history" && method == "GET") {
        int customerId = readQueryOrJsonInt(endpoint, body, "customerId");
        return HttpResponse(200, rentalCtrl->getCustomerHistory(customerId));
    }

    if (pathOnly == "/api/rentals/rented-customers" && method == "GET") {
        return HttpResponse(200, rentalCtrl->getRentedCustomers());
    }

    if (pathOnly == "/api/rentals/return" && method == "POST") {
        int orderId = (int)readJsonNumber(body, "orderId");
        int customerId = (int)readJsonNumber(body, "customerId");
        return HttpResponse(200, rentalCtrl->returnRental(customerId, orderId));
    }

    // 4. Notification endpoints
    if (pathOnly == "/api/notifications" && method == "GET") {
        int customerId = readQueryOrJsonInt(endpoint, body, "customerId");
        return HttpResponse(200, notifCtrl->getNotifications(customerId));
    }

    if (pathOnly == "/api/notifications/mark-read" && method == "POST") {
        int customerId = (int)readJsonNumber(body, "customerId");
        return HttpResponse(200, notifCtrl->markAsRead(customerId));
    }

    // 5. Admin endpoints
    if (pathOnly == "/api/admin/revenue" && method == "GET") {
        return HttpResponse(200, adminCtrl->calculateTotalRevenue());
    }

    if (pathOnly == "/api/admin/managers" && method == "GET") {
        return HttpResponse(200, adminCtrl->getAllManagers());
    }

    if (pathOnly == "/api/admin/managers/add" && method == "POST") {
        string username = readJsonString(body, "username");
        string password = readJsonString(body, "password");
        return HttpResponse(200, adminCtrl->addManager(username, password));
    }

    if (pathOnly == "/api/admin/managers/delete" && method == "POST") {
        int managerId = (int)readJsonNumber(body, "managerId");
        return HttpResponse(200, adminCtrl->deleteManager(managerId));
    }

    // Static web files
    if (pathOnly == "/" || pathOnly == "/index.html" || pathOnly == "/customer.html" ||
        pathOnly == "/manager.html" || pathOnly == "/admin.html" ||
        pathOnly.rfind("/css/", 0) == 0 || pathOnly.rfind("/js/", 0) == 0) {
        return serveStaticFile(pathOnly);
    }

    return HttpResponse(404, "{\"error\": \"Route not found!\"}", "application/json");
}
