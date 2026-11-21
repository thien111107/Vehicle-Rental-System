#include "AdminController.h"
#include <sstream>

using namespace std;

// Constructor
AdminController::AdminController(DatabaseConnection* dbConnection)
    : db(dbConnection) {}

// Add a manager account
string AdminController::addManager(const string& username, const string& password) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection not available!\"}";
    }

    if (username.empty() || password.empty()) {
        return "{\"success\": false, \"message\": \"Manager username and password cannot be empty!\"}";
    }

    int createdManagerId = 0;
    bool success = db->insertManager(username, password, createdManagerId);
    if (success) {
        return "{\"success\": true, \"message\": \"Manager account created successfully!\", \"managerId\": " +
               to_string(createdManagerId) + "}";
    }

    return "{\"success\": false, \"message\": \"Username already taken! Please choose another.\"}";
}

// Delete a manager account
string AdminController::deleteManager(int managerId) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection not available!\"}";
    }

    bool success = db->deleteManager(managerId);
    if (success) {
        return "{\"success\": true, \"message\": \"Manager account #" + to_string(managerId) + " deleted successfully!\"}";
    }

    return "{\"success\": false, \"message\": \"Manager account not found or cannot be deleted!\"}";
}

// Calculate total system revenue and counts
string AdminController::calculateTotalRevenue() {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database not available!\"}";
    }

    vector<RentalOrder> rentedOrders = db->getApprovedOrCompletedOrders();
    vector<RentalOrder> pendingOrders = db->getPendingOrders();

    double totalRevenue = 0.0;
    int approvedCount = 0;
    for (size_t i = 0; i < rentedOrders.size(); i++) {
        totalRevenue += rentedOrders[i].getTotalAmount();
        approvedCount++;
    }

    int pendingCount = (int)pendingOrders.size();
    int rejectedCount = 0;

    ostringstream json;
    json << "{\"success\": true, \"revenue\": {"
         << "\"totalRevenue\": " << (long long)totalRevenue << ", "
         << "\"approvedOrders\": " << approvedCount << ", "
         << "\"pendingOrders\": " << pendingCount << ", "
         << "\"rejectedOrders\": " << rejectedCount
         << "}}";
    return json.str();
}

// Get all manager accounts
string AdminController::getAllManagers() {
    if (!db) return "{\"success\": false, \"data\": []}";

    vector<Manager*> managers = db->getAllManagers();
    ostringstream json;
    json << "{\"success\": true, \"data\": [";

    for (size_t i = 0; i < managers.size(); i++) {
        Manager* m = managers[i];
        json << "{"
             << "\"id\": " << m->getId() << ", "
             << "\"managerId\": " << m->getId() << ", "
             << "\"username\": \"" << m->getUsername() << "\", "
             << "\"role\": \"" << m->getRole() << "\""
             << "}" << (i + 1 < managers.size() ? ", " : "");
    }

    json << "]}";
    return json.str();
}
