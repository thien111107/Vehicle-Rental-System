#ifndef RENTAL_CONTROLLER_H
#define RENTAL_CONTROLLER_H

#include <string>
#include "../database/DatabaseConnection.h"
#include "NotificationController.h"

using namespace std;

// Controller managing vehicle rental reservations, approvals, returns, and history
class RentalController {
private:
    DatabaseConnection* db;
    NotificationController* notifCtrl;

public:
    // Constructor
    RentalController(DatabaseConnection* dbConnection, NotificationController* notificationController);

    // Create a new rental reservation order
    string createRental(int cusId, int vehId, const string& cccd,
                        const string& license, const string& start,
                        const string& end);

    // Retrieve pending rental orders awaiting approval
    string getPendingOrders();

    // Approve a rental order
    string approveRental(int orderId);

    // Reject a rental order
    string rejectRental(int orderId);

    // Get personal rental history for a customer
    string getCustomerHistory(int cusId);

    // Get list of customers who have rented or are renting vehicles
    string getRentedCustomers();

    // Process customer vehicle return
    string returnRental(int cusId, int orderId);
};

#endif // RENTAL_CONTROLLER_H
