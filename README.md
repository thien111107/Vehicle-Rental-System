# Vehicle Rental System

A full-stack Vehicle Rental System engineered following the standard **MVC (Model - View - Controller)** architectural pattern. Built with a high-performance **C++17 native backend** and a modern **HTML5/CSS3/JavaScript web frontend**. 

> **Zero External Database Dependency:** (`database_store.json`).

---

## 1. Key Architectural Features

- **Model-View-Controller (MVC):** Strict separation of concerns between business models, routing controllers, and client views.
- **Cross-Platform Compatibility:** Native Winsock support for Windows and POSIX Sockets for Linux/macOS/Docker containers.
- **Automated Unit Testing:** Integrated **Google Test (GTest)** suite verifying Object-Oriented Programming (OOP) principles, polymorphism, and business logic.
- **Containerization:** Ready-to-deploy multi-stage `Dockerfile` and `docker-compose.yml`.
- **Role-Based Access Control (RBAC):** Dedicated dashboards and route guards for Admin, Manager, and Customer roles.

---

## 2. System Architecture

### Frontend (View)
- `index.html`: Unified authentication portal (Login & Registration with validation).
- `customer.html`: Customer portal (Catalog browsing, vehicle type filtering, rental booking, history, and real-time notifications).
- `manager.html`: Branch Manager dashboard (Vehicle inventory management, rental order approval/rejection, vehicle return inspection).
- `admin.html`: Superuser dashboard (Manager account administration and system-wide revenue analytics).

### Backend (C++ Controllers & HTTP Engine)
- `HttpServer`: Lightweight, multi-threaded raw TCP socket HTTP server.
- `Router`: HTTP request dispatcher supporting RESTful API methods (GET, POST, OPTIONS) and static file serving.
- `AuthController`: Handles credential verification and new customer registration.
- `VehicleController`: Manages vehicle inventory (Cars and Motorbikes) using OOP inheritance and polymorphism.
- `RentalController`: Handles order placement, status workflows (`PENDING` -> `APPROVED`/`REJECTED` -> `RETURNED`), and rental history.
- `NotificationController`: Dispatches status notifications to customers upon manager order decisions.
- `AdminController`: Facilitates manager provisioning and aggregates total revenue statistics.
- `DatabaseConnection`: Singleton data manager for atomic in-memory querying and JSON disk synchronization.

---

## 3. Demo Accounts

| Role | Username | Password | Permissions & Capabilities |
| :--- | :--- | :--- | :--- |
| **Admin** | `admin` | `admin123` | System oversight, create/delete branch managers, view revenue statistics. |
| **Manager** | `manager1` | `manager123` | Inventory management (Add/Edit/Delete Cars and Motorbikes), approve/reject orders, process vehicle returns. |
| **Customer** | `customer1` | `customer123` | Browse catalog, filter available vehicles, create rental orders, view rental history and decision notifications. |

---

## 4. Quick Start Guide

### Option A: Native Command Line (Windows, Linux, macOS)

Prerequisites: `g++` compiler supporting C++17.

1. **Compile Backend:**
   - **On Windows (MinGW / MSYS2):**
     ```bash
     g++ -std=c++17 -Wall -O2 backend/main.cpp backend/database/DatabaseConnection.cpp backend/controllers/AuthController.cpp backend/controllers/VehicleController.cpp backend/controllers/RentalController.cpp backend/controllers/AdminController.cpp backend/controllers/NotificationController.cpp backend/http/Router.cpp backend/http/HttpServer.cpp -lws2_32 -o server.exe
     ```
   - **On Linux / macOS:**
     ```bash
     g++ -std=c++17 -Wall -O2 backend/main.cpp backend/database/DatabaseConnection.cpp backend/controllers/AuthController.cpp backend/controllers/VehicleController.cpp backend/controllers/RentalController.cpp backend/controllers/AdminController.cpp backend/controllers/NotificationController.cpp backend/http/Router.cpp backend/http/HttpServer.cpp -pthread -o server
     ```

2. **Start Server:**
   - **On Windows:**
     ```bash
     .\server.exe
     ```
   - **On Linux / macOS:**
     ```bash
     ./server
     ```

3. **Access Web Application:**
   - Open browser and navigate to: **`http://localhost:8080`**

4. **Run Unit Tests (Google Test):**
   - **On Windows:**
     ```bash
     g++ -std=c++17 -I tests/googletest/googletest/include -I tests/googletest/googletest tests/googletest/googletest/src/gtest-all.cc tests/googletest/googletest/src/gtest_main.cc tests/unit_tests.cpp -o run_tests.exe
     .\run_tests.exe
     ```
   - **On Linux / macOS:**
     ```bash
     g++ -std=c++17 -pthread -I tests/googletest/googletest/include -I tests/googletest/googletest tests/googletest/googletest/src/gtest-all.cc tests/googletest/googletest/src/gtest_main.cc tests/unit_tests.cpp -o run_tests
     ./run_tests
     ```

5. **Stop Server:**
   - Press `Ctrl + C` in the running terminal.

---

### Option B: Docker Container (Recommended for Cross-Platform Deployment)

Prerequisites: [Docker Desktop](https://www.docker.com/products/docker-desktop/) installed and running.

1. **Build and Run Container:**
   ```bash
   docker compose up -d --build
   ```
2. **Access Application:**
   - Open browser at: **`http://localhost:8080`**
3. **View Server Logs:**
   ```bash
   docker compose logs -f
   ```
4. **Stop Container:**
   ```bash
   docker compose down
   ```
5. **Restart the server at any time once it has been built**
   ```bash
   docker compose up -d
---

## 5. Project Directory Structure

```text
VehicleRentalSystem/
├── backend/
│   ├── controllers/      # AuthController, VehicleController, RentalController, etc.
│   ├── database/         # DatabaseConnection (Singleton JSON file manager)
│   ├── http/             # HttpServer (TCP Sockets) and Router (REST & Static)
│   ├── models/           # User, Customer, Manager, Admin, Vehicle, Car, Motobike, RentalOrder
│   └── main.cpp          # Application entry point
├── frontend/
│   ├── css/              # Responsive stylesheets (dashboard.css, customer.css, etc.)
│   ├── js/               # API clients and view controllers (customer.js, manager.js, etc.)
│   ├── index.html        # Authentication view
│   ├── customer.html     # Customer portal
│   ├── manager.html      # Manager portal
│   └── admin.html        # Superuser portal
├── tests/
│   ├── googletest/       # Google Test framework source
│   └── unit_tests.cpp    # OOP and business logic test suite
├── database_store.json   # Persistent JSON database
├── Dockerfile            # Multi-stage production container build
├── docker-compose.yml    # Service orchestration
├── .dockerignore         # Docker ignore rules
├── .gitignore            # Git ignore rules
└── README.md             # Project documentation
```

---

## 6. Testing & Quality Assurance

- **Unit Testing:** Implemented using **Google Test**. Covers polymorphic vehicle instantiations, dynamic inheritance behavior, customer balance mutations, and state machine transitions of rental orders.
- **Functional Testing:** Detailed test cases documented in [`TESTING_DOCUMENT.md`](file:///c:/Vehicle1/TESTING_DOCUMENT.md) and [`TestCases_VehicleRental.csv`](file:///c:/Vehicle1/TestCases_VehicleRental.csv).
