#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <string>
#include "../database/DatabaseConnection.h"

using namespace std;

// Controller managing user authentication and customer registration
class AuthController {
private:
    DatabaseConnection* db;

public:
    // Constructor
    AuthController(DatabaseConnection* dbConnection);

    // Process user login
    string login(const string& username, const string& password);

    // Register a new customer
    string registerCustomer(const string& username, const string& password);
};

#endif // AUTH_CONTROLLER_H
