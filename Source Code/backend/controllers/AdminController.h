#ifndef ADMIN_CONTROLLER_H
#define ADMIN_CONTROLLER_H

#include <string>
#include "../database/DatabaseConnection.h"

using namespace std;

// Controller managing system administration, manager accounts, and revenue analytics
class AdminController {
private:
    DatabaseConnection* db;

public:
    // Constructor
    AdminController(DatabaseConnection* dbConnection);

    // Create a new manager account
    string addManager(const string& username, const string& password);

    // Delete a manager account by ID
    string deleteManager(int managerId);

    // Calculate total system revenue and order breakdown
    string calculateTotalRevenue();

    // Get list of all manager accounts
    string getAllManagers();
};

#endif // ADMIN_CONTROLLER_H
