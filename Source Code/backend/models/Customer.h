#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "User.h"

// Customer class inheriting from abstract User
class Customer : public User {
private:
    string cccd;
    string driverLicense;
    double balance;

public:
    // Constructor
    Customer(int userId, const string& usernameVal, const string& passwordVal,
             const string& cccdVal = "", const string& licenseVal = "", double balanceVal = 0.0)
        : User(userId, usernameVal, passwordVal, "Customer"),
          cccd(cccdVal), driverLicense(licenseVal), balance(balanceVal) {}

    // Method to display customer information
    void displayInfo() override {
        cout << "[Customer] ID: " << id
             << " | Username: " << userName
             << " | CCCD: " << cccd
             << " | License: " << driverLicense
             << " | Balance: " << balance << endl;
    }

    // Getters
    string getCccd() const { return cccd; }
    string getDriverLicense() const { return driverLicense; }
    double getBalance() const { return balance; }

    // Setters
    void setCccd(const string& cccdVal) { cccd = cccdVal; }
    void setDriverLicense(const string& licenseVal) { driverLicense = licenseVal; }
    void setBalance(double balanceVal) { balance = balanceVal; }
};

#endif // CUSTOMER_H
