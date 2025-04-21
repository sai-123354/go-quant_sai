#include "env_loader.h"
#include "RestClient.h"
#include "WSServer.h"
#include <iostream>
#include <cstdlib>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

int main()
{

    loadEnv(".env");

    const std::string clientId = std::getenv("DERIBIT_CLIENT_ID");
    const std::string clientSecret = std::getenv("DERIBIT_CLIENT_SECRET");
    const std::string baseUrl = "https://test.deribit.com";
    std::cout << "DEBUG: clientId = " << clientId << std::endl;
    std::cout << "DEBUG: clientSecret = " << clientSecret << std::endl;
    if (clientId.empty() || clientSecret.empty())
    {
        std::cerr << "Error: CLIENT_ID or CLIENT_SECRET not set in environment variables." << std::endl;
        return 1;
    }

    RestClient client(clientId, clientSecret, baseUrl);

    try
    {

        client.authenticate();

       // ************************   Create a buy order ************************ 

        json buy_obj = {
            {"amount", "50"},
             {"instrument_name", "BTC-PERPETUAL"},
             {"type", "limit"},
             {"price","60"}
         };

         try {
             auto start_loop = std::chrono::high_resolution_clock::now();
             json buyResponse = client.buy(buy_obj);
             auto end_loop = std::chrono::high_resolution_clock::now();
             auto loop_latency = std::chrono::duration_cast<std::chrono::microseconds>(end_loop - start_loop).count();
             std::cout << "🔁 End-to-End Trading Loop Latency: " << loop_latency << " microseconds" << std::endl;
             std::cout << "Order placed: " << buyResponse.dump(4) << std::endl;
         } catch (const std::exception& e) {
             std::cerr << "Error in placing order: " << e.what() << std::endl;

         }






          // ************************  Modify an order  ************************

         json modifyParams = {
             {"amount", "80"},
             {"order_id", "29627727487"},
             // {"price", "0.21"}
         };

        try {
             auto start_modify = std::chrono::high_resolution_clock::now();
             json modifyResponse = client.modifyOrder(modifyParams);
             auto end_modify = std::chrono::high_resolution_clock::now();
             auto modify_latency = std::chrono::duration_cast<std::chrono::microseconds>(end_modify - start_modify).count();
             std::cout << "🛠️ Modify Order Latency: " << modify_latency << " microseconds" << std::endl;
             std::cout << "Order modified: " << modifyResponse.dump(4) << std::endl;
         } catch (const std::exception& e) {
             std::cerr << "Error in modifying order: " << e.what() << std::endl;
         }





        // ************************  Cancel an order ************************  
         std::string order_id = "29627727487";
         try {
             json cancelResponse = client.cancelOrder(order_id);
             std::cout << "Order canceled: " << cancelResponse.dump(4) << std::endl;
         } catch (const std::exception& e) {
             std::cerr << "Error in canceling order: " << e.what() << std::endl;
         }







        // ************************   Get order book  ************************

         try {
             json orderBookResponse = client.getOrderBook("BTC-PERPETUAL", 5);
             std::cout << "Order Book: " << orderBookResponse.dump(4) << std::endl;
         } catch (const std::exception& e) {
             std::cerr << "Error in getting order book: " << e.what() << std::endl;
         }






       // ************************   Get positions ************************ 

         json positionParams = {
             {"currency", "BTC"},
             {"kind", "future"}
         };

         try {
             json positionsResponse = client.getPositions(positionParams);
             std::cout << "Positions: " << positionsResponse.dump(4) << std::endl;
         } catch (const std::exception& e) {
             std::cerr << "Error in getting positions: " << e.what() << std::endl;
         }








        //  ************************  Websocket  ************************

        WebSocketServer wsServer(8080, client);
        std::cout << "WebSocket server started successfully on port 8080." << std::endl;

       //  Run the WebSocket server (this is a blocking call)
        wsServer.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "General error: " << e.what() << std::endl;
    }

    return 0;
}
