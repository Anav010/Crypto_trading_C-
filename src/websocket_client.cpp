#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include "../include/json.hpp"

using namespace boost::asio;
using namespace boost::beast;
using json = nlohmann::json;

class WebSocketClient {
public:
    WebSocketClient(io_context& ioc) : resolver_(ioc), ws_(ioc) {}

    void connect(const std::string& host, const std::string& port, const std::string& path) {
        auto results = resolver_.resolve(host, port);
        boost::asio::connect(ws_.next_layer(), results.begin(), results.end());
        ws_.handshake(host, path);
        std::cout << "✅ Connected to Deribit WebSocket!" << std::endl;
    }

    void subscribe_order_book(const std::string& instrument) {
        json request = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "public/subscribe"},
            {"params", {
                {"channels", {"book." + instrument + ".100ms"}}
            }}
        };

        ws_.write(boost::asio::buffer(request.dump()));
        std::cout << "📡 Subscribed to Order Book for " << instrument << std::endl;
        listen();
    }

    void subscribe_order_updates() {
        json request = {
            {"jsonrpc", "2.0"},
            {"id", 2},
            {"method", "private/subscribe"},
            {"params", {
                {"channels", {"user.orders.BTC-PERPETUAL.raw"}}
            }}
        };

        ws_.write(boost::asio::buffer(request.dump()));
        std::cout << "📡 Subscribed to Order Updates!" << std::endl;
        listen();
    }

private:
    ip::tcp::resolver resolver_;
    websocket::stream<ip::tcp::socket> ws_;

    void listen() {
        while (true) {
            flat_buffer buffer;
            ws_.read(buffer);
            std::string response = boost::beast::buffers_to_string(buffer.data());
            json parsed = json::parse(response);
            std::cout << "🔹 Update: " << parsed.dump(4) << std::endl;
        }
    }
};
