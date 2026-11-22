#ifndef ADMIN_H
#define ADMIN_H

#include "Manager.h"

// Admin class inheriting from Manager
class Admin : public Manager {
public:
    // Constructor
    Admin(int userId, const string& usernameVal, const string& passwordVal)
        : Manager(userId, usernameVal, passwordVal, "Admin") {}

    // Method to display admin summary
    void display() {
        cout << "[Admin Summary] Superuser ID: " << id << " | Username: " << userName << endl;
    }

    // Method to display detailed admin info
    void displayInfo() override {
        cout << "[Admin Superuser] ID: " << id
             << " | Username: " << userName
             << " | Role: " << role << endl;
    }
};

#endif // ADMIN_H
