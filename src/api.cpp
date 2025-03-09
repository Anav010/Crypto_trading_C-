#include "../include/api.h"
#include <iostream>
#include <curl/curl.h>
#include "../include/json.hpp"

using json = nlohmann::json;

API::API(const std::string& client_id, const std::string& client_secret) 
    : client_id(client_id), client_secret(client_secret) {}

// Safe Write Callback Function
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* response = static_cast<std::string*>(userdata);
    if (!response) return 0;
    response->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

json API::send_post_request(const std::string& url, const json& data, const std::string& access_token) {
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
    if (!curl) {
        return {{"error", "CURL initialization failed"}};
    }

    std::string json_data = data.dump();
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + access_token).c_str());

    std::string response_string;
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl.get());
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        return {{"error", curl_easy_strerror(res)}};
    }

    if (response_string.empty()) {
        return {{"error", "Empty response"}};
    }

    try {
        return json::parse(response_string);
    } catch (const json::parse_error& e) {
        return {{"error", "JSON Parse Error"}, {"details", e.what()}, {"raw_response", response_string}};
    }
}

std::string API::authenticate() {
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
    if (!curl) {
        return "";
    }

    std::string url = "https://test.deribit.com/api/v2/public/auth";
    json json_data = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"client_id", client_id},
            {"client_secret", client_secret},
            {"scope", "trade:read_write"}
        }}
    };

    std::string post_data = json_data.dump();
    std::string response;
    struct curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POST, 1);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl.get());
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        return "";
    }

    try {
        json json_response = json::parse(response);
        if (json_response.contains("result") && json_response["result"].contains("access_token")) {
            return json_response["result"]["access_token"];
        }
    } catch (const json::parse_error& e) {
        return "";
    }
    return "";
}

std::string API::place_order(const std::string& access_token, const std::string& instrument, int amount, const std::string& type, double price) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize cURL" << std::endl;
        return "";
    }

    std::string url = "https://test.deribit.com/api/v2/private/" + std::string(type == "limit" ? "buy" : "sell");

    json json_data = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", type == "limit" ? "private/buy" : "private/sell"},
        {"params", {
            {"instrument_name", instrument},
            {"amount", amount},
            {"type", type}  // ✅ Send type explicitly
        }}
    };
    
    // ✅ Only include price if it's a limit order
    if (type == "limit") {
        json_data["params"]["price"] = price;
    }
    

    std::string post_data = json_data.dump();
    
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + access_token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "cURL request failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    std::cout << "Raw Order Response: " << response << std::endl;

    try {
        json json_response = json::parse(response);

        // ✅ Debug: Print full response
        std::cout << "Parsed Order Response: " << json_response.dump(4) << std::endl;

        // ✅ Check if response contains expected fields
        if (json_response.contains("result")) {
            if (json_response["result"].contains("order_id")) {
                return json_response["result"]["order_id"].get<std::string>();
            } else if (json_response["result"].contains("order") && json_response["result"]["order"].contains("order_id")) {
                return json_response["result"]["order"]["order_id"].get<std::string>();
            }
        }

        std::cerr << "Error: 'order_id' missing in response!" << std::endl;
        return "";

    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << "\nResponse: " << response << std::endl;
        return "";
    }
}


json API::cancel_order(const std::string &access_token, const std::string &order_id) {
    std::string url = "https://test.deribit.com/api/v2/private/cancel";

    json json_data = {
        {"jsonrpc", "2.0"},
        {"id", 2},
        {"method", "private/cancel"},
        {"params", {
            {"order_id", order_id}
        }}
    };

    json response = send_post_request(url, json_data, access_token);

    // ✅ Print response for debugging
    std::cout << "Cancel Order Raw Response: " << response.dump(4) << std::endl;

    // ✅ Handle error cases
    if (!response.contains("result")) {
        std::cerr << "❌ Error: 'result' field missing in cancel order response." << std::endl;
        return {{"error", "Invalid response structure"}};
    }

    try {
        std::string canceled_order_id = response["result"]["order_id"];
        std::string order_state = response["result"]["order_state"];
        std::cout << "✅ Order " << canceled_order_id << " is now " << order_state << std::endl;
        return response;
    } catch (json::exception& e) {
        std::cerr << "❌ JSON Exception: " << e.what() << std::endl;
        return {{"error", "JSON parsing failed"}};
    }
}

