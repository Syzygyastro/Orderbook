#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <vector>
#include <chrono>

enum class OrderType {
    BUY,
    SELL
};

struct Trade {
    int buyOrderID;
    int sellOrderID;
    int quantity;
    double tradePrice;
};

class Order {
public:
    int orderID;
    double price;
    int quantity;
    OrderType orderType;
    std::chrono::system_clock::time_point timestamp;

    // Constructor.
    Order(int id, double p, int qty, OrderType type);

    void displayOrder() const;
};

class OrderBook {
private:
    std::vector<Order> orders;

public:
    void addOrder(const Order& order);

    void displayOrders() const;

    std::vector<Trade> matchOrders();
};

#endif // ORDER_BOOK_H
