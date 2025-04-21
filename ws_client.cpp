#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iostream>

typedef websocketpp::client<websocketpp::config::asio_client> client;
using json = nlohmann::json;

void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    try {
        json message = json::parse(msg->get_payload());

        if (message.contains("server_timestamp")) {
            auto server_ts = message["server_timestamp"].get<long long>();
            auto latency = now - server_ts;
            std::cout << "📡 WebSocket Propagation Delay: " << latency << " ns" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Failed to parse message: " << e.what() << std::endl;
    }
}

int main() {
    client c;

    try {
        c.init_asio();
        c.set_message_handler(&on_message);

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection("ws://localhost:8080", ec);

        if (ec) {
            std::cerr << "Connection failed: " << ec.message() << std::endl;
            return 1;
        }

        c.connect(con);
        c.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

