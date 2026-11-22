#ifndef MANAGER_H
#define MANAGER_H

#include "User.h"

// Manager class inheriting from abstract User
class Manager : public User {
public:
    // Constructor
    Manager(int userId, const string& usernameVal, const string& passwordVal, const string& roleVal = "Manager")
        : User(userId, usernameVal, passwordVal, roleVal) {}

    // Method to display manager summary
    void display() {
        cout << "[Manager Summary] ID: " << id << " | Username: " << userName << endl;
    }

    // Method to display detailed manager info
    void displayInfo() override {
        cout << "[Manager] ID: " << id
             << " | Username: " << userName
             << " | Role: " << role << endl;
    }
};

#endif // MANAGER_H
