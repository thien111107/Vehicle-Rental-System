#ifndef ROUTER_H
#define ROUTER_H

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "../controllers/AuthController.h"
#include "../controllers/VehicleController.h"
#include "../controllers/RentalController.h"
#include "../controllers/AdminController.h"
#include "../controllers/NotificationController.h"

using namespace std;

// Request router dispatching HTTP requests to controllers and static asset handlers
class Router {
private:
    AuthController* authCtrl;
    VehicleController* vehicleCtrl;
    RentalController* rentalCtrl;
    AdminController* adminCtrl;
    NotificationController* notifCtrl;
    string frontendStaticRoot;

    // Helper to serve static frontend files
    HttpResponse serveStaticFile(const string& relativePath);

public:
    // Constructor matching Diagram 1
    Router(AuthController* auth, VehicleController* vehicle, RentalController* rental,
           AdminController* admin, NotificationController* notif,
           const string& staticPath = "../frontend");

    // Main request handler
    HttpResponse handleRequest(const HttpRequest& req);
};

#endif // ROUTER_H
