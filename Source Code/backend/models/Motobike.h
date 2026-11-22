#ifndef MOTOBIKE_H
#define MOTOBIKE_H

#include "Vehicle.h"

// Motobike class inheriting from abstract Vehicle
class Motobike : public Vehicle {
private:
    int engineCapacity;

public:
    // Constructor
    Motobike(int vehicleId, const string& brandVal, const string& modelVal,
             const string& plateVal, double priceVal, bool availableVal, int capacityVal)
        : Vehicle(vehicleId, brandVal, modelVal, plateVal, priceVal, availableVal, "Motobike"),
          engineCapacity(capacityVal) {}

    // Method to display motorcycle details
    void displayVehicle() override {
        cout << "[Motobike] ID: " << id
             << " | " << brand << " " << model
             << " | Plate: " << licensePlate
             << " | Engine Capacity: " << engineCapacity << " cc"
             << " | Price/Day: " << pricePerDay
             << " | Available: " << (isAvailable ? "Yes" : "No") << endl;
    }

    // Getter and setter for engine capacity
    int getEngineCapacity() const { return engineCapacity; }
    void setEngineCapacity(int capacityVal) { engineCapacity = capacityVal; }
};

#endif // MOTOBIKE_H
