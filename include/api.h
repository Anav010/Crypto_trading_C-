#ifndef API_H
#define API_H

#include <string>
#include "../include/json.hpp"  // ✅ Include JSON library

using json = nlohmann::json; // ✅ Define 'json' globally

class API {
public:
    API(const std::string& client_id, const std::string& client_secret);
    
    std::string authenticate();
    std::string place_order(const std::string& access_token, const std::string& instrument, int amount, const std::string& type, double price);
    json cancel_order(const std::string& access_token, const std::string& order_id);  // ✅ Now 'json' is recognized
    json send_post_request(const std::string& url, const json& data, const std::string& access_token);
    json modify_order(const std::string& access_token, const std::string& order_id, int new_amount, double new_price);
    json get_order_book(const std::string& instrument_name);
    json get_current_positions(const std::string& access_token);

private:
    std::string client_id;
    std::string client_secret;
};

#endif
