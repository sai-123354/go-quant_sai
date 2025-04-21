#include "RestClient.h"
#include <iostream>
#include <stdexcept>
#include <curl/curl.h>
#include "json.hpp"
using namespace std;

using json = nlohmann::json;

RestClient::RestClient(const std::string& clientId, const std::string& clientSecret, const std::string& baseUrl)
    : clientId(clientId), clientSecret(clientSecret), baseUrl(baseUrl),
      publicApiBase("/api/v2/public"), privateApiBase("/api/v2/private"), token("") {}

void RestClient::authenticate() {
    try {
        json response = makeGetRequest(publicApiBase + "/auth", {
            {"client_id", clientId},
            {"client_secret", clientSecret},
            {"grant_type", "client_credentials"}
        });

        if (!response.contains("result")) {
            throw std::runtime_error("Authentication failed");
        }

        token = response["result"]["access_token"];
       
        // std::cout << "Token: " << token << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error during authentication: " << e.what() << std::endl;
        throw;
    }
}

json RestClient::buy(const json& buy_obj) {
    ensureAuthenticated();

    try {
        json response = makeGetRequest(privateApiBase + "/buy", buy_obj);
        std::cout << "Buy response: " << response.dump(4) << std::endl;
        return response["result"];
    } catch (const std::exception& e) {
       
        std::cerr << "Error during buy: " << e.what() << std::endl;
        throw;
    }
}

json RestClient::modifyOrder(const json& orderParams) {
    ensureAuthenticated();

    try {
        json response = makeGetRequest(privateApiBase + "/edit", orderParams);
        std::cout << "Modify order response: " << response.dump(4) << std::endl;
        return response["result"];
    } catch (const std::exception& e) {
        std::cerr << "Error during modifyOrder: " << e.what() << std::endl;
        throw;
    }
}

json RestClient::cancelOrder(const std::string& order_id) {
    ensureAuthenticated();

    try {
        json response = makeGetRequest(privateApiBase + "/cancel", {{"order_id", order_id}});
        std::cout << "Cancel order response: " << response.dump(4) << std::endl;
        return response["result"];
    } catch (const std::exception& e) {
        std::cerr << "Error during cancelOrder: " << e.what() << std::endl;
        throw;
    }
}

json RestClient::getOrderBook(const std::string& instrument_name, int depth) {
    try {
        json response = makeGetRequest(publicApiBase + "/get_order_book", {
            {"instrument_name", instrument_name},
            {"depth", std::to_string(depth)}
        });
        std::cout << "Order book: " << response.dump(4) << std::endl;
        return response["result"];
    } catch (const std::exception& e) {
        std::cerr << "Error during getOrderBook: " << e.what() << std::endl;
        throw;
    }
}

json RestClient::getPositions(const json& positionParams) {
    ensureAuthenticated();

    try {
        json response = makeGetRequest(privateApiBase + "/get_positions", positionParams);
        std::cout << "Positions: " << response.dump(4) << std::endl;
        return response["result"];
    } catch (const std::exception& e) {
        std::cerr << "Error during getPositions: " << e.what() << std::endl;
        throw;
    }
}

json RestClient::makeGetRequest(const std::string& endpoint, const json& params) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl) {
        throw std::runtime_error("Failed to initialize cURL");
    }

    std::string urlWithParams = baseUrl + endpoint + "?";
    for (auto& el : params.items()) {
        urlWithParams += el.key() + "=" + el.value().get<std::string>() + "&";
    }
     
    curl_easy_setopt(curl, CURLOPT_URL, urlWithParams.c_str());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!token.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string responseString;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

    res = curl_easy_perform(curl);
    // std::cout<<res<<std::endl;
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("cURL request failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_cleanup(curl);
    return json::parse(responseString);
}

size_t RestClient::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void RestClient::ensureAuthenticated() {
    if (token.empty()) {
        authenticate();
    }
}
