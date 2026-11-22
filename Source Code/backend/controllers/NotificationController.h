#ifndef NOTIFICATION_CONTROLLER_H
#define NOTIFICATION_CONTROLLER_H

#include <string>
#include "../database/DatabaseConnection.h"

using namespace std;

// Controller handling system notification dispatch and retrieval
class NotificationController {
private:
    DatabaseConnection* db;

public:
    // Constructor
    NotificationController(DatabaseConnection* dbConnection);

    // Send a notification to customer
    void sendNotification(int cusId, int orderId, const string& msg);

    // Get notifications for customer as JSON
    string getNotifications(int cusId);

    // Mark notifications as read
    string markAsRead(int cusId);
};

#endif // NOTIFICATION_CONTROLLER_H
