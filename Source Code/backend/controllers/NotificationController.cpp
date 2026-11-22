#include "NotificationController.h"
#include <sstream>

using namespace std;

// Constructor
NotificationController::NotificationController(DatabaseConnection* dbConnection)
    : db(dbConnection) {}

// Send notification to customer
void NotificationController::sendNotification(int cusId, int orderId, const string& msg) {
    if (db) {
        db->insertNotification(cusId, orderId, msg);
    }
}

// Retrieve notifications for a customer
string NotificationController::getNotifications(int cusId) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection unavailable.\"}";
    }

    vector<Notification> notificationsList = db->getNotificationsByCustomer(cusId);
    ostringstream json;
    json << "{\"success\": true, \"data\": [";

    for (size_t i = 0; i < notificationsList.size(); i++) {
        Notification& n = notificationsList[i];
        json << "{"
             << "\"id\": " << n.getId() << ", "
             << "\"customerId\": " << n.getCustomerId() << ", "
             << "\"orderId\": " << n.getOrderId() << ", "
             << "\"message\": \"" << n.getMessage() << "\", "
             << "\"isRead\": " << (n.getIsRead() ? "true" : "false")
             << "}" << (i + 1 < notificationsList.size() ? ", " : "");
    }

    json << "]}";
    return json.str();
}

// Mark notifications as read
string NotificationController::markAsRead(int cusId) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection unavailable.\"}";
    }
    db->markNotificationsAsRead(cusId);
    return "{\"success\": true, \"message\": \"Notifications marked as read.\"}";
}
