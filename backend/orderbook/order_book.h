#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <chrono>
#include <iostream>
#include <memory>

// Type aliases for clarity.
using Price = double;
using OrderId = int;

// Enum to represent order side.
enum class OrderType {
    BUY,
    SELL
};

// Structure to represent a trade execution.
struct Trade {
    OrderId buyOrderID;    // ID of the buy order involved in the trade
    OrderId sellOrderID;   // ID of the sell order involved in the trade
    int quantity;          // Number of shares traded
    Price tradePrice;      // Price at which the trade was executed
};

// Order class.
class Order {
public:
    OrderId orderID;
    Price price;
    int quantity;
    OrderType orderType;
    std::chrono::system_clock::time_point timestamp;

    Order()
        : orderID(0), price(0.0), quantity(0),
        orderType(OrderType::BUY),
        timestamp(std::chrono::system_clock::now()) {
    }

    Order(OrderId id, Price p, int qty, OrderType type)
        : orderID(id), price(p), quantity(qty),
        orderType(type),
        timestamp(std::chrono::system_clock::now()) {
    }

    // Getters for convenience
    OrderType GetSide() const { return orderType; }
    Price GetPrice() const { return price; }
    OrderId GetOrderId() const { return orderID; }
};

// For each price level, orders are kept in a list (to preserve FIFO ordering).
using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;

// When an order is inserted, we remember its location (iterator) in the list.
struct OrderEntry {
    OrderPointer order_{ nullptr };
    OrderPointers::iterator location_;
};

//
// The OrderBook class
//
class OrderBook {
private:
    // Bids: sorted by price descending (using std::greater).
    std::map<Price, OrderPointers, std::greater<Price>> bids_;

    // Asks: sorted by price ascending (using std::less, the default).
    std::map<Price, OrderPointers, std::less<Price>> asks_;

    // Map from order ID to OrderEntry (so we know exactly where the order is stored).
    std::unordered_map<OrderId, OrderEntry> orders_;

    // Internal matching routine.
    void matchOrders(OrderPointer order, std::vector<Trade>& trades);

public:
    // Adds an order to the order book. If the order is not fully matched, it is inserted.
    std::vector<Trade> addOrder(OrderPointer order);

    // Cancels an order by its order ID (using the stored iterator for fast removal).
    bool cancelOrder(OrderId orderId);

    // Displays the current order book.
    void displayOrders() const;

    // Returns raw order book data (for example, for JSON conversion).
    std::pair<std::vector<OrderPointer>, std::vector<OrderPointer>> getRawOrderBookData() const;
};

#endif // ORDER_BOOK_H
