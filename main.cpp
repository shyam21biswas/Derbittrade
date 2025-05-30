#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


// Function to handle the response from the cURL request
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


// Class to measure latency
class LatencyModule {
public:
    static std::chrono::high_resolution_clock::time_point start() {
        return std::chrono::high_resolution_clock::now();
    }

    static void end(const std::chrono::high_resolution_clock::time_point& start_time, const std::string& action_name) {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> latency = end_time - start_time;
        std::cout << action_name << " Latency: " << latency.count() << " seconds" << std::endl;
    }
};


// General function to send a cURL request with optional access token
std::string sendRequest(const std::string& url, const json& payload, const std::string& accessToken = "") {
    std::string readBuffer;
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);  // Set the HTTP method to POST

        // Set the request payload
        std::string jsonStr = payload.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        // Set headers, including Authorization if accessToken is provided
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!accessToken.empty()) {
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set up the write callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Free Resources
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return readBuffer;
}

// Error log storage define in Golbal level....
std::vector<std::string> errorLog;

// Function to get the access token
std::string getAccessToken(const std::string& clientId, const std::string& clientSecret) {
    json payload = {
        {"id", 0},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"scope", "session:apiconsole-c5i26ds6dsr expires:2592000"},
            {"client_id", clientId},
            {"client_secret", clientSecret}
        }},
        {"jsonrpc", "2.0"}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/public/auth", payload);
    auto responseJson = json::parse(response);

    // Retrieve the access token from the response
    if (responseJson.contains("result") && responseJson["result"].contains("access_token")) {
        return responseJson["result"]["access_token"];
    } else {
        std::cerr << "Failed to retrieve access token." << std::endl;
        return "";
    }
}



// Function to place an order
void placeOrder(const std::string& price, const std::string& accessToken, const std::string& amount, const std::string& instrument) {
    auto start_time = std::chrono::high_resolution_clock::now();
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/buy"},
        {"params", {
            {"instrument_name", instrument},
            {"type", "limit"},
            {"price", price},
            {"amount", amount}
        }},
        {"id", 1}
    };


    try {
        std::string response = sendRequest("https://test.deribit.com/api/v2/private/buy", payload, accessToken);
        std::cout << "Place Order Response: " << response << std::endl;

        // Check if the response indicates an error
        auto responseJson = json::parse(response);
        if (responseJson.contains("error")) {
            std::cerr << "Error placing order: " << responseJson["error"] << std::endl;
            //errorLog.push_back( "Error placing Order: ");
            errorLog.push_back( responseJson["error"]);
        }
    } catch (const std::exception& e) {
        // Log exception details
        std::cerr << "Exception while placing order: " << e.what() << std::endl;
        errorLog.push_back(e.what());
    } catch (...) {
        // Catch all unexpected errors
        std::cerr << "An unknown error occurred while placing order." << std::endl;
        errorLog.push_back("An unknown error occurred while placing order.");
    }

    // End measuring latency
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end_time - start_time;
    std::cout << "placeOrder Latency: " << latency.count() << " seconds" << std::endl;

}

// Function to cancel an order
void cancelOrder(const std::string& accessToken, const std::string& orderID) {
    auto start_time = std::chrono::high_resolution_clock::now();
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/cancel"},
        {"params", {{"order_id", orderID}}},
        {"id", 6}
    };


    try {
        std::string response = sendRequest("https://test.deribit.com/api/v2/private/cancel", payload,accessToken);
        std::cout << "Cancel Order Response: " << response << std::endl;

        // Check if the response indicates an error
        auto responseJson = json::parse(response);
        if (responseJson.contains("error")) {
            std::cerr << "Error cancel order: " << responseJson["error"] << std::endl;
            errorLog.push_back( responseJson["error"]);
        }
    } catch (const std::exception& e) {
        // Log exception details
        std::cerr << "Exception while cancel order: " << e.what() << std::endl;
        errorLog.push_back(e.what());
    } catch (...) {
        // Catch all unexpected errors
        std::cerr << "An unknown error occurred while cancel order." << std::endl;
        errorLog.push_back("An unknown error occurred while cancel order.");
    }

    // End measuring latency
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end_time - start_time;
    std::cout << "CancelOrder Latency: " << latency.count() << " seconds" << std::endl;

}

// Function to modify an order
void modifyOrder(const std::string& accessToken, const std::string& orderID, int amount, double price) {
    auto start_time = std::chrono::high_resolution_clock::now();
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/edit"},
        {"params", {
            {"order_id", orderID},
            {"amount", amount},
            {"price", price}
        }},
        {"id", 11}
    };



    try {
        std::string response = sendRequest("https://test.deribit.com/api/v2/private/edit", payload,accessToken);
        std::cout << "Modify Order Response: " << response << std::endl;

        // Check if the response indicates an error
        auto responseJson = json::parse(response);
        if (responseJson.contains("error")) {
            std::cerr << "Error Modify Order: " << responseJson["error"] << std::endl;

            errorLog.push_back( responseJson["error"]);

        }
    } catch (const std::exception& e) {
        // Log exception details
        std::cerr << "Exception while Modify Order: " << e.what() << std::endl;
        errorLog.push_back(e.what());
    } catch (...) {
        // Catch all unexpected errors
        std::cerr << "An unknown error occurred while Modify Order." << std::endl;
        errorLog.push_back("An unknown error occurred while Modify Order.");
    }

    // End measuring latency
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end_time - start_time;
    std::cout << "ModifyOrder Latency: " << latency.count() << " seconds" << std::endl;


}

