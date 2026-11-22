#ifndef VEHICLE_CONTROLLER_H
#define VEHICLE_CONTROLLER_H

#include <string>
#include <vector>
#include "../database/DatabaseConnection.h"
#include "../models/Vehicle.h"
#include "../models/Car.h"
#include "../models/Motobike.h"

using namespace std;

// Controller managing vehicle inventory, availability filtering, and fleet counts
class VehicleController {
private:
    DatabaseConnection* db;

public:
    // Constructor
    VehicleController(DatabaseConnection* dbConnection);

    // Add new vehicle to inventory
    string addVehicle(Vehicle* v);

    // Delete vehicle from inventory
    string deleteVehicle(int id);

    // Update vehicle details
    string updateVehicle(Vehicle* v);

    // Get available vehicles filtered by type
    string getAvailableVehicles(const string& typeFilter);

    // Get all vehicles in inventory
    string getAllVehicles(const string& typeFilter = "");

    // Count fleet inventory
    string countVehicles();
};

#endif // VEHICLE_CONTROLLER_H
