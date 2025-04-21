#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <string>
#include <set>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>
#include "RestClient.h"

class WebSocketServer {
public:
    WebSocketServer(int port, RestClient& restClient);
    ~WebSocketServer();
    void run();

private:
    typedef websocketpp::server<websocketpp::config::asio> server;
    server wsServer;
    RestClient& restClient;

    // Subscription tracking: symbol -> set of connection handles
    std::unordered_map<std::string, std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>>> subscriptions;

    // Control variable to stop the update thread gracefully
    std::atomic<bool> running;

    void onOpen(websocketpp::connection_hdl hdl);
    void onClose(websocketpp::connection_hdl hdl);
    void onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg);
    void sendOrderBookUpdates(const std::string& symbol);

    // Helper functions for managing subscriptions
    void subscribeClient(websocketpp::connection_hdl hdl, const std::string& symbol);
    void unsubscribeClient(websocketpp::connection_hdl hdl);
};