// Function to retrieve the order book
void getOrderBook(const std::string& accessToken, const std::string& instrument) {
    auto start_time = std::chrono::high_resolution_clock::now();
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "public/get_order_book"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 15}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book",payload, accessToken);

    auto responseJson = json::parse(response);

      std::cout << "Order Book for " << instrument << ":\n\n";

            // Printing best bid and ask
            std::cout << "Best Bid Price: " << responseJson["result"]["best_bid_price"] << ", Amount: " << responseJson["result"]["best_bid_amount"] << '\n';
            std::cout << "Best Ask Price: " << responseJson["result"]["best_ask_price"] << ", Amount: " << responseJson["result"]["best_ask_amount"] << '\n';

            // Printing bids and asks in detail
            std::cout << "Asks:\n";
            for (const auto& ask : responseJson["result"]["asks"]) {
                std::cout << "Price: " << ask[0] << ", Amount: " << ask[1] << '\n';
            }

            std::cout << "\nBids:\n";
            for (const auto& bid : responseJson["result"]["bids"]) {
                std::cout << "Price: " << bid[0] << ", Amount: " << bid[1] << '\n';
            }

            // Additional information
            std::cout << "\nMark Price: " << responseJson["result"]["mark_price"] << '\n';
            std::cout << "Open Interest: " << responseJson["result"]["open_interest"] << '\n';
            std::cout << "Timestamp: " << responseJson["result"]["timestamp"] << '\n';

    // End measuring latency
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end_time - start_time;
    std::cout << "Get Order Latency: " << latency.count() << " seconds" << std::endl;
}

// Function to get position details of a specific instrument
void getPosition(const std::string& accessToken, const std::string& instrument) {
    auto start_time = std::chrono::high_resolution_clock::now();
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_position"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 20}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_position", payload, accessToken);
    auto responseJson = json::parse(response);

    // Parse and print position details if available
    if (responseJson.contains("result")) {
                std::cout << "Position Details for " << instrument << ":\n\n";
                auto result = responseJson["result"];
                std::cout << "Estimated Liquidation Price: " << result["estimated_liquidation_price"] << '\n';
                std::cout << "Size Currency: " << result["size_currency"] << '\n';
                std::cout << "Realized Funding: " << result["realized_funding"] << '\n';
                std::cout << "Total Profit Loss: " << result["total_profit_loss"] << '\n';
                std::cout << "Realized Profit Loss: " << result["realized_profit_loss"] << '\n';
                std::cout << "Floating Profit Loss: " << result["floating_profit_loss"] << '\n';
                std::cout << "Leverage: " << result["leverage"] << '\n';
                std::cout << "Average Price: " << result["average_price"] << '\n';
                std::cout << "Delta: " << result["delta"] << '\n';
                std::cout << "Interest Value: " << result["interest_value"] << '\n';
                std::cout << "Mark Price: " << result["mark_price"] << '\n';
                std::cout << "Settlement Price: " << result["settlement_price"] << '\n';
                std::cout << "Index Price: " << result["index_price"] << '\n';
                std::cout << "Direction: " << result["direction"] << '\n';
                std::cout << "Open Orders Margin: " << result["open_orders_margin"] << '\n';
                std::cout << "Initial Margin: " << result["initial_margin"] << '\n';
                std::cout << "Maintenance Margin: " << result["maintenance_margin"] << '\n';
                std::cout << "Kind: " << result["kind"] << '\n';
                std::cout << "Size: " << result["size"] << '\n';
    } else {
        std::cerr << "Error: Could not retrieve position data." << std::endl;
        errorLog.push_back("Error: Could not retrieve position data." );


    }

    // End measuring latency
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end_time - start_time;
    std::cout << "get Position Latency: " << latency.count() << " seconds" << std::endl;
}

// Function to print all open orders with instrument, order ID, price, and amount
void getOpenOrders(const std::string& accessToken) {
    auto start_time = std::chrono::high_resolution_clock::now();
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_open_orders"},
        {"params", {{"kind", "future"}, {"type", "limit"}}},
        {"id", 25}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_open_orders", payload, accessToken);
    auto responseJson = json::parse(response);

    // Check if the response contains the "result" array
    if (responseJson.contains("result")) {
        std::cout << "Open Orders:\n\n";
        for (const auto& order : responseJson["result"]) {
            std::string instrument = order["instrument_name"];
            std::string orderId = order["order_id"];
            double price = order["price"];
            double amount = order["amount"];

            std::cout << "Instrument: " << instrument << ", Order ID: " << orderId
                      << ", Price: " << price << ", Amount: " << amount << '\n';
        }
    } else {
        std::cerr << "Error: Could not retrieve open orders." << std::endl;
        errorLog.push_back("Error: Could not retrieve open orders." );
    }

    // End measuring latency
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end_time - start_time;
    std::cout << "getopenOrder Latency: " << latency.count() << " seconds" << std::endl;
}
//

