#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <vector>
#include <chrono>

// Enum to represent Buy or Sell order type.
enum class OrderType {
    BUY,
    SELL
};

// Structure to represent a trade execution.
struct Trade {
    int buyOrderID;   // ID of the buy order involved in the trade
    int sellOrderID;  // ID of the sell order involved in the trade
    int quantity;     // Number of shares traded
    double tradePrice;// Price at which the trade was executed
};

// Class to represent an Order.
class Order {
public:
    int orderID;   // Unique identifier for the order
    double price;  // Price at which the order is placed
    int quantity;  // Number of shares
    OrderType orderType; // Type of order (Buy or Sell)
    std::chrono::system_clock::time_point timestamp; // Time when the order was created

    // Constructor.
    Order(int id, double p, int qty, OrderType type);

    // Displays the order details (for debugging/demo purposes).
    void displayOrder() const;
};

// Class to represent the OrderBook.
class OrderBook {
private:
    std::vector<Order> orders; // Container to hold orders

public:
    // Add an order to the order book.
    void addOrder(const Order& order);

    // Display all orders in the order book.
    void displayOrders() const;

    // Match orders using price-time priority and return a vector of executed trades.
    std::vector<Trade> matchOrders();
};

#endif // ORDER_BOOK_H
