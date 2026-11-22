#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif
#include <iostream>
#include "database/DatabaseConnection.h"
#include "controllers/AuthController.h"
#include "controllers/VehicleController.h"
#include "controllers/RentalController.h"
#include "controllers/AdminController.h"
#include "controllers/NotificationController.h"
#include "http/Router.h"
#include "http/HttpServer.h"

using namespace std;

// Main entry point for the Vehicle Rental C++ Backend application
int main() {
    cout << "[System] Initializing Vehicle Rental MVC Backend..." << endl;

    // 1. Initialize Singleton Database Connection
    DatabaseConnection* db = DatabaseConnection::getInstance();
    if (!db->connect()) {
        cerr << "[System] Fatal Error: Could not connect to database!" << endl;
        return 1;
    }

    // 2. Instantiate System Controllers matching Diagram 1
    NotificationController notifCtrl(db);
    AuthController authCtrl(db);
    VehicleController vehicleCtrl(db);
    RentalController rentalCtrl(db, &notifCtrl);
    AdminController adminCtrl(db);

    // 3. Instantiate Request Router with relative static path to frontend
    Router router(&authCtrl, &vehicleCtrl, &rentalCtrl, &adminCtrl, &notifCtrl, "../frontend");

    // 4. Start HTTP Server on port 8080
    int port = 8080;
    HttpServer server(port, &router);

    cout << "[System] Server initialized successfully. Ready for incoming requests." << endl;
    server.start();

    return 0;
}
