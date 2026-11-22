#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>
#include <iostream>
using namespace std;

// Abstract base class representing a generic vehicle
class Vehicle {
protected:
    int id;
    string brand;
    string model;
    string licensePlate;
    double pricePerDay;
    bool isAvailable;
    string type;

public:
    // Constructor
    Vehicle(int vehicleId, const string& brandVal, const string& modelVal,
            const string& plateVal, double priceVal, bool availableVal, const string& typeVal)
        : id(vehicleId), brand(brandVal), model(modelVal), licensePlate(plateVal),
          pricePerDay(priceVal), isAvailable(availableVal), type(typeVal) {}

    // Virtual destructor
    virtual ~Vehicle() = default;

    // Pure virtual display method
    virtual void displayVehicle() = 0;

    // Getters
    int getId() const { return id; }
    string getBrand() const { return brand; }
    string getModel() const { return model; }
    string getLicensePlate() const { return licensePlate; }
    double getPricePerDay() const { return pricePerDay; }
    bool getIsAvailable() const { return isAvailable; }
    string getType() const { return type; }

    // Setters
    void setId(int vehicleId) { id = vehicleId; }
    void setBrand(const string& brandVal) { brand = brandVal; }
    void setModel(const string& modelVal) { model = modelVal; }
    void setLicensePlate(const string& plateVal) { licensePlate = plateVal; }
    void setPricePerDay(double priceVal) { pricePerDay = priceVal; }
    void setIsAvailable(bool status) { isAvailable = status; }
    void setType(const string& typeVal) { type = typeVal; }
};

#endif // VEHICLE_H
