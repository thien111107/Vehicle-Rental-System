#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include <string>
#include <vector>
#include "../models/User.h"
#include "../models/Customer.h"
#include "../models/Manager.h"
#include "../models/Admin.h"
#include "../models/Vehicle.h"
#include "../models/Car.h"
#include "../models/Motobike.h"
#include "../models/RentalOrder.h"
#include "../models/Notification.h"

using namespace std;

// Singleton database connection managing file storage and in-memory lists
class DatabaseConnection {
private:
    static DatabaseConnection* instance;

    bool isConnected;
    string dataFilePath;

    // In-memory data lists using standard C++ vectors
    vector<User*> usersTable;
    vector<Vehicle*> vehiclesTable;
    vector<RentalOrder> rentalOrdersTable;
    vector<Notification> notificationsTable;

    // Auto-increment ID counters
    int nextUserId;
    int nextVehicleId;
    int nextOrderId;
    int nextNotificationId;

    // Private constructor for Singleton pattern
    DatabaseConnection();

    // Helper functions for file loading and saving
    void initializeSeedData();
    void saveToFile();
    void loadFromFile();

public:
    // Destructor to clean up allocated pointers
    ~DatabaseConnection();

    // Singleton instance getter
    static DatabaseConnection* getInstance();

    // Connection lifecycle
    bool connect();
    void disconnect();

    // User operations
    User* findUserByUsername(const string& username);
    User* findUserById(int userId);
    bool insertCustomer(const string& username, const string& password, int& outId);
    bool insertManager(const string& username, const string& password, int& outId);
    bool deleteManager(int managerId);
    vector<Manager*> getAllManagers();
    bool updateCustomerCredentials(int customerId, const string& cccd, const string& driverLicense);

    // Vehicle operations
    vector<Vehicle*> getVehicles(const string& typeFilter = "", bool onlyAvailable = false);
    Vehicle* findVehicleById(int vehicleId);
    bool insertVehicle(Vehicle* vehicle, int& outId);
    bool updateVehicle(Vehicle* vehicle);
    bool deleteVehicle(int vehicleId);
    void countVehicles(int& outTotal, int& outAvailableCars, int& outAvailableMotos);

    // Rental order operations
    bool insertRentalOrder(const RentalOrder& order, int& outOrderId);
    RentalOrder* findOrderById(int orderId);
    bool updateOrderStatus(int orderId, const string& newStatus);
    vector<RentalOrder> getPendingOrders();
    vector<RentalOrder> getOrdersByCustomer(int customerId);
    vector<RentalOrder> getApprovedOrCompletedOrders();

    // Notification operations
    bool insertNotification(int customerId, int orderId, const string& message);
    vector<Notification> getNotificationsByCustomer(int customerId);
    bool markNotificationsAsRead(int customerId);
};

#endif // DATABASE_CONNECTION_H
