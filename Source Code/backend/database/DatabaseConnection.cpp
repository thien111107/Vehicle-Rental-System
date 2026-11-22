#include "DatabaseConnection.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

// Initialize static singleton instance pointer
DatabaseConnection* DatabaseConnection::instance = nullptr;

// Helper to find string value between quotes in simple JSON format
static string readJsonString(const string& text, const string& key) {
    string search = "\"" + key + "\"";
    size_t pos = text.find(search);
    if (pos == string::npos) return "";

    size_t colon = text.find(':', pos + search.length());
    if (colon == string::npos) return "";

    size_t q1 = text.find('"', colon + 1);
    if (q1 == string::npos) return "";

    size_t q2 = text.find('"', q1 + 1);
    if (q2 == string::npos) return "";

    return text.substr(q1 + 1, q2 - q1 - 1);
}

// Helper to find numeric value in simple JSON format
static double readJsonNumber(const string& text, const string& key, double defaultValue = 0.0) {
    string search = "\"" + key + "\"";
    size_t pos = text.find(search);
    if (pos == string::npos) return defaultValue;

    size_t colon = text.find(':', pos + search.length());
    if (colon == string::npos) return defaultValue;

    size_t start = colon + 1;
    while (start < text.length() && (text[start] == ' ' || text[start] == '\t' || text[start] == '"')) {
        start++;
    }

    size_t end = start;
    while (end < text.length() && (isdigit(text[end]) || text[end] == '.' || text[end] == '-' || text[end] == 'e' || text[end] == 'E' || text[end] == '+')) {
        end++;
    }

    if (end > start) {
        try {
            return stod(text.substr(start, end - start));
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

// Helper to read boolean value from JSON
static bool readJsonBool(const string& text, const string& key, bool defaultValue = false) {
    string search = "\"" + key + "\"";
    size_t pos = text.find(search);
    if (pos == string::npos) return defaultValue;

    size_t colon = text.find(':', pos + search.length());
    if (colon == string::npos) return defaultValue;

    size_t start = colon + 1;
    while (start < text.length() && (text[start] == ' ' || text[start] == '\t')) {
        start++;
    }

    if (text.compare(start, 4, "true") == 0 || text.compare(start, 1, "1") == 0) return true;
    if (text.compare(start, 5, "false") == 0 || text.compare(start, 1, "0") == 0) return false;
    return defaultValue;
}

// Helper to split JSON array into individual object strings
static vector<string> splitJsonArray(const string& json, const string& arrayName) {
    vector<string> objects;
    string search = "\"" + arrayName + "\"";
    size_t pos = json.find(search);
    if (pos == string::npos) return objects;

    size_t openBracket = json.find('[', pos);
    if (openBracket == string::npos) return objects;

    size_t i = openBracket + 1;
    while (i < json.length()) {
        while (i < json.length() && json[i] != '{' && json[i] != ']') {
            i++;
        }
        if (i >= json.length() || json[i] == ']') break;

        size_t start = i;
        int depth = 0;
        bool inQuotes = false;

        while (i < json.length()) {
            if (json[i] == '"' && (i == 0 || json[i - 1] != '\\')) {
                inQuotes = !inQuotes;
            } else if (!inQuotes) {
                if (json[i] == '{') depth++;
                else if (json[i] == '}') {
                    depth--;
                    if (depth == 0) {
                        objects.push_back(json.substr(start, i - start + 1));
                        i++;
                        break;
                    }
                }
            }
            i++;
        }
    }
    return objects;
}

// Private constructor
DatabaseConnection::DatabaseConnection()
    : isConnected(false), dataFilePath("database_store.json"),
      nextUserId(1), nextVehicleId(1), nextOrderId(1), nextNotificationId(1) {
    connect();
}

// Destructor
DatabaseConnection::~DatabaseConnection() {
    disconnect();
    // Clean up dynamic user pointers
    for (size_t i = 0; i < usersTable.size(); i++) {
        delete usersTable[i];
    }
    usersTable.clear();

    // Clean up dynamic vehicle pointers
    for (size_t i = 0; i < vehiclesTable.size(); i++) {
        delete vehiclesTable[i];
    }
    vehiclesTable.clear();
}

// Singleton getInstance method
DatabaseConnection* DatabaseConnection::getInstance() {
    if (instance == nullptr) {
        instance = new DatabaseConnection();
    }
    return instance;
}

// Connect to database and load data from file
bool DatabaseConnection::connect() {
    if (isConnected) return true;

    loadFromFile();
    if (usersTable.empty() && vehiclesTable.empty()) {
        initializeSeedData();
        saveToFile();
    }

    isConnected = true;
    cout << "[DatabaseConnection] Successfully loaded data from file: " << dataFilePath << endl;
    return true;
}

// Disconnect from database and save file
void DatabaseConnection::disconnect() {
    if (!isConnected) return;
    saveToFile();
    isConnected = false;
    cout << "[DatabaseConnection] Saved and disconnected." << endl;
}

// Create default demo data on first start
void DatabaseConnection::initializeSeedData() {
    usersTable.clear();
    vehiclesTable.clear();
    rentalOrdersTable.clear();
    notificationsTable.clear();

    // 1. Initial users
    usersTable.push_back(new Admin(1, "admin", "admin123"));
    usersTable.push_back(new Manager(2, "manager1", "manager123"));
    usersTable.push_back(new Customer(3, "customer1", "customer123", "001200008888", "B2-888999", 5000000.0));
    nextUserId = 4;

    // 2. Initial vehicles (Cars and Motorbikes)
    vehiclesTable.push_back(new Car(1, "Toyota", "Camry 2.5Q", "30A-999.88", 900000.0, true, 5));
    vehiclesTable.push_back(new Car(2, "Mazda", "CX-5 Premium", "29B-555.22", 850000.0, true, 5));
    vehiclesTable.push_back(new Car(3, "Hyundai", "SantaFe 7 Seats", "30F-123.45", 1200000.0, true, 7));

    vehiclesTable.push_back(new Motobike(4, "Honda", "SH 150i ABS", "29G1-888.88", 250000.0, true, 150));
    vehiclesTable.push_back(new Motobike(5, "Yamaha", "Exciter 155 VVA", "30H2-666.99", 150000.0, true, 155));
    vehiclesTable.push_back(new Motobike(6, "Honda", "Winner X", "29K1-333.11", 140000.0, true, 150));
    nextVehicleId = 7;

    // 3. Initial sample order
    RentalOrder sampleOrder(1, 3, 1, "001200008888", "B2-888999", "2026-11-01", "2026-11-04", 2700000.0, "Approved");
    rentalOrdersTable.push_back(sampleOrder);
    nextOrderId = 2;

    // 4. Initial sample notification
    notificationsTable.push_back(Notification(1, 3, 1, "Your rental order #1 for vehicle Toyota Camry 2.5Q has been APPROVED!", true));
    nextNotificationId = 2;
}

// Save all in-memory data to JSON file
void DatabaseConnection::saveToFile() {
    ofstream file(dataFilePath);
    if (!file.is_open()) return;

    file << "{\n";

    // 1. Save users
    file << "  \"users\": [\n";
    for (size_t i = 0; i < usersTable.size(); i++) {
        User* u = usersTable[i];
        file << "    {\"id\": " << u->getId()
             << ", \"username\": \"" << u->getUsername() << "\""
             << ", \"password\": \"" << u->getPassword() << "\""
             << ", \"role\": \"" << u->getRole() << "\"";

        if (u->getRole() == "Customer") {
            Customer* cus = (Customer*)u;
            file << ", \"cccd\": \"" << cus->getCccd() << "\""
                 << ", \"driverLicense\": \"" << cus->getDriverLicense() << "\""
                 << ", \"balance\": " << (long long)cus->getBalance();
        }
        file << "}" << (i + 1 < usersTable.size() ? "," : "") << "\n";
    }
    file << "  ],\n";

    // 2. Save vehicles
    file << "  \"vehicles\": [\n";
    for (size_t i = 0; i < vehiclesTable.size(); i++) {
        Vehicle* v = vehiclesTable[i];
        file << "    {\"id\": " << v->getId()
             << ", \"brand\": \"" << v->getBrand() << "\""
             << ", \"model\": \"" << v->getModel() << "\""
             << ", \"licensePlate\": \"" << v->getLicensePlate() << "\""
             << ", \"pricePerDay\": " << (long long)v->getPricePerDay()
             << ", \"isAvailable\": " << (v->getIsAvailable() ? "true" : "false")
             << ", \"type\": \"" << v->getType() << "\"";

        if (v->getType() == "Car") {
            Car* c = (Car*)v;
            file << ", \"numSeats\": " << c->getNumSeats();
        } else {
            Motobike* m = (Motobike*)v;
            file << ", \"engineCapacity\": " << m->getEngineCapacity();
        }
        file << "}" << (i + 1 < vehiclesTable.size() ? "," : "") << "\n";
    }
    file << "  ],\n";

    // 3. Save rental orders
    file << "  \"rentalOrders\": [\n";
    for (size_t i = 0; i < rentalOrdersTable.size(); i++) {
        RentalOrder& o = rentalOrdersTable[i];
        file << "    {\"orderId\": " << o.getOrderId()
             << ", \"customerId\": " << o.getCustomerId()
             << ", \"vehicleId\": " << o.getVehicleId()
             << ", \"cccd\": \"" << o.getCccd() << "\""
             << ", \"driverLicense\": \"" << o.getDriverLicense() << "\""
             << ", \"startDate\": \"" << o.getStartDate() << "\""
             << ", \"endDate\": \"" << o.getEndDate() << "\""
             << ", \"totalAmount\": " << (long long)o.getTotalAmount()
             << ", \"status\": \"" << o.getStatus() << "\"}"
             << (i + 1 < rentalOrdersTable.size() ? "," : "") << "\n";
    }
    file << "  ],\n";

    // 4. Save notifications
    file << "  \"notifications\": [\n";
    for (size_t i = 0; i < notificationsTable.size(); i++) {
        Notification& n = notificationsTable[i];
        file << "    {\"id\": " << n.getId()
             << ", \"customerId\": " << n.getCustomerId()
             << ", \"orderId\": " << n.getOrderId()
             << ", \"message\": \"" << n.getMessage() << "\""
             << ", \"isRead\": " << (n.getIsRead() ? "true" : "false") << "}"
             << (i + 1 < notificationsTable.size() ? "," : "") << "\n";
    }
    file << "  ]\n";

    file << "}\n";
    file.close();
}

// Load data from JSON file into in-memory vectors
void DatabaseConnection::loadFromFile() {
    ifstream file(dataFilePath);
    if (!file.is_open()) return;

    stringstream buffer;
    buffer << file.rdbuf();
    string content = buffer.str();
    file.close();

    if (content.find("\"users\"") == string::npos) return;

    // Clear old data
    for (size_t i = 0; i < usersTable.size(); i++) delete usersTable[i];
    usersTable.clear();
    for (size_t i = 0; i < vehiclesTable.size(); i++) delete vehiclesTable[i];
    vehiclesTable.clear();
    rentalOrdersTable.clear();
    notificationsTable.clear();

    // 1. Load users
    vector<string> userItems = splitJsonArray(content, "users");
    for (size_t i = 0; i < userItems.size(); i++) {
        int id = (int)readJsonNumber(userItems[i], "id");
        string username = readJsonString(userItems[i], "username");
        string password = readJsonString(userItems[i], "password");
        string role = readJsonString(userItems[i], "role");

        if (role == "Admin") {
            usersTable.push_back(new Admin(id, username, password));
        } else if (role == "Manager") {
            usersTable.push_back(new Manager(id, username, password));
        } else {
            string cccd = readJsonString(userItems[i], "cccd");
            string license = readJsonString(userItems[i], "driverLicense");
            double balance = readJsonNumber(userItems[i], "balance", 0.0);
            if (balance > 0.0 && balance < 1000.0) balance *= 1000000.0;
            usersTable.push_back(new Customer(id, username, password, cccd, license, balance));
        }
        if (id >= nextUserId) nextUserId = id + 1;
    }

    // 2. Load vehicles
    vector<string> vehItems = splitJsonArray(content, "vehicles");
    for (size_t i = 0; i < vehItems.size(); i++) {
        int id = (int)readJsonNumber(vehItems[i], "id");
        string brand = readJsonString(vehItems[i], "brand");
        string model = readJsonString(vehItems[i], "model");
        string plate = readJsonString(vehItems[i], "licensePlate");
        double price = readJsonNumber(vehItems[i], "pricePerDay");
        if (price > 0.0 && price < 1000.0) price *= 1000000.0;
        bool isAvail = readJsonBool(vehItems[i], "isAvailable", true);
        string type = readJsonString(vehItems[i], "type");

        if (type == "Car") {
            int seats = (int)readJsonNumber(vehItems[i], "numSeats", 5);
            vehiclesTable.push_back(new Car(id, brand, model, plate, price, isAvail, seats));
        } else {
            int cc = (int)readJsonNumber(vehItems[i], "engineCapacity", 150);
            vehiclesTable.push_back(new Motobike(id, brand, model, plate, price, isAvail, cc));
        }
        if (id >= nextVehicleId) nextVehicleId = id + 1;
    }

    // 3. Load rental orders
    vector<string> orderItems = splitJsonArray(content, "rentalOrders");
    for (size_t i = 0; i < orderItems.size(); i++) {
        int orderId = (int)readJsonNumber(orderItems[i], "orderId");
        int customerId = (int)readJsonNumber(orderItems[i], "customerId");
        int vehicleId = (int)readJsonNumber(orderItems[i], "vehicleId");
        string cccd = readJsonString(orderItems[i], "cccd");
        string license = readJsonString(orderItems[i], "driverLicense");
        string start = readJsonString(orderItems[i], "startDate");
        string end = readJsonString(orderItems[i], "endDate");
        double total = readJsonNumber(orderItems[i], "totalAmount");
        if (total > 0.0 && total < 1000.0) total *= 1000000.0;
        string status = readJsonString(orderItems[i], "status");

        RentalOrder order(orderId, customerId, vehicleId, cccd, license, start, end, total, status);
        rentalOrdersTable.push_back(order);
        if (orderId >= nextOrderId) nextOrderId = orderId + 1;
    }

    // 4. Load notifications
    vector<string> notifItems = splitJsonArray(content, "notifications");
    for (size_t i = 0; i < notifItems.size(); i++) {
        int id = (int)readJsonNumber(notifItems[i], "id");
        int customerId = (int)readJsonNumber(notifItems[i], "customerId");
        int orderId = (int)readJsonNumber(notifItems[i], "orderId");
        string msg = readJsonString(notifItems[i], "message");
        bool isRead = readJsonBool(notifItems[i], "isRead", false);

        notificationsTable.push_back(Notification(id, customerId, orderId, msg, isRead));
        if (id >= nextNotificationId) nextNotificationId = id + 1;
    }
}

// User operations
User* DatabaseConnection::findUserByUsername(const string& username) {
    for (size_t i = 0; i < usersTable.size(); i++) {
        if (usersTable[i]->getUsername() == username) {
            return usersTable[i];
        }
    }
    return nullptr;
}

User* DatabaseConnection::findUserById(int userId) {
    for (size_t i = 0; i < usersTable.size(); i++) {
        if (usersTable[i]->getId() == userId) {
            return usersTable[i];
        }
    }
    return nullptr;
}

bool DatabaseConnection::insertCustomer(const string& username, const string& password, int& outId) {
    if (findUserByUsername(username) != nullptr) {
        return false;
    }

    outId = nextUserId++;
    Customer* newCustomer = new Customer(outId, username, password);
    usersTable.push_back(newCustomer);
    saveToFile();
    return true;
}

bool DatabaseConnection::insertManager(const string& username, const string& password, int& outId) {
    if (findUserByUsername(username) != nullptr) {
        return false;
    }

    outId = nextUserId++;
    Manager* newManager = new Manager(outId, username, password);
    usersTable.push_back(newManager);
    saveToFile();
    return true;
}

bool DatabaseConnection::deleteManager(int managerId) {
    for (size_t i = 0; i < usersTable.size(); i++) {
        if (usersTable[i]->getId() == managerId && usersTable[i]->getRole() == "Manager") {
            delete usersTable[i];
            usersTable.erase(usersTable.begin() + i);
            saveToFile();
            return true;
        }
    }
    return false;
}

vector<Manager*> DatabaseConnection::getAllManagers() {
    vector<Manager*> managers;
    for (size_t i = 0; i < usersTable.size(); i++) {
        if (usersTable[i]->getRole() == "Manager") {
            managers.push_back((Manager*)usersTable[i]);
        }
    }
    return managers;
}

bool DatabaseConnection::updateCustomerCredentials(int customerId, const string& cccd, const string& driverLicense) {
    for (size_t i = 0; i < usersTable.size(); i++) {
        if (usersTable[i]->getId() == customerId && usersTable[i]->getRole() == "Customer") {
            Customer* cus = (Customer*)usersTable[i];
            cus->setCccd(cccd);
            cus->setDriverLicense(driverLicense);
            saveToFile();
            return true;
        }
    }
    return false;
}

// Vehicle operations
vector<Vehicle*> DatabaseConnection::getVehicles(const string& typeFilter, bool onlyAvailable) {
    vector<Vehicle*> result;
    for (size_t i = 0; i < vehiclesTable.size(); i++) {
        Vehicle* v = vehiclesTable[i];
        if (onlyAvailable && !v->getIsAvailable()) continue;
        if (!typeFilter.empty() && typeFilter != "All" && v->getType() != typeFilter) continue;
        result.push_back(v);
    }
    return result;
}

Vehicle* DatabaseConnection::findVehicleById(int vehicleId) {
    for (size_t i = 0; i < vehiclesTable.size(); i++) {
        if (vehiclesTable[i]->getId() == vehicleId) {
            return vehiclesTable[i];
        }
    }
    return nullptr;
}

bool DatabaseConnection::insertVehicle(Vehicle* vehicle, int& outId) {
    if (!vehicle) return false;
    outId = nextVehicleId++;
    vehicle->setId(outId);
    vehiclesTable.push_back(vehicle);
    saveToFile();
    return true;
}

bool DatabaseConnection::updateVehicle(Vehicle* vehicle) {
    if (!vehicle) return false;
    for (size_t i = 0; i < vehiclesTable.size(); i++) {
        if (vehiclesTable[i]->getId() == vehicle->getId()) {
            delete vehiclesTable[i];
            vehiclesTable[i] = vehicle;
            saveToFile();
            return true;
        }
    }
    return false;
}

bool DatabaseConnection::deleteVehicle(int vehicleId) {
    for (size_t i = 0; i < vehiclesTable.size(); i++) {
        if (vehiclesTable[i]->getId() == vehicleId) {
            delete vehiclesTable[i];
            vehiclesTable.erase(vehiclesTable.begin() + i);
            saveToFile();
            return true;
        }
    }
    return false;
}

void DatabaseConnection::countVehicles(int& outTotal, int& outAvailableCars, int& outAvailableMotos) {
    outTotal = (int)vehiclesTable.size();
    outAvailableCars = 0;
    outAvailableMotos = 0;

    for (size_t i = 0; i < vehiclesTable.size(); i++) {
        if (vehiclesTable[i]->getIsAvailable()) {
            if (vehiclesTable[i]->getType() == "Car") outAvailableCars++;
            else if (vehiclesTable[i]->getType() == "Motobike") outAvailableMotos++;
        }
    }
}

// Rental order operations
bool DatabaseConnection::insertRentalOrder(const RentalOrder& order, int& outOrderId) {
    outOrderId = nextOrderId++;
    RentalOrder newOrder = order;
    newOrder.setOrderId(outOrderId);
    rentalOrdersTable.push_back(newOrder);

    // Mark vehicle as rented/reserved
    for (size_t i = 0; i < vehiclesTable.size(); i++) {
        if (vehiclesTable[i]->getId() == order.getVehicleId()) {
            vehiclesTable[i]->setIsAvailable(false);
            break;
        }
    }

    saveToFile();
    return true;
}

RentalOrder* DatabaseConnection::findOrderById(int orderId) {
    for (size_t i = 0; i < rentalOrdersTable.size(); i++) {
        if (rentalOrdersTable[i].getOrderId() == orderId) {
            return &rentalOrdersTable[i];
        }
    }
    return nullptr;
}

bool DatabaseConnection::updateOrderStatus(int orderId, const string& newStatus) {
    for (size_t i = 0; i < rentalOrdersTable.size(); i++) {
        if (rentalOrdersTable[i].getOrderId() == orderId) {
            rentalOrdersTable[i].setStatus(newStatus);

            // If order was rejected or completed, release vehicle back to available
            if (newStatus == "Rejected" || newStatus == "Completed") {
                for (size_t j = 0; j < vehiclesTable.size(); j++) {
                    if (vehiclesTable[j]->getId() == rentalOrdersTable[i].getVehicleId()) {
                        vehiclesTable[j]->setIsAvailable(true);
                        break;
                    }
                }
            } else if (newStatus == "Approved") {
                for (size_t j = 0; j < vehiclesTable.size(); j++) {
                    if (vehiclesTable[j]->getId() == rentalOrdersTable[i].getVehicleId()) {
                        vehiclesTable[j]->setIsAvailable(false);
                        break;
                    }
                }
            }

            saveToFile();
            return true;
        }
    }
    return false;
}

vector<RentalOrder> DatabaseConnection::getPendingOrders() {
    vector<RentalOrder> pending;
    for (size_t i = 0; i < rentalOrdersTable.size(); i++) {
        if (rentalOrdersTable[i].getStatus() == "Pending") {
            pending.push_back(rentalOrdersTable[i]);
        }
    }
    return pending;
}

vector<RentalOrder> DatabaseConnection::getOrdersByCustomer(int customerId) {
    vector<RentalOrder> customerOrders;
    for (size_t i = 0; i < rentalOrdersTable.size(); i++) {
        if (rentalOrdersTable[i].getCustomerId() == customerId) {
            customerOrders.push_back(rentalOrdersTable[i]);
        }
    }
    return customerOrders;
}

vector<RentalOrder> DatabaseConnection::getApprovedOrCompletedOrders() {
    vector<RentalOrder> list;
    for (size_t i = 0; i < rentalOrdersTable.size(); i++) {
        if (rentalOrdersTable[i].getStatus() == "Approved" || rentalOrdersTable[i].getStatus() == "Completed") {
            list.push_back(rentalOrdersTable[i]);
        }
    }
    return list;
}

// Notification operations
bool DatabaseConnection::insertNotification(int customerId, int orderId, const string& message) {
    int newId = nextNotificationId++;
    Notification notif(newId, customerId, orderId, message, false);
    notificationsTable.push_back(notif);
    saveToFile();
    return true;
}

vector<Notification> DatabaseConnection::getNotificationsByCustomer(int customerId) {
    vector<Notification> list;
    for (int i = (int)notificationsTable.size() - 1; i >= 0; i--) {
        if (notificationsTable[i].getCustomerId() == customerId) {
            list.push_back(notificationsTable[i]);
        }
    }
    return list;
}

bool DatabaseConnection::markNotificationsAsRead(int customerId) {
    for (size_t i = 0; i < notificationsTable.size(); i++) {
        if (notificationsTable[i].getCustomerId() == customerId) {
            notificationsTable[i].markAsRead();
        }
    }
    saveToFile();
    return true;
}
