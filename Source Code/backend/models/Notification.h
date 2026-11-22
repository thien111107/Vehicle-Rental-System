#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include <iostream>
using namespace std;

// Class representing a system notification sent to a customer
class Notification {
private:
    int id;
    int customerId;
    int orderId;
    string message;
    bool isRead;

public:
    // Default constructor
    Notification() : id(0), customerId(0), orderId(0), message(""), isRead(false) {}

    // Parameterized constructor
    Notification(int notifId, int cusId, int ordId, const string& msgVal, bool readVal = false)
        : id(notifId), customerId(cusId), orderId(ordId), message(msgVal), isRead(readVal) {}

    // Method to display notification details
    void displayNotification() const {
        cout << "[Notification #" << id << "] "
             << "To Customer: " << customerId
             << " | Order: " << orderId
             << " | Status: " << (isRead ? "Read" : "Unread")
             << " | Content: " << message << endl;
    }

    // Method to mark notification as read
    void markAsRead() {
        isRead = true;
    }

    // Getters
    int getId() const { return id; }
    int getCustomerId() const { return customerId; }
    int getOrderId() const { return orderId; }
    string getMessage() const { return message; }
    bool getIsRead() const { return isRead; }

    // Setters
    void setId(int idVal) { id = idVal; }
    void setCustomerId(int cusIdVal) { customerId = cusIdVal; }
    void setOrderId(int ordIdVal) { orderId = ordIdVal; }
    void setMessage(const string& msgVal) { message = msgVal; }
    void setIsRead(bool readVal) { isRead = readVal; }
};

#endif // NOTIFICATION_H
