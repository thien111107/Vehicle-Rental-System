#include <gtest/gtest.h>
#include "../backend/models/Vehicle.h"
#include "../backend/models/Car.h"
#include "../backend/models/Motobike.h"
#include "../backend/models/User.h"
#include "../backend/models/Customer.h"
#include "../backend/models/Manager.h"
#include "../backend/models/Admin.h"
#include "../backend/models/RentalOrder.h"
#include "../backend/models/Notification.h"

// 1. Test Car model creation and properties
TEST(ModelTest, CarAttributesAndMethods) {
    Car car(101, "Toyota", "Camry 2.5Q", "30A-999.88", 900000.0, true, 5);
    
    EXPECT_EQ(car.getId(), 101);
    EXPECT_EQ(car.getBrand(), "Toyota");
    EXPECT_EQ(car.getModel(), "Camry 2.5Q");
    EXPECT_EQ(car.getLicensePlate(), "30A-999.88");
    EXPECT_DOUBLE_EQ(car.getPricePerDay(), 900000.0);
    EXPECT_TRUE(car.getIsAvailable());
    EXPECT_EQ(car.getNumSeats(), 5);
    EXPECT_EQ(car.getType(), "Car");

    // Test toggle availability
    car.setIsAvailable(false);
    EXPECT_FALSE(car.getIsAvailable());
}

// 2. Test Motobike model creation and properties
TEST(ModelTest, MotobikeAttributesAndMethods) {
    Motobike moto(202, "Honda", "SH 150i ABS", "29G1-888.88", 250000.0, true, 150);

    EXPECT_EQ(moto.getId(), 202);
    EXPECT_EQ(moto.getBrand(), "Honda");
    EXPECT_EQ(moto.getModel(), "SH 150i ABS");
    EXPECT_DOUBLE_EQ(moto.getPricePerDay(), 250000.0);
    EXPECT_TRUE(moto.getIsAvailable());
    EXPECT_EQ(moto.getEngineCapacity(), 150);
    EXPECT_EQ(moto.getType(), "Motobike");
}

// 3. Test Polymorphism with Vehicle base pointer
TEST(OOPTest, VehiclePolymorphism) {
    Vehicle* v1 = new Car(1, "Mazda", "CX-5", "29B-123.45", 850000.0, true, 5);
    Vehicle* v2 = new Motobike(2, "Yamaha", "Exciter", "30H-666.77", 150000.0, true, 155);

    EXPECT_EQ(v1->getType(), "Car");
    EXPECT_EQ(v2->getType(), "Motobike");
    EXPECT_TRUE(v1->getIsAvailable());

    delete v1;
    delete v2;
}

// 4. Test Customer entity properties
TEST(ModelTest, CustomerAttributes) {
    Customer cus(3, "testcustomer", "pass123", "001200009999", "B2-123456", 0.0);

    EXPECT_EQ(cus.getId(), 3);
    EXPECT_EQ(cus.getUsername(), "testcustomer");
    EXPECT_EQ(cus.getPassword(), "pass123");
    EXPECT_EQ(cus.getRole(), "Customer");
    EXPECT_EQ(cus.getCccd(), "001200009999");
    EXPECT_EQ(cus.getDriverLicense(), "B2-123456");
}

// 5. Test Manager and Admin roles
TEST(ModelTest, UserRoleInheritance) {
    Manager mgr(10, "manager_user", "mgrpass");
    Admin adm(1, "admin_user", "admpass");

    EXPECT_EQ(mgr.getRole(), "Manager");
    EXPECT_EQ(adm.getRole(), "Admin");
}

// 6. Test RentalOrder lifecycle
TEST(ModelTest, RentalOrderLifecycle) {
    RentalOrder order(1, 3, 101, "001200009999", "B2-123456", "2026-11-20", "2026-11-22", 1800000.0, "Pending");

    EXPECT_EQ(order.getOrderId(), 1);
    EXPECT_EQ(order.getCustomerId(), 3);
    EXPECT_EQ(order.getVehicleId(), 101);
    EXPECT_DOUBLE_EQ(order.getTotalAmount(), 1800000.0);
    EXPECT_EQ(order.getStatus(), "Pending");

    // Manager approves
    order.setStatus("Approved");
    EXPECT_EQ(order.getStatus(), "Approved");

    // Customer completes / returns
    order.setStatus("Completed");
    EXPECT_EQ(order.getStatus(), "Completed");
}

// 7. Test Notification entity
TEST(ModelTest, NotificationAttributes) {
    Notification notif(1, 3, 10, "Order #10 has been APPROVED!", false);

    EXPECT_EQ(notif.getId(), 1);
    EXPECT_EQ(notif.getCustomerId(), 3);
    EXPECT_EQ(notif.getOrderId(), 10);
    EXPECT_FALSE(notif.getIsRead());

    notif.setIsRead(true);
    EXPECT_TRUE(notif.getIsRead());
}
