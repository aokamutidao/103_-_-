/**
 * @file client_ui.h
 * @brief 客户端用户界面
 */

#ifndef CLIENT_UI_H
#define CLIENT_UI_H

#include "client/flight_client.h"
#include <memory>

class ClientUI {
public:
    ClientUI(std::shared_ptr<FlightClient> client);
    ~ClientUI() = default;

    void run();

private:
    void showMenu();
    void handleQueryFlight();
    void handleGetFlightInfo();
    void handleReserveSeats();
    void handleGetSeatCount();
    void handleCancelReservation();
    void handleRegisterMonitor();

    std::shared_ptr<FlightClient> client_;
};

#endif // CLIENT_UI_H
