#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include "json.hpp"

using json = nlohmann::json;
using namespace boost::asio;
using namespace boost::beast;

class WebSocketClient {
public:
    WebSocketClient(io_context& ioc);

    void connect(const std::string& host, const std::string& port, const std::string& path);
    void subscribe_order_book(const std::string& instrument);
    void subscribe_order_updates();

private:
    ip::tcp::resolver resolver_;
    websocket::stream<ip::tcp::socket> ws_;
    void listen();
};

#endif // WEBSOCKET_CLIENT_H