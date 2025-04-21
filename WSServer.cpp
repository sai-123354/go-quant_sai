#include "WSServer.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "json.hpp"  // Using nlohmann/json

using json = nlohmann::json;  // Alias for convenience

WebSocketServer::WebSocketServer(int port, RestClient& restClient) 
    : restClient(restClient), running(true) {
    wsServer.init_asio();
    wsServer.set_open_handler(bind(&WebSocketServer::onOpen, this, std::placeholders::_1));
    wsServer.set_close_handler(bind(&WebSocketServer::onClose, this, std::placeholders::_1));
    wsServer.set_message_handler(bind(&WebSocketServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    wsServer.listen(port);
    wsServer.start_accept();
}   

WebSocketServer::~WebSocketServer() {
    running = false;
}

void WebSocketServer::run() {
    // Run the WebSocket server in a separate thread to handle client connections
    std::thread serverThread([this]() { wsServer.run(); });

    // Main loop to periodically fetch and broadcast order book updates
    while (running) {
        for (const auto& subscription : subscriptions) {
            const std::string& symbol = subscription.first;
            sendOrderBookUpdates(symbol);
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));  // 5 seconds delay
    }

    serverThread.join();
}

void WebSocketServer::onOpen(websocketpp::connection_hdl hdl) {
    std::cout << "Client connected." << std::endl;
}

void WebSocketServer::onClose(websocketpp::connection_hdl hdl) {
    std::cout << "Client disconnected." << std::endl;
    unsubscribeClient(hdl);
}

void WebSocketServer::onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    try {
        // Parse the incoming message (assumed to be JSON format)
        auto jsonData = json::parse(msg->get_payload());

        if (jsonData.contains("action") && jsonData["action"] == "subscribe" && jsonData.contains("symbol")) {
            std::string symbol = jsonData["symbol"];
            std::cout << "Client subscribed to symbol: " << symbol << std::endl;
            subscribeClient(hdl, symbol);
            auto ws_subscribe_time = std::chrono::high_resolution_clock::now();


            // Start sending updates for this symbol
            sendOrderBookUpdates(symbol);
auto ws_send_time = std::chrono::high_resolution_clock::now();
auto ws_latency = std::chrono::duration_cast<std::chrono::microseconds>(ws_send_time - ws_subscribe_time).count();
std::cout << "📡 WebSocket Market Data Latency (subscribe to send): " << ws_latency << " microseconds" << std::endl;

        } else {
            wsServer.send(hdl, R"({"error": "Invalid action or missing symbol"})", websocketpp::frame::opcode::text);
        }
    } catch (const json::exception& e) {
        wsServer.send(hdl, R"({"error": "Invalid message format"})", websocketpp::frame::opcode::text);
    }
}

void WebSocketServer::subscribeClient(websocketpp::connection_hdl hdl, const std::string& symbol) {
    // Add the client to the symbol subscription set
    subscriptions[symbol].insert(hdl);
}

void WebSocketServer::unsubscribeClient(websocketpp::connection_hdl hdl) {
    // Remove the client from all subscribed symbols
    for (auto& [symbol, clients] : subscriptions) {
        clients.erase(hdl);
    }
}

void WebSocketServer::sendOrderBookUpdates(const std::string& symbol) {


    // Create a thread to handle asynchronous order book updates
    std::thread([this, symbol]() {
        while (subscriptions[symbol].size() > 0) {  // Check if there are active subscribers for this symbol
            try {
                 auto start_processing = std::chrono::high_resolution_clock::now();
                 json orderBook = restClient.getOrderBook(symbol);
 auto end_processing = std::chrono::high_resolution_clock::now();
                auto processing_latency = 
std::chrono::duration_cast<std::chrono::microseconds>(end_processing - start_processing).count();
                std::cout << "⚙️ Market Data Processing Latency: " << processing_latency
                          << " microseconds for symbol: " << symbol << std::endl;
                if (orderBook.is_null()) {
                    std::cerr << "Received null order book for " << symbol << std::endl;
                    continue;
                }

                // Create a message with the symbol and order book data
                json message;
                message["symbol"] = symbol;
                message["orderBook"] = orderBook;
                message["server_timestamp"] = std::chrono::high_resolution_clock::now().time_since_epoch().count();

                // Convert message to string and broadcast to all clients subscribed to this symbol
                std::string orderBookMessage = message.dump();
                for (const auto& client : subscriptions[symbol]) {
                    if (wsServer.get_con_from_hdl(client)->get_state() == websocketpp::session::state::open) {
                        wsServer.send(client, orderBookMessage, websocketpp::frame::opcode::text);
                    }
                }

                // Delay to avoid overwhelming the API
                std::this_thread::sleep_for(std::chrono::seconds(5));
            } catch (const std::exception& e) {
                std::cerr << "Error fetching order book for " << symbol << ": " << e.what() << std::endl;
            }
        }
    }).detach();  // Detach the thread to allow continuous execution in the background
}
