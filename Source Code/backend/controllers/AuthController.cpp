#include "AuthController.h"
#include <sstream>

using namespace std;

// Constructor
AuthController::AuthController(DatabaseConnection* dbConnection)
    : db(dbConnection) {}

// Process user login
string AuthController::login(const string& username, const string& password) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection not available!\"}";
    }

    User* user = db->findUserByUsername(username);
    if (!user) {
        return "{\"success\": false, \"message\": \"User does not exist! Please check username.\"}";
    }

    if (user->getPassword() != password) {
        return "{\"success\": false, \"message\": \"Incorrect password! Please try again.\"}";
    }

    ostringstream json;
    json << "{\"success\": true, \"message\": \"Login successful!\", \"data\": {";
    json << "\"userId\": " << user->getId() << ", ";
    json << "\"username\": \"" << user->getUsername() << "\", ";
    json << "\"role\": \"" << user->getRole() << "\"";

    if (user->getRole() == "Customer") {
        Customer* customer = (Customer*)user;
        json << ", \"cccd\": \"" << customer->getCccd() << "\"";
        json << ", \"driverLicense\": \"" << customer->getDriverLicense() << "\"";
        json << ", \"balance\": " << customer->getBalance();
    }

    json << "}}";
    return json.str();
}

// Register a new customer
string AuthController::registerCustomer(const string& username, const string& password) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection not available!\"}";
    }

    if (username.empty() || password.empty()) {
        return "{\"success\": false, \"message\": \"Username and password cannot be empty!\"}";
    }

    int newUserId = 0;
    bool success = db->insertCustomer(username, password, newUserId);
    if (!success) {
        return "{\"success\": false, \"message\": \"Username already taken! Please choose another.\"}";
    }

    ostringstream json;
    json << "{\"success\": true, \"message\": \"Customer registered successfully!\", \"data\": {";
    json << "\"userId\": " << newUserId << ", ";
    json << "\"username\": \"" << username << "\", ";
    json << "\"role\": \"Customer\", ";
    json << "\"cccd\": \"\", ";
    json << "\"driverLicense\": \"\", ";
    json << "\"balance\": 0.0";
    json << "}}";
    return json.str();
}
