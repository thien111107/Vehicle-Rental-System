#ifndef CAR_H
#define CAR_H

#include "Vehicle.h"

// Car class inheriting from abstract Vehicle
class Car : public Vehicle {
private:
    int numSeats;

public:
    // Constructor
    Car(int vehicleId, const string& brandVal, const string& modelVal,
        const string& plateVal, double priceVal, bool availableVal, int seatsVal)
        : Vehicle(vehicleId, brandVal, modelVal, plateVal, priceVal, availableVal, "Car"),
          numSeats(seatsVal) {}

    // Method to display car details
    void displayVehicle() override {
        cout << "[Car] ID: " << id
             << " | " << brand << " " << model
             << " | Plate: " << licensePlate
             << " | Seats: " << numSeats
             << " | Price/Day: " << pricePerDay
             << " | Available: " << (isAvailable ? "Yes" : "No") << endl;
    }

    // Getter and setter for seats
    int getNumSeats() const { return numSeats; }
    void setNumSeats(int seatsVal) { numSeats = seatsVal; }
};

#endif // CAR_H
