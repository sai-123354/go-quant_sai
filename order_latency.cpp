#include "env_loader.h"
#include "RestClient.h"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    loadEnv(".env");

    std::string clientId = std::getenv("DERIBIT_CLIENT_ID");
    std::string clientSecret = std::getenv("DERIBIT_CLIENT_SECRET");
    std::string baseUrl = "https://test.deribit.com";

    if (clientId.empty() || clientSecret.empty()) {
        std::cerr << "❌ Environment variables not loaded!" << std::endl;
        return 1;
    }

    RestClient client(clientId, clientSecret, baseUrl);

    try {
        client.authenticate();

        json buy_obj = {
            {"amount", "10"},
            {"instrument_name", "BTC-PERPETUAL"},
            {"type", "limit"},
            {"price", "70"}
        };

        auto start = std::chrono::high_resolution_clock::now();
        json buyResponse = client.buy(buy_obj);
        auto end = std::chrono::high_resolution_clock::now();

        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "✅ Order Placement Latency: " << latency << " microseconds" << std::endl;
        std::cout << "📦 Response: " << buyResponse.dump(4) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
    }

    return 0;
}

