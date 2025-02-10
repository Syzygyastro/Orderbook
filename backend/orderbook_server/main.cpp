#include "crow.h"                 // Main Crow header
#include "crow/middlewares/cors.h"            // CORSHandler and CORSRules
#include "../orderbook/order_book.h"
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <thread>

// Instead of crow::SimpleApp, we define an App with CORSHandler
using MyCORSApp = crow::App<crow::CORSHandler>;

std::mutex connection_mutex;
std::unordered_set<crow::websocket::connection*> active_connections;
std::atomic<bool> running{ true };
std::mutex orderBookMutex;

// Global OrderBook instance
OrderBook globalOrderBook;

// -----------------------------------------------------------------------------
// Helper function: convert the raw order book data to a single crow::json::wvalue
// -----------------------------------------------------------------------------
crow::json::wvalue convertOrderBookToJson(
    const std::vector<std::shared_ptr<Order>>& buyOrders,
    const std::vector<std::shared_ptr<Order>>& sellOrders)
{
    crow::json::wvalue result;
    crow::json::wvalue::list bids;
    crow::json::wvalue::list asks;

    // Convert buy orders to JSON array
    for (const auto& order : buyOrders)
    {
        crow::json::wvalue orderJson;
        orderJson["orderID"] = order->orderID;
        orderJson["price"] = order->price;
        orderJson["quantity"] = order->quantity;
        bids.push_back(std::move(orderJson));
    }

    // Convert sell orders to JSON array
    for (const auto& order : sellOrders)
    {
        crow::json::wvalue orderJson;
        orderJson["orderID"] = order->orderID;
        orderJson["price"] = order->price;
        orderJson["quantity"] = order->quantity;
        asks.push_back(std::move(orderJson));
    }

    result["bids"] = std::move(bids);
    result["asks"] = std::move(asks);
    return result;
}

// -----------------------------------------------------------------------------
// Broadcast entire order book to all WebSocket clients
// -----------------------------------------------------------------------------
void broadcastOrderBookUpdate()
{
    // First, gather the current state under a lock
    std::lock_guard<std::mutex> lock(orderBookMutex);
    auto [buyOrders, sellOrders] = globalOrderBook.getRawOrderBookData();

    // Build a single JSON object for the update
    crow::json::wvalue updateMsg;
    updateMsg["status"] = "update";
    updateMsg["data"] = convertOrderBookToJson(buyOrders, sellOrders);

    // Serialize once
    std::string jsonStr = updateMsg.dump();

    // Now send to all active connections
    std::lock_guard<std::mutex> connLock(connection_mutex);
    for (auto conn : active_connections)
    {
        conn->send_text(jsonStr);
    }
}

int main()
{
    // Create an App that uses CORSHandler as middleware
    MyCORSApp app;

    // Configure the CORS settings
    //   - "global()" applies these rules to all routes
    //   - "origin("*") allows any origin to access
    //   - "methods(...)" defines which HTTP methods are permitted
    //   - "allow_credentials(false)" typically used to permit cookies if needed
    //   - You can also specify "headers(...)" or other rules
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("*")                                        // Allow all origins
        .methods("GET"_method, "POST"_method, "DELETE"_method, "OPTIONS"_method)
        .allow_credentials();

    // WebSocket for real-time order book
    CROW_WEBSOCKET_ROUTE(app, "/orderbook")
        .onopen([&](crow::websocket::connection& conn) {
            {
                std::lock_guard<std::mutex> lock(connection_mutex);
                active_connections.insert(&conn);
                CROW_LOG_INFO << "New WebSocket connection. Total: "
                    << active_connections.size();
            }

            // Send an initial snapshot to the newly connected client
            std::lock_guard<std::mutex> lock(orderBookMutex);
            auto [buyOrders, sellOrders] = globalOrderBook.getRawOrderBookData();

            // Build the snapshot
            crow::json::wvalue snapshot;
            snapshot["type"] = "snapshot";

            // Convert the entire order book to JSON and store in the snapshot
            crow::json::wvalue obJson = convertOrderBookToJson(buyOrders, sellOrders);
            snapshot["bids"] = std::move(obJson["bids"]);
            snapshot["asks"] = std::move(obJson["asks"]);

            // Send the snapshot
            conn.send_text(snapshot.dump());
            })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
        std::lock_guard<std::mutex> lock(connection_mutex);
        active_connections.erase(&conn);
        CROW_LOG_INFO << "WebSocket disconnected: " << reason
            << ". Total now: " << active_connections.size();
            })
        .onmessage([&](crow::websocket::connection& /*conn*/, const std::string& data, bool is_binary) {
        if (!is_binary) {
            CROW_LOG_INFO << "Received WebSocket message: " << data;
        }
            });

    // POST /api/orders  -> Add a New Order
    CROW_ROUTE(app, "/api/orders")
        .methods("POST"_method)
        ([&](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        int orderID = body["orderID"].i();
        double price = body["price"].d();
        int quantity = body["quantity"].i();
        std::string type = body["orderType"].s();

        OrderType orderType = (type == "buy") ? OrderType::BUY : OrderType::SELL;
        auto order = std::make_shared<Order>(orderID, price, quantity, orderType);

        std::vector<Trade> trades;
        {
            std::lock_guard<std::mutex> lock(orderBookMutex);
            trades = globalOrderBook.addOrder(order);
        }

        // Broadcast the updated order book
        broadcastOrderBookUpdate();

        // Return executed trades as JSON
        crow::json::wvalue result;
        crow::json::wvalue::list trades_list;

        for (const auto& t : trades) {
            crow::json::wvalue trade;
            trade["buyOrderID"] = t.buyOrderID;
            trade["sellOrderID"] = t.sellOrderID;
            trade["quantity"] = t.quantity;
            trade["tradePrice"] = t.tradePrice;
            trades_list.push_back(std::move(trade));
        }
        result["trades"] = std::move(trades_list);
        return crow::response(result);
            });

    // GET /api/orderbook -> Retrieve the entire Order Book
    CROW_ROUTE(app, "/api/orderbook")
        .methods("GET"_method)
        ([&]() {
        std::lock_guard<std::mutex> lock(orderBookMutex);
        auto [buyOrders, sellOrders] = globalOrderBook.getRawOrderBookData();
        auto resultJson = convertOrderBookToJson(buyOrders, sellOrders);
        return crow::response(resultJson);
            });

    // DELETE /api/order/<int> -> Cancel an order by ID
    CROW_ROUTE(app, "/api/order/<int>")
        .methods("DELETE"_method)
        ([&](int id) {
        bool cancelled;
        {
            std::lock_guard<std::mutex> lock(orderBookMutex);
            cancelled = globalOrderBook.cancelOrder(id);
        }

        if (cancelled) {
            broadcastOrderBookUpdate();
            return crow::response(200, "Order cancelled");
        }
        else {
            return crow::response(404, "Order not found");
        }
            });

    // Start the Crow server on port 8080
    app.port(8080).multithreaded().run();

    running = false;
    return 0;
}
