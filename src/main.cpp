#include <iostream>
#include "../include/api.h" 
#include "json.hpp"


using json = nlohmann::json;


int main() {
    std::string login;
    std::string password;
    std::cout << "************************************************************"<< std::endl;
    std::cout << "           LOGIN TO YOUR ACCOUNT        "<< std::endl;
    std::cout << "USERNAME:  " << std::endl;
    std::cin >> login;
    std::cout << "PASSWORD:  " << std::endl;
    std::cin >> password;

    API api(login , password);

    // Authenticate and get access token
    std::string access_token = api.authenticate();

    if (access_token.empty()) {
        std::cerr << "\u274C Authentication failed! Please check your credentials." << std::endl;
        return 1;
    }

    std::cout << std::endl;
    std::cout << "\u2705 Authentication successful!" << std::endl;

    int choice;
    while (true) {
        std::cout << "************************************************************\n";
        std::cout << "1. PLACE ORDER\n";
        std::cout << "2. CANCEL ORDER\n";
        std::cout << "3. MODIFY ORDER\n";
        std::cout << "4. GET ORDERBOOK\n";
        std::cout << "5. VIEW CURRENT POSITIONS\n";
        std::cout << "6. EXIT\n";
        std::cout << "************************************************************\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) { 
            case 1: {  // Place Order
                std::cout << "\U0001F680 Placing order..." << std::endl;
                std::string instrument_name, order_type;
                int amount;
                double price = 0.0;

                std::cout << "\n\U0001F680 Enter instrument name (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument_name;

                std::cout << "\U0001F680 Enter order type (limit/market): ";
                std::cin >> order_type;

                std::cout << "\U0001F680 Enter amount: ";
                std::cin >> amount;

                if (order_type == "limit") {
                    std::cout << "\U0001F680 Enter price: ";
                    std::cin >> price;
                }

                std::cout << "\n\U0001F680 Placing order...\n";
                std::string order_response = api.place_order(access_token, instrument_name, amount, order_type, price);

                // ✅ Parse JSON response
                json order_json = json::parse(order_response, nullptr, false);
                if (order_json.is_discarded()) {
                    std::cerr << "\u274C Error: Failed to parse JSON response!" << std::endl;
                    continue;
                }
                std::cout << "ORDER PLACED SUCCESSFULLY 💰" << std::endl;
                // ✅ Debugging: Print Order Response
                std::cout << "ORDER ID: " << order_json.dump(4) << "\n";

                // ✅ Extract order ID safely
                if (!order_json.contains("result") || 
                    !order_json["result"].contains("order") || 
                    !order_json["result"]["order"].contains("order_id")) {
                    
                    //std::cerr << "\u274C Error: 'order_id' missing in response!" << std::endl;
                    continue;
                }

                std::string order_id = order_json["result"]["order"]["order_id"];
                std::cout << "\U0001F680 Order placed successfully! Order ID: " << order_id << "\n";
                break;
            }

            case 2: {  // Cancel Order
                std::cout << "\U0001F680 Enter Order ID to cancel: ";
                std::string order_id;
                std::cin >> order_id;

                json cancel_response = api.cancel_order(access_token, order_id);

                std::cout << "🚀 Attempting to cancel order: " << order_id << "\n";
                std::cout << "Cancel Order Raw Response: " << cancel_response.dump(4) << "\n";

                if (cancel_response.contains("result") && cancel_response["result"].contains("order_state") &&
                    cancel_response["result"]["order_state"] == "cancelled") {
                    std::cout << "\U00002705 Order " << order_id << " is now cancelled.\n";
                } else {
                    std::cerr << "\U0001F6AB Failed to cancel order. Check response above.\n";
                }
                break;
            }

            case 3: { // MODIFY ORDER
                std::cout << "🛠 Enter Order ID to modify: ";
                std::string order_id;
                std::cin >> order_id;

                int new_amount;
                double new_price;

                std::cout << "🛠 Enter new amount: ";
                std::cin >> new_amount;

                std::cout << "🛠 Enter new price: ";
                std::cin >> new_price;

                std::cout << "🚀 Modifying order..." << std::endl;

                json modify_response = api.modify_order(access_token, order_id, new_amount, new_price);

                // ✅ Extract Order ID from response
                if (modify_response.contains("result") && modify_response["result"].contains("order") &&
                    modify_response["result"]["order"].contains("order_id")) {
                    std::cout << "✅ Order modified successfully! New Order ID: "<< modify_response["result"]["order"]["order_id"] << std::endl;
                } else {
                std::cerr << "❌ Failed to modify order!" << std::endl;
                }
                break;
            }


            case 4:
                std::cout << "📖 Retrieving Order Book..." << std::endl;
                {
                std::string instrument;
                std::cout << "🚀 Enter instrument name for order book (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument;

                json order_book_response = api.get_order_book(instrument);

                if (order_book_response.contains("error")) {
                    std::cerr << "❌ Failed to fetch order book!\n";
                    break;
                }

                // ✅ Extract key order book details
                double best_ask = order_book_response["best_ask_price"];
                double best_bid = order_book_response["best_bid_price"];
                double last_price = order_book_response["last_price"];
                double high_24h = order_book_response["stats"]["high"];
                double low_24h = order_book_response["stats"]["low"];
                double volume_24h = order_book_response["stats"]["volume"];
                double index_price = order_book_response["index_price"];
                double mark_price = order_book_response["mark_price"];
                double funding_rate = order_book_response["funding_8h"];

                std::cout << "\n=========================================" << std::endl;
                std::cout << "📖 ORDER BOOK SUMMARY - " << instrument << std::endl;
                std::cout << "=========================================" << std::endl;
                std::cout << "💰 Best Ask (Sell):  " << best_ask << " BTC" << std::endl;
                std::cout << "📉 Best Bid (Buy):   " << best_bid << " BTC" << std::endl;
                std::cout << "📊 Last Traded Price: " << last_price << " BTC" << std::endl;
                std::cout << "📈 24h High:  " << high_24h << " BTC" << std::endl;
                std::cout << "📉 24h Low:   " << low_24h << " BTC" << std::endl;
                std::cout << "📊 24h Volume: " << volume_24h << " BTC" << std::endl;
                std::cout << "📌 Index Price: " << index_price << " BTC" << std::endl;
                std::cout << "📌 Mark Price: " << mark_price << " BTC" << std::endl;
                std::cout << "💸 Funding Rate (8h): " << funding_rate << std::endl;
                std::cout << "=========================================\n" << std::endl;
            }
            break;
            case 5:
                std::cout << "🔍 Fetching current positions...\n";
                api.get_current_positions(access_token);
            break;
            
            case 6:
                std::cout << "🔴 Exiting program... Goodbye!" << std::endl;
            break;
            
            default:
                std::cerr << "\u26A0 Invalid choice! Please enter a valid option.\n";
                break;
        }
    }
    
    return 0;
}



