#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include "include/json.hpp"  // Make sure json.hpp is inside the "include/" folder

std::string response_data;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    response_data.append((char*)contents, total_size);
    return total_size;
}

int main() {
    CURL* curl;
    CURLcode res;

    const std::string client_id = "rDS0XZB6";
    const std::string client_secret = "GbjQiaXuGHE99s93O-o_L8vA8VZjaUdB5WDAAd85_HU";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        const std::string url = "https://test.deribit.com/api/v2/public/auth";
        nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "public/auth"},
            {"params", {
                {"grant_type", "client_credentials"},
                {"client_id", client_id},
                {"client_secret", client_secret}
            }}
        };

        std::string payload_str = payload.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        response_data.clear();
        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            nlohmann::json response = nlohmann::json::parse(response_data, nullptr, false);
            if (response.contains("result") && response["result"].contains("access_token")) {
                std::string token = response["result"]["access_token"];
                std::cout << "✅ Access Token: " << token << std::endl;
            } else {
                std::cerr << "❌ Error: access_token not found in response.\n";
                std::cerr << "Full response:\n" << response.dump(4) << std::endl;
            }
        } else {
            std::cerr << "Curl error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
