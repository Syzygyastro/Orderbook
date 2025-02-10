#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <map>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <iostream>
#include <memory>  // For std::shared_ptr

// Enum to represent Buy or Sell order type.
enum class OrderType {
    BUY,
    SELL
};

// Structure to represent a trade execution.
struct Trade {
    int buyOrderID;    // ID of the buy order involved in the trade
    int sellOrderID;   // ID of the sell order involved in the trade
    int quantity;      // Number of shares traded
    double tradePrice; // Price at which the trade was executed
};

class Order {
public:
    int orderID;
    double price;
    int quantity;
    OrderType orderType;
    std::chrono::system_clock::time_point timestamp;

    // Default constructor (used for containers like map)
    Order() : orderID(0), price(0.0), quantity(0), orderType(OrderType::BUY), timestamp(std::chrono::system_clock::now()) {}

    // Parameterized constructor (used when explicitly creating orders)
    Order(int id, double p, int qty, OrderType type)
        : orderID(id), price(p), quantity(qty), orderType(type), timestamp(std::chrono::system_clock::now()) {
    }

    // Comparison functions for priority queues.
    struct BuyOrderComparator {
        bool operator()(const std::shared_ptr<Order>& a, const std::shared_ptr<Order>& b) const {
            return (a->price < b->price) || (a->price == b->price && a->timestamp > b->timestamp);
        }
    };

    struct SellOrderComparator {
        bool operator()(const std::shared_ptr<Order>& a, const std::shared_ptr<Order>& b) const {
            return (a->price > b->price) || (a->price == b->price && a->timestamp > b->timestamp);
        }
    };
};

// Class to represent the OrderBook.
class OrderBook {
private:
    // Using shared pointers to store orders by price and timestamp, for efficient order matching and cancellation.
    std::map<double, std::multimap<std::chrono::system_clock::time_point, std::shared_ptr<Order>>> buyOrders;
    std::map<double, std::multimap<std::chrono::system_clock::time_point, std::shared_ptr<Order>>> sellOrders;

    // A map to store orders by orderID, for efficient access during cancellation.
    std::unordered_map<int, std::shared_ptr<Order>> ordersById;

    // Match buy and sell orders when a new order is added.
    void matchOrders(std::shared_ptr<Order>& order, std::vector<Trade>& trades);

public:
    // Adds an order to the order book and returns a vector of trades that occurred.
    std::vector<Trade> addOrder(std::shared_ptr<Order>& order);

    // Cancels an order by its order ID.
    bool cancelOrder(int orderId);

    // Displays the current orders in the book.
    void displayOrders() const;
};

#endif // ORDER_BOOK_H
