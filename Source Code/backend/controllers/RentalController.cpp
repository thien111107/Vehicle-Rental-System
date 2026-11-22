#include "RentalController.h"
#include <sstream>

using namespace std;

// Constructor
RentalController::RentalController(DatabaseConnection* dbConnection, NotificationController* notificationController)
    : db(dbConnection), notifCtrl(notificationController) {}

// Simple helper to calculate days between two YYYY-MM-DD dates
static int calculateDays(const string& start, const string& end) {
    int y1 = 0, m1 = 0, d1 = 0;
    int y2 = 0, m2 = 0, d2 = 0;
    char sep;
    istringstream s1(start);
    s1 >> y1 >> sep >> m1 >> sep >> d1;
    istringstream s2(end);
    s2 >> y2 >> sep >> m2 >> sep >> d2;

    int days1 = y1 * 365 + m1 * 30 + d1;
    int days2 = y2 * 365 + m2 * 30 + d2;
    int diff = days2 - days1;
    return (diff <= 0) ? 1 : diff;
}

// Create a rental request
string RentalController::createRental(int cusId, int vehId, const string& cccd,
                                      const string& license, const string& start,
                                      const string& end) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection not available!\"}";
    }

    if (cccd.empty() || license.empty() || start.empty() || end.empty()) {
        return "{\"success\": false, \"message\": \"Customer must provide CCCD, Driver License, start and end dates!\"}";
    }

    Vehicle* vehicle = db->findVehicleById(vehId);
    if (!vehicle) {
        return "{\"success\": false, \"message\": \"Requested vehicle does not exist!\"}";
    }

    if (!vehicle->getIsAvailable()) {
        return "{\"success\": false, \"message\": \"Vehicle is currently rented or reserved!\"}";
    }

    int rentalDays = calculateDays(start, end);
    double totalAmount = rentalDays * vehicle->getPricePerDay();

    // Save CCCD and license to customer profile
    db->updateCustomerCredentials(cusId, cccd, license);

    // Create rental order
    RentalOrder newOrder(0, cusId, vehId, cccd, license, start, end, totalAmount, "Pending");
    int createdOrderId = 0;
    bool success = db->insertRentalOrder(newOrder, createdOrderId);

    if (success) {
        if (notifCtrl) {
            string msg = "Your rental booking request #" + to_string(createdOrderId) +
                         " for " + vehicle->getBrand() + " " + vehicle->getModel() +
                         " is currently PENDING approval from the manager.";
            notifCtrl->sendNotification(cusId, createdOrderId, msg);
        }

        ostringstream json;
        json << "{\"success\": true, \"message\": \"Rental request submitted successfully!\", \"orderId\": "
             << createdOrderId << ", \"totalAmount\": " << (long long)totalAmount << "}";
        return json.str();
    }

    return "{\"success\": false, \"message\": \"Could not create rental order!\"}";
}

// Get pending rental orders
string RentalController::getPendingOrders() {
    if (!db) return "{\"success\": false, \"data\": []}";

    vector<RentalOrder> pendingList = db->getPendingOrders();
    ostringstream json;
    json << "{\"success\": true, \"data\": [";

    for (size_t i = 0; i < pendingList.size(); i++) {
        RentalOrder& o = pendingList[i];
        User* user = db->findUserById(o.getCustomerId());
        Vehicle* veh = db->findVehicleById(o.getVehicleId());

        string customerName = user ? user->getUsername() : "Unknown";
        string brand = veh ? veh->getBrand() : "Unknown";
        string model = veh ? veh->getModel() : "Vehicle";
        string plate = veh ? veh->getLicensePlate() : "N/A";
        string vehType = veh ? veh->getType() : "Car";

        json << "{"
             << "\"orderID\": " << o.getOrderId() << ", "
             << "\"orderId\": " << o.getOrderId() << ", "
             << "\"customerID\": " << o.getCustomerId() << ", "
             << "\"customerId\": " << o.getCustomerId() << ", "
             << "\"customerUsername\": \"" << customerName << "\", "
             << "\"customerName\": \"" << customerName << "\", "
             << "\"vehicleID\": " << o.getVehicleId() << ", "
             << "\"vehicleId\": " << o.getVehicleId() << ", "
             << "\"vehicleBrand\": \"" << brand << "\", "
             << "\"vehicleModel\": \"" << model << "\", "
             << "\"licensePlate\": \"" << plate << "\", "
             << "\"vehicleType\": \"" << vehType << "\", "
             << "\"cccd\": \"" << o.getCccd() << "\", "
             << "\"driverLicense\": \"" << o.getDriverLicense() << "\", "
             << "\"startDate\": \"" << o.getStartDate() << "\", "
             << "\"endDate\": \"" << o.getEndDate() << "\", "
             << "\"totalAmount\": " << (long long)o.getTotalAmount() << ", "
             << "\"status\": \"" << o.getStatus() << "\""
             << "}" << (i + 1 < pendingList.size() ? ", " : "");
    }

    json << "]}";
    return json.str();
}

// Approve rental order
string RentalController::approveRental(int orderId) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection not available!\"}";
    }

    RentalOrder* order = db->findOrderById(orderId);
    if (!order) {
        return "{\"success\": false, \"message\": \"Rental order #" + to_string(orderId) + " not found!\"}";
    }

    bool updated = db->updateOrderStatus(orderId, "Approved");
    if (updated) {
        if (notifCtrl) {
            Vehicle* veh = db->findVehicleById(order->getVehicleId());
            string vehName = veh ? (veh->getBrand() + " " + veh->getModel()) : "vehicle";
            string msg = "Congratulations! Your rental order #" + to_string(orderId) +
                         " for " + vehName + " has been APPROVED! You may pick up your vehicle now.";
            notifCtrl->sendNotification(order->getCustomerId(), orderId, msg);
        }

        return "{\"success\": true, \"message\": \"Rental request #" + to_string(orderId) + " has been approved!\"}";
    }

    return "{\"success\": false, \"message\": \"Failed to approve order!\"}";
}

