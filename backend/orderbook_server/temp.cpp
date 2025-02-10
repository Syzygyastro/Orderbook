//// main.cpp
//#define CROW_ENABLE_WEBSOCKET
//#include "crow/app.h"         // Crow application header
//#include "crow/json.h"        // Crow JSON support header
//#include "crow/websocket.h"   // Crow WebSocket support header
//#include "crow/logging.h"     // Crow logging macros
//#include "../orderbook/order_book.h"  // Your order book header (using shared_ptr)
//#include <mutex>
//#include <algorithm>
//#include <vector>
//#include <iostream>
//#include <sstream>
//#include <functional>
//#include <unordered_set>
//
//using namespace std;
//
//// Helper: Make a copyable lambda from a lambda (wrap in std::function).
//template<typename Signature, typename Func>
//std::function<Signature> make_copyable_lambda(Func&& f)
//{
//    return std::function<Signature>(std::forward<Func>(f));
//}
//
//// Global order book instance and mutex for thread safety.
//OrderBook globalOrderBook;
//std::mutex orderBookMutex;
//
//// Global vector to store active WebSocket connections and its mutex.
//std::unordered_set<crow::websocket::connection*> wsConnections;
//std::mutex wsMutex;
//
//// Helper: Broadcast updated order book data to all connected WebSocket clients.
//void broadcastOrderBookUpdate()
//{
//    crow::json::wvalue message;
//    message["status"] = "update";
//    message["data"] = "orderbook updated"; // Replace with a full JSON representation if desired.
//    std::string jsonStr = message.dump();
//
//    std::lock_guard<std::mutex> lock(wsMutex);
//    for (auto conn : wsConnections)
//    {
//        conn->send_text(jsonStr);
//    }
//}
//
//// Dummy function to convert the order book to JSON.
//// Replace this stub with your actual order-book serialization.
//crow::json::wvalue orderBookToJson(const OrderBook& ob)
//{
//    crow::json::wvalue result;
//    result["info"] = "order book JSON here";
//    return result;
//}
//
//int main()
//{
//    crow::SimpleApp app;
//
//    // ---------------------------------------------------
//    // HTTP POST /order endpoint: Add a new order.
//    // Expected JSON format:
//    // {
//    //   "orderID": 1,
//    //   "price": 50.0,
//    //   "quantity": 100,
//    //   "orderType": "buy"  // or "sell"
//    // }
//    // ---------------------------------------------------
//    auto post_order_handler = [&](const crow::request& req) -> crow::response {
//        auto body = crow::json::load(req.body);
//        if (!body)
//        {
//            return crow::response(400, "Invalid JSON");
//        }
//        int orderID = body["orderID"].i();
//        double price = body["price"].d();
//        int quantity = body["quantity"].i();
//        std::string typeStr = body["orderType"].s();
//        OrderType type = (typeStr == "buy" ? OrderType::BUY : OrderType::SELL);
//
//        // Create order using shared_ptr.
//        auto order = std::make_shared<Order>(orderID, price, quantity, type);
//
//        std::vector<Trade> trades;
//        {
//            std::lock_guard<std::mutex> lock(orderBookMutex);
//            trades = globalOrderBook.addOrder(order);
//        }
//        // Broadcast an update via WebSocket.
//        broadcastOrderBookUpdate();
//
//        // Build JSON response for executed trades.
//        crow::json::wvalue result;
//        crow::json::wvalue::list trades_list;
//        for (const auto& t : trades)
//        {
//            crow::json::wvalue trade;
//            trade["buyOrderID"] = t.buyOrderID;
//            trade["sellOrderID"] = t.sellOrderID;
//            trade["quantity"] = t.quantity;
//            trade["tradePrice"] = t.tradePrice;
//            trades_list.push_back(trade);
//        }
//        result["trades"] = std::move(trades_list);
//        return crow::response{ result };
//        };
//    CROW_ROUTE(app, "/order").methods("POST"_method)(post_order_handler);
//
//    // ---------------------------------------------------
//    // HTTP GET /orderbook endpoint: Retrieve the current order book.
//    // ---------------------------------------------------
//    auto get_orderbook_handler = [&]() -> crow::response {
//        std::lock_guard<std::mutex> lock(orderBookMutex);
//        return crow::response{ orderBookToJson(globalOrderBook) };
//        };
//    CROW_ROUTE(app, "/orderbook").methods("GET"_method)(get_orderbook_handler);
//
//    // ---------------------------------------------------
//    // HTTP DELETE /order/<id> endpoint: Cancel an order by its ID.
//    // ---------------------------------------------------
//    auto delete_order_handler = [&](int id) -> crow::response {
//        bool cancelled;
//        {
//            std::lock_guard<std::mutex> lock(orderBookMutex);
//            cancelled = globalOrderBook.cancelOrder(id);
//        }
//        if (cancelled)
//        {
//            broadcastOrderBookUpdate();
//            return crow::response(200, "Order cancelled");
//        }
//        else
//        {
//            return crow::response(404, "Order not found");
//        }
//        };
//    CROW_ROUTE(app, "/order/<int>").methods("DELETE"_method)(delete_order_handler);
//
//    // ---------------------------------------------------
//    // WebSocket /ws endpoint: Streams real-time order book updates.
//    // ---------------------------------------------------
//    //CROW_WEBSOCKET_ROUTE(app, "/ws")
//    //    .onopen([&](crow::websocket::connection& conn) {
//    //    CROW_LOG_INFO << "New websocket connection from " << conn.get_remote_ip();
//    //    std::lock_guard<std::mutex> lock(wsMutex);
//    //    wsConnections.insert(&conn);
//    //        })
//    //    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
//    //    CROW_LOG_INFO << "WebSocket connection closed: " << reason << " (Code: " << code << ")";
//    //    std::lock_guard<std::mutex> lock(wsMutex);
//    //    wsConnections.erase(&conn);
//    //        })
//    //    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
//    //    std::lock_guard<std::mutex> lock(wsMutex);
//    //    for (auto u : wsConnections)
//    //    {
//    //        if (is_binary)
//    //            u->send_binary(data);
//    //        else
//    //            u->send_text(data);
//    //    }
//    //        });
//
//    CROW_WEBSOCKET_ROUTE(app, "/ws")
//        .onopen([](crow::websocket::connection& conn) {
//        std::cout << "WebSocket opened!" << std::endl;
//            })
//        .onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
//        std::cout << "WebSocket closed: " << reason << " (Code: " << code << ")" << std::endl;
//            });
//
//    CROW_ROUTE(app, "/")
//        ([] {
//        char name[256];
//        gethostname(name, 256);
//        crow::mustache::context x;
//        x["servername"] = name;
//
//        auto page = crow::mustache::load("ws.html");
//        return page.render(x);
//            });
//
//    // Start the server on port 8080 (multithreaded).
//	std::cout << "Starting server on port 8080..." << std::endl;
//    app.port(40080).multithreaded().run();
//	std::cout << "Server started." << std::endl;
//    return 0;
//}
