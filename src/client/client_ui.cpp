/**
 * @file client_ui.cpp
 * @brief 客户端用户界面实现
 */

#include "client/client_ui.h"
#include "common/message_types.h"
#include "common/logger.h"
#include <iostream>

ClientUI::ClientUI(std::shared_ptr<FlightClient> client) : client_(client) {}

void ClientUI::run() {
    bool running = true;

    while (running) {
        showMenu();

        int choice;
        std::cout << "Enter your choice (0-6): ";
        std::cin >> choice;

        switch (choice) {
            case 0:
                running = false;
                std::cout << "Exiting...\n";
                break;
            case 1:
                handleQueryFlight();
                break;
            case 2:
                handleGetFlightInfo();
                break;
            case 3:
                handleReserveSeats();
                break;
            case 4:
                handleGetSeatCount();
                break;
            case 5:
                handleCancelReservation();
                break;
            case 6:
                handleRegisterMonitor();
                break;
            default:
                std::cout << "Invalid choice!\n";
        }

        std::cout << "\n";
    }
}

void ClientUI::showMenu() {
    std::cout << "\n========== Flight Information System ==========\n"
              << "1. Query flights by source and destination\n"
              << "2. Get flight information\n"
              << "3. Reserve seats\n"
              << "4. Get seat count\n"
              << "5. Cancel reservation\n"
              << "6. Register monitor\n"
              << "0. Exit\n"
              << "===============================================\n";
}

void ClientUI::handleQueryFlight() {
    std::string source, destination;
    std::cout << "Enter source: ";
    std::cin >> source;
    std::cout << "Enter destination: ";
    std::cin >> destination;

    std::vector<int32_t> flight_ids;
    int32_t status = client_->queryFlight(source, destination, flight_ids);

    if (status == ERR_SUCCESS) {
        std::cout << "Found " << flight_ids.size() << " flights:\n";
        for (int32_t id : flight_ids) {
            std::cout << "  Flight " << id << "\n";
        }
    } else {
        std::cout << "Error: " << getErrorMessage(status) << "\n";
    }
}

void ClientUI::handleGetFlightInfo() {
    int32_t flight_id;
    std::cout << "Enter flight ID: ";
    std::cin >> flight_id;

    Flight flight;
    int32_t status = client_->getFlightInfo(flight_id, flight);

    if (status == ERR_SUCCESS) {
        std::cout << "Flight Information:\n"
                  << "  Flight ID: " << flight.flight_id << "\n"
                  << "  Source: " << flight.source << "\n"
                  << "  Destination: " << flight.destination << "\n"
                  << "  Departure: " << flight.departure_time.toString() << "\n"
                  << "  Airfare: " << flight.airfare << "\n"
                  << "  Available Seats: " << flight.seat_availability << "\n";
    } else {
        std::cout << "Error: " << getErrorMessage(status) << "\n";
    }
}

void ClientUI::handleReserveSeats() {
    int32_t flight_id, seat_count;
    std::cout << "Enter flight ID: ";
    std::cin >> flight_id;
    std::cout << "Enter number of seats: ";
    std::cin >> seat_count;

    int32_t remaining;
    int32_t status = client_->reserveSeats(flight_id, seat_count, remaining);

    if (status == ERR_SUCCESS) {
        std::cout << "Reservation successful! Remaining seats: " << remaining << "\n";
    } else {
        std::cout << "Error: " << getErrorMessage(status) << "\n";
    }
}

void ClientUI::handleGetSeatCount() {
    int32_t flight_id;
    std::cout << "Enter flight ID: ";
    std::cin >> flight_id;

    int32_t count;
    int32_t status = client_->getSeatCount(flight_id, count);

    if (status == ERR_SUCCESS) {
        std::cout << "Available seats: " << count << "\n";
    } else {
        std::cout << "Error: " << getErrorMessage(status) << "\n";
    }
}

void ClientUI::handleCancelReservation() {
    int32_t flight_id, seat_count;
    std::cout << "Enter flight ID: ";
    std::cin >> flight_id;
    std::cout << "Enter number of seats to cancel: ";
    std::cin >> seat_count;

    int32_t new_count;
    int32_t status = client_->cancelReservation(flight_id, seat_count, new_count);

    if (status == ERR_SUCCESS) {
        std::cout << "Cancellation successful! Current seats: " << new_count << "\n";
    } else {
        std::cout << "Error: " << getErrorMessage(status) << "\n";
    }
}

void ClientUI::handleRegisterMonitor() {
    int32_t flight_id, interval;
    std::cout << "Enter flight ID: ";
    std::cin >> flight_id;
    std::cout << "Enter monitor interval (seconds): ";
    std::cin >> interval;

    auto callback = [](int32_t flight_id, int32_t seat_count, int32_t remaining_time) {
        std::cout << "\n[Callback] Flight " << flight_id
                  << " seat count updated to " << seat_count
                  << " (remaining monitor time: " << remaining_time << "s)\n";
    };

    int32_t status = client_->registerMonitor(flight_id, interval, callback);

    if (status == ERR_SUCCESS) {
        std::cout << "Monitor registered. Waiting for updates...\n";
        std::cout << "(Client is blocked during monitoring)\n";
    } else {
        std::cout << "Error: " << getErrorMessage(status) << "\n";
    }
}