json API::modify_order(const std::string& access_token, const std::string& order_id, int new_amount, double new_price) {
    std::string url = "https://test.deribit.com/api/v2/private/edit";

    json json_data = {
        {"jsonrpc", "2.0"},
        {"id", 3},
        {"method", "private/edit"},
        {"params", {
            {"order_id", order_id},
            {"amount", new_amount},
            {"price", new_price}
        }}
    };

    json response = send_post_request(url, json_data, access_token);

    std::cout << "Modify Order Raw Response: " << response.dump(4) << std::endl;

    // ✅ Ensure response contains "result"
    if (!response.contains("result")) {
        std::cerr << "❌ Error: 'result' field missing in modify order response." << std::endl;
        return {{"error", "Invalid response structure"}};
    }

    return response;
}

json API::get_order_book(const std::string& instrument_name) {
    std::string url = "https://test.deribit.com/api/v2/public/get_order_book";

    json json_data = {
        {"jsonrpc", "2.0"},
        {"id", 3},
        {"method", "public/get_order_book"},
        {"params", {
            {"instrument_name", instrument_name},
            {"depth", 10}  // Get top 10 bids/asks
        }}
    };

    json response = send_post_request(url, json_data, ""); // No access token needed for public API

    // Debugging: Print raw response
    std::cout << "📖 Raw Order Book Response:\n" << response.dump(4) << std::endl;

    // ✅ Validate response
    if (!response.contains("result")) {
        std::cerr << "❌ Error: Failed to retrieve order book!\n";
        return {{"error", "Invalid response"}};
    }

    json result = response["result"];

    return result;
}

json API::get_current_positions(const std::string& access_token) {
    std::string url = "https://test.deribit.com/api/v2/private/get_positions";

    json json_data = {
        {"jsonrpc", "2.0"},
        {"id", 4},
        {"method", "private/get_positions"},
        {"params", {
            {"currency", "BTC"}
        }}
    };

    json response = send_post_request(url, json_data, access_token);

    // 🛠 Validate response
    if (!response.contains("result")) {
        std::cerr << "❌ Error: Unable to fetch positions.\n";
        return {{"error", "Invalid response"}};
    }

    json positions = response["result"];

    // 📊 Display summary
    std::cout << "\n📌 **Open Positions Summary**\n";
    if (positions.empty()) {
        std::cout << "✅ No open positions.\n";
    } else {
        for (const auto& pos : positions) {
            std::string instrument = pos.value("instrument_name", "N/A");
            double size = pos.value("size", 0.0);
            double entry_price = pos.value("average_price", 0.0);
            double mark_price = pos.value("mark_price", 0.0);
            double unrealized_pnl = pos.value("floating_profit_loss", 0.0);
            std::string direction = (size > 0) ? "LONG 🟢" : "SHORT 🔴";

            std::cout << "--------------------------------------\n";
            std::cout << "📍 Instrument: " << instrument << "\n";
            std::cout << "📏 Size: " << size << " contracts (" << direction << ")\n";
            std::cout << "🎯 Entry Price: " << entry_price << "\n";
            std::cout << "📊 Mark Price: " << mark_price << "\n";
            std::cout << "💰 Unrealized PnL: " 
                      << (unrealized_pnl >= 0 ? "🟢 " : "🔴 ") << unrealized_pnl << "\n";
        }
        std::cout << "--------------------------------------\n";
    }

    return positions;
}
