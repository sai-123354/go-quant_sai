#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include <string>
#include "json.hpp"

using json = nlohmann::json;

class RestClient {
public:
    RestClient(const std::string& clientId, const std::string& clientSecret, const std::string& baseUrl);

    void authenticate();
    json buy(const json& buy_obj);
    json modifyOrder(const json& orderParams);
    json cancelOrder(const std::string& order_id);
    json getOrderBook(const std::string& instrument_name, int depth = 5);
    json getPositions(const json& positionParams);

private:
    std::string clientId;
    std::string clientSecret;
    std::string baseUrl;
    std::string publicApiBase;
    std::string privateApiBase;
    std::string token;

    json makeGetRequest(const std::string& endpoint, const json& params);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);
    void ensureAuthenticated();
};

#endif // RESTCLIENT_H