// Reject rental order
string RentalController::rejectRental(int orderId) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection not available!\"}";
    }

    RentalOrder* order = db->findOrderById(orderId);
    if (!order) {
        return "{\"success\": false, \"message\": \"Rental order #" + to_string(orderId) + " not found!\"}";
    }

    bool updated = db->updateOrderStatus(orderId, "Rejected");
    if (updated) {
        if (notifCtrl) {
            Vehicle* veh = db->findVehicleById(order->getVehicleId());
            string vehName = veh ? (veh->getBrand() + " " + veh->getModel()) : "vehicle";
            string msg = "Your rental order #" + to_string(orderId) + " for " + vehName +
                         " was REJECTED by the manager.";
            notifCtrl->sendNotification(order->getCustomerId(), orderId, msg);
        }

        return "{\"success\": true, \"message\": \"Order #" + to_string(orderId) + " rejected.\"}";
    }

    return "{\"success\": false, \"message\": \"Failed to reject order!\"}";
}

// Get customer personal rental history
string RentalController::getCustomerHistory(int cusId) {
    if (!db) return "{\"success\": false, \"data\": []}";

    vector<RentalOrder> historyList = db->getOrdersByCustomer(cusId);
    ostringstream json;
    json << "{\"success\": true, \"data\": [";

    for (size_t i = 0; i < historyList.size(); i++) {
        RentalOrder& o = historyList[i];
        Vehicle* veh = db->findVehicleById(o.getVehicleId());

        string brand = veh ? veh->getBrand() : "Unknown";
        string model = veh ? veh->getModel() : "Vehicle";
        string plate = veh ? veh->getLicensePlate() : "N/A";
        string vehType = veh ? veh->getType() : "Car";

        json << "{"
             << "\"orderID\": " << o.getOrderId() << ", "
             << "\"orderId\": " << o.getOrderId() << ", "
             << "\"vehicleBrand\": \"" << brand << "\", "
             << "\"vehicleModel\": \"" << model << "\", "
             << "\"licensePlate\": \"" << plate << "\", "
             << "\"vehicleType\": \"" << vehType << "\", "
             << "\"startDate\": \"" << o.getStartDate() << "\", "
             << "\"endDate\": \"" << o.getEndDate() << "\", "
             << "\"totalAmount\": " << (long long)o.getTotalAmount() << ", "
             << "\"status\": \"" << o.getStatus() << "\""
             << "}" << (i + 1 < historyList.size() ? ", " : "");
    }

    json << "]}";
    return json.str();
}

// Get list of rented customers
string RentalController::getRentedCustomers() {
    if (!db) return "{\"success\": false, \"data\": []}";

    vector<RentalOrder> list = db->getApprovedOrCompletedOrders();
    ostringstream json;
    json << "{\"success\": true, \"data\": [";

    for (size_t i = 0; i < list.size(); i++) {
        RentalOrder& o = list[i];
        User* user = db->findUserById(o.getCustomerId());
        Vehicle* veh = db->findVehicleById(o.getVehicleId());

        string customerName = user ? user->getUsername() : "Customer";
        string vehInfo = veh ? (veh->getBrand() + " " + veh->getModel()) : "Vehicle";
        string plate = veh ? veh->getLicensePlate() : "N/A";

        json << "{"
             << "\"orderID\": " << o.getOrderId() << ", "
             << "\"orderId\": " << o.getOrderId() << ", "
             << "\"customerID\": " << o.getCustomerId() << ", "
             << "\"customerName\": \"" << customerName << "\", "
             << "\"cccd\": \"" << o.getCccd() << "\", "
             << "\"driverLicense\": \"" << o.getDriverLicense() << "\", "
             << "\"vehicleInfo\": \"" << vehInfo << "\", "
             << "\"licensePlate\": \"" << plate << "\", "
             << "\"startDate\": \"" << o.getStartDate() << "\", "
             << "\"endDate\": \"" << o.getEndDate() << "\", "
             << "\"totalAmount\": " << (long long)o.getTotalAmount() << ", "
             << "\"status\": \"" << o.getStatus() << "\""
             << "}" << (i + 1 < list.size() ? ", " : "");
    }

    json << "]}";
    return json.str();
}

// Process vehicle return
string RentalController::returnRental(int cusId, int orderId) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database connection not available!\"}";
    }

    RentalOrder* order = db->findOrderById(orderId);
    if (!order) {
        return "{\"success\": false, \"message\": \"Order #" + to_string(orderId) + " not found!\"}";
    }

    if (order->getCustomerId() != cusId) {
        return "{\"success\": false, \"message\": \"Unauthorized operation on this rental order!\"}";
    }

    if (order->getStatus() != "Approved") {
        return "{\"success\": false, \"message\": \"Only currently approved active rentals can be returned!\"}";
    }

    bool updated = db->updateOrderStatus(orderId, "Completed");
    if (updated) {
        if (notifCtrl) {
            Vehicle* veh = db->findVehicleById(order->getVehicleId());
            string vehName = veh ? (veh->getBrand() + " " + veh->getModel()) : "vehicle";
            string msg = "Vehicle return confirmed for order #" + to_string(orderId) +
                         " (" + vehName + "). Thank you for choosing our Vehicle Rental service!";
            notifCtrl->sendNotification(cusId, orderId, msg);
        }

        return "{\"success\": true, \"message\": \"Vehicle returned successfully! Thank you for using our service.\"}";
    }
    return "{\"success\": false, \"message\": \"Failed to complete vehicle return!\"}";
}
