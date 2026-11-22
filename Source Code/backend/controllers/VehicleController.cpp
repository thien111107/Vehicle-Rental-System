#include "VehicleController.h"
#include <sstream>

using namespace std;

// Constructor
VehicleController::VehicleController(DatabaseConnection* dbConnection)
    : db(dbConnection) {}

// Add vehicle
string VehicleController::addVehicle(Vehicle* v) {
    if (!db || !v) {
        return "{\"success\": false, \"message\": \"Database or vehicle data invalid!\"}";
    }

    Vehicle* newVehicle = nullptr;
    if (v->getType() == "Car") {
        Car* car = (Car*)v;
        newVehicle = new Car(0, v->getBrand(), v->getModel(), v->getLicensePlate(),
                             v->getPricePerDay(), true, car->getNumSeats());
    } else {
        Motobike* moto = (Motobike*)v;
        newVehicle = new Motobike(0, v->getBrand(), v->getModel(), v->getLicensePlate(),
                                  v->getPricePerDay(), true, moto->getEngineCapacity());
    }

    int newId = 0;
    bool success = db->insertVehicle(newVehicle, newId);
    if (success) {
        return "{\"success\": true, \"message\": \"Vehicle added successfully to inventory!\", \"id\": " + to_string(newId) + "}";
    }
    delete newVehicle;
    return "{\"success\": false, \"message\": \"Failed to add vehicle.\"}";
}

// Delete vehicle
string VehicleController::deleteVehicle(int id) {
    if (!db) {
        return "{\"success\": false, \"message\": \"Database not available!\"}";
    }

    bool success = db->deleteVehicle(id);
    if (success) {
        return "{\"success\": true, \"message\": \"Vehicle #" + to_string(id) + " deleted successfully!\"}";
    }
    return "{\"success\": false, \"message\": \"Vehicle not found or could not be deleted!\"}";
}

// Update vehicle
string VehicleController::updateVehicle(Vehicle* v) {
    if (!db || !v) {
        return "{\"success\": false, \"message\": \"Database or vehicle data invalid!\"}";
    }

    Vehicle* updatedVehicle = nullptr;
    if (v->getType() == "Car") {
        Car* car = (Car*)v;
        updatedVehicle = new Car(v->getId(), v->getBrand(), v->getModel(), v->getLicensePlate(),
                                 v->getPricePerDay(), v->getIsAvailable(), car->getNumSeats());
    } else {
        Motobike* moto = (Motobike*)v;
        updatedVehicle = new Motobike(v->getId(), v->getBrand(), v->getModel(), v->getLicensePlate(),
                                      v->getPricePerDay(), v->getIsAvailable(), moto->getEngineCapacity());
    }

    bool success = db->updateVehicle(updatedVehicle);
    if (success) {
        return "{\"success\": true, \"message\": \"Vehicle #" + to_string(v->getId()) + " updated successfully!\"}";
    }
    delete updatedVehicle;
    return "{\"success\": false, \"message\": \"Failed to update vehicle.\"}";
}

// Helper to convert vehicle list to JSON string
static string formatVehicleListJson(const vector<Vehicle*>& list) {
    ostringstream json;
    json << "{\"success\": true, \"data\": [";

    for (size_t i = 0; i < list.size(); i++) {
        Vehicle* v = list[i];
        bool isCar = (v->getType() == "Car");
        int numSeats = 0;
        int engineCapacity = 0;

        if (isCar) {
            Car* c = (Car*)v;
            numSeats = c->getNumSeats();
        } else {
            Motobike* m = (Motobike*)v;
            engineCapacity = m->getEngineCapacity();
        }

        json << "{"
             << "\"vehicleID\": " << v->getId() << ", "
             << "\"id\": " << v->getId() << ", "
             << "\"brand\": \"" << v->getBrand() << "\", "
             << "\"model\": \"" << v->getModel() << "\", "
             << "\"licenseplate\": \"" << v->getLicensePlate() << "\", "
             << "\"licensePlate\": \"" << v->getLicensePlate() << "\", "
             << "\"priceperday\": " << (long long)v->getPricePerDay() << ", "
             << "\"pricePerDay\": " << (long long)v->getPricePerDay() << ", "
             << "\"isavailable\": " << (v->getIsAvailable() ? 1 : 0) << ", "
             << "\"isAvailable\": " << (v->getIsAvailable() ? "true" : "false") << ", "
             << "\"type\": \"" << v->getType() << "\", "
             << "\"numseat\": " << numSeats << ", "
             << "\"numSeats\": " << numSeats << ", "
             << "\"enginecapacity\": " << engineCapacity << ", "
             << "\"engineCapacity\": " << engineCapacity
             << "}" << (i + 1 < list.size() ? ", " : "");
    }

    json << "]}";
    return json.str();
}

// Get available vehicles
string VehicleController::getAvailableVehicles(const string& typeFilter) {
    if (!db) return "{\"success\": false, \"data\": []}";
    vector<Vehicle*> list = db->getVehicles(typeFilter, true);
    return formatVehicleListJson(list);
}

// Get all vehicles
string VehicleController::getAllVehicles(const string& typeFilter) {
    if (!db) return "{\"success\": false, \"data\": []}";
    vector<Vehicle*> list = db->getVehicles(typeFilter, false);
    return formatVehicleListJson(list);
}

// Count vehicles in stock
string VehicleController::countVehicles() {
    if (!db) {
        return "{\"success\": false, \"counts\": {\"total\": 0, \"availableCars\": 0, \"availableMotobikes\": 0}}";
    }

    int total = 0, availableCars = 0, availableMotos = 0;
    db->countVehicles(total, availableCars, availableMotos);

    ostringstream json;
    json << "{\"success\": true, \"counts\": {"
         << "\"total\": " << total << ", "
         << "\"availableCars\": " << availableCars << ", "
         << "\"availableMotobikes\": " << availableMotos
         << "}}";
    return json.str();
}
