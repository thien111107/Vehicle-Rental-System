#ifndef RENTAL_ORDER_H
#define RENTAL_ORDER_H

#include <string>
#include <iostream>
using namespace std;

// Class representing a vehicle rental order
class RentalOrder {
private:
    int orderId;
    int customerId;
    int vehicleId;
    string cccd;
    string driverLicense;
    string startDate;
    string endDate;
    double totalAmount;
    string status;

public:
    // Default constructor
    RentalOrder()
        : orderId(0), customerId(0), vehicleId(0), totalAmount(0.0), status("Pending") {}

    // Parameterized constructor
    RentalOrder(int ordId, int cusId, int vehId, const string& cccdVal,
                const string& licenseVal, const string& startVal,
                const string& endVal, double totalVal, const string& statusVal = "Pending")
        : orderId(ordId), customerId(cusId), vehicleId(vehId), cccd(cccdVal),
          driverLicense(licenseVal), startDate(startVal), endDate(endVal),
          totalAmount(totalVal), status(statusVal) {}

    // Method to display rental order summary
    void displayOrder() const {
        cout << "[RentalOrder #" << orderId << "] "
             << "Customer: " << customerId
             << " | Vehicle: " << vehicleId
             << " | Period: " << startDate << " to " << endDate
             << " | Total: " << totalAmount
             << " | Status: " << status << endl;
    }

    // Getters
    int getOrderId() const { return orderId; }
    int getCustomerId() const { return customerId; }
    int getVehicleId() const { return vehicleId; }
    string getCccd() const { return cccd; }
    string getDriverLicense() const { return driverLicense; }
    string getStartDate() const { return startDate; }
    string getEndDate() const { return endDate; }
    double getTotalAmount() const { return totalAmount; }
    string getStatus() const { return status; }

    // Setters
    void setOrderId(int idVal) { orderId = idVal; }
    void setCustomerId(int idVal) { customerId = idVal; }
    void setVehicleId(int idVal) { vehicleId = idVal; }
    void setCccd(const string& cccdVal) { cccd = cccdVal; }
    void setDriverLicense(const string& licenseVal) { driverLicense = licenseVal; }
    void setStartDate(const string& dateVal) { startDate = dateVal; }
    void setEndDate(const string& dateVal) { endDate = dateVal; }
    void setTotalAmount(double amountVal) { totalAmount = amountVal; }
    void setStatus(const string& statusVal) { status = statusVal; }
};

#endif // RENTAL_ORDER_H
