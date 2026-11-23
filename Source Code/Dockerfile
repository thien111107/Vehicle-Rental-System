# ==============================================================================
# Multi-stage Dockerfile for Vehicle Rental System (C++ Backend + Web Frontend)
# ==============================================================================

# ------------------------------------------------------------------------------
# Stage 1: Build the C++ Backend application using GCC compiler
# ------------------------------------------------------------------------------
FROM gcc:12-bullseye AS builder

WORKDIR /app

# Copy all source files and data
COPY backend/ ./backend/
COPY database_store.json ./

# Compile C++ backend into a Linux binary executable
# Static link libgcc and libstdc++ for maximum portability and no external dependencies
RUN g++ -std=c++17 -O2 -Wall \
    backend/main.cpp \
    backend/database/DatabaseConnection.cpp \
    backend/controllers/AuthController.cpp \
    backend/controllers/VehicleController.cpp \
    backend/controllers/RentalController.cpp \
    backend/controllers/AdminController.cpp \
    backend/controllers/NotificationController.cpp \
    backend/http/Router.cpp \
    backend/http/HttpServer.cpp \
    -pthread \
    -static-libgcc \
    -static-libstdc++ \
    -o server

# ------------------------------------------------------------------------------
# Stage 2: Minimal Runtime Environment
# ------------------------------------------------------------------------------
FROM debian:bullseye-slim

WORKDIR /app

# Copy compiled binary from builder stage
COPY --from=builder /app/server ./server

# Copy frontend static files (HTML, CSS, JS) and JSON database store
COPY frontend/ ./frontend/
COPY database_store.json ./

# Make sure server binary is executable
RUN chmod +x ./server

# Expose HTTP port 8080 for web access
EXPOSE 8080

# Run the compiled C++ HTTP server
CMD ["./server"]