// Main program
int main() {
    // Replace with your actual client credentials
    std::string clientId = "yW37rgCX";
    std::string clientSecret = "8ebBJ1Kq4AdZBufz5NtTZETNVHd9Babgv16rJ5KxGuI";

    // Retrieve the access token
    std::string accessToken = getAccessToken(clientId, clientSecret);



    if (!accessToken.empty()) {
        while (true) {
            std::cout << "\n\t\t..............................................................\n";

            std::cout << "\nMENU:\n";
            std::cout << "1. PLACE ORDER\n";
            std::cout << "2. CANCEL ORDER\n";
            std::cout << "3. MODIFY ORDER\n";
            std::cout << "4. GET ORDER BOOK\n";
            std::cout << "5. GET POSITION\n";
            std::cout << "6. GET OPEN ORDERS\n";
            std::cout << "7. EXIT\n";
            std::cout << "8. VIEW ERROR LOG\n";

            int variable = -1;
            std::cout << "Enter a Choice: ";
            std::cin >> variable;

            switch (variable) {
            case 1:
                try {
                    std::cout << "Placing an order..." << std::endl;
                    std::string  name;
                    std::cout << "Enter the name of instrument : ";
                    std::cin >> name;
                    std::string  price ;
                    std::cout << "Enter the price of instrument : ";
                    std::cin >> price;
                    std::string  amount ;
                    std::cout << "Enter the amount of instrument : ";
                    std::cin >> amount;

                    //call this function
                    placeOrder(price, accessToken, amount, name);

                    // Simulate order placement logic

                } catch (const std::exception& e) {
                    std::cerr << "Exception caught: " << e.what() << std::endl;
                    errorLog.push_back(e.what());
                }
                break;
            case 2:
                try {
                    std::cout << "Cancelling an order..." << std::endl;
                    // Simulate order cancellation logic
                    std::string  orderId ;
                    std::cout << "Enter the order id : ";
                    std::cin >> orderId;

                    cancelOrder(accessToken,orderId);


                } catch (const std::exception& e) {
                    std::cerr << "Exception caught: " << e.what() << std::endl;
                    errorLog.push_back(e.what());
                }
                break;
            case 3:
                try {
                    std::cout << "Modifying an order..." << std::endl;
                    std::string  orderId ;
                    std::cout << "Enter the order id : ";
                    std::cin >> orderId;
                    double  price ;
                    std::cout << "Enter the price of instrument : ";
                    std::cin >> price;
                    int  amount ;
                    std::cout << "Enter the amount of instrument : ";
                    std::cin >> amount;

                    //call the funtion

                    modifyOrder(accessToken,orderId,amount,price);

                    // Simulate order modification logic

                } catch (const std::exception& e) {
                    std::cerr << "Exception caught: " << e.what() << std::endl;
                    errorLog.push_back(e.what());
                }
                break;
            case 4:
                try {
                    std::cout << "Getting the order book..." << std::endl;
                    // Simulate fetching order book logic
                    std::string  instrument_name;
                    std::cout << "Enter the instrument name : ";
                    std::cin >> instrument_name;
                    getOrderBook(accessToken,instrument_name);


                } catch (const std::exception& e) {
                    std::cerr << "Exception caught: " << e.what() << std::endl;
                    errorLog.push_back(e.what());
                }
                break;
            case 5:
                try {
                    std::cout << "Getting position details..." << std::endl;
                    // Simulate fetching position details logic
                    std::string  instrument_name;
                    std::cout << "Enter the instrument name : ";
                    std::cin >> instrument_name;

                    getPosition(accessToken, instrument_name);

                } catch (const std::exception& e) {
                    std::cerr << "Exception caught: " << e.what() << std::endl;
                    errorLog.push_back(e.what());
                }

                break;
                case 6:
                    try {
                        std::cout << "Getting OpenOrder details..." << std::endl;
                        // Simulate fetching position details logic


                        getOpenOrders(accessToken);

                    } catch (const std::exception& e) {
                        std::cerr << "Exception caught: " << e.what() << std::endl;
                        errorLog.push_back(e.what());
                    }
                break;
            case 7:
                std::cout << "Exiting the program..." << std::endl;
                return 0;
            case 8:
                std::cout << "\nDisplaying Error Log:\n";
                if (errorLog.empty()) {
                    std::cout << "No errors logged.\n";
                } else {
                    for (size_t i = 0; i < errorLog.size(); ++i) {
                        std::cout << i + 1 << ". " << errorLog[i] << "\n";
                    }
                }
                break;
            default:
                std::cerr << "Invalid choice. Please try again." << std::endl;
                break;
            }
        }
    }
    else {
        std::cerr << "Unable to obtain access token, aborting operations." << std::endl;
    }

    return 0;
}
