#include "order_book.h"
#include <iostream>
#include <ctime>
#include <algorithm>

// ---------------------
// Order Class Functions
// ---------------------

Order::Order(int id, double p, int qty, OrderType type)
    : orderID(id), price(p), quantity(qty), orderType(type),
    timestamp(std::chrono::system_clock::now()) {
}

void Order::displayOrder() const {
    // Convert timestamp to a C-string safely using ctime_s.
    std::time_t timeT = std::chrono::system_clock::to_time_t(timestamp);
    char timeStr[26]; // ctime_s requires a 26-character buffer.
    errno_t err = ctime_s(timeStr, sizeof(timeStr), &timeT);
    if (err == 0) {
        std::cout << "Order ID: " << orderID
            << ", Type: " << (orderType == OrderType::BUY ? "Buy" : "Sell")
            << ", Price: $" << price
            << ", Quantity: " << quantity
            << ", Timestamp: " << timeStr; // timeStr already ends with a newline.
    }
    else {
        std::cerr << "Error converting time" << std::endl;
    }
}

// -------------------------
// OrderBook Class Functions
// -------------------------

void OrderBook::addOrder(const Order& order) {
    orders.push_back(order);
}

void OrderBook::displayOrders() const {
    if (orders.empty()) {
        std::cout << "Order book is empty!" << std::endl;
        return;
    }
    for (const auto& order : orders) {
        order.displayOrder();
    }
}

std::vector<Trade> OrderBook::matchOrders() {
    std::vector<Trade> trades;

    // Separate orders into buy and sell lists.
    std::vector<Order> buyOrders;
    std::vector<Order> sellOrders;
    for (const auto& order : orders) {
        if (order.orderType == OrderType::BUY)
            buyOrders.push_back(order);
        else
            sellOrders.push_back(order);
    }

    // Sort buy orders: highest price first; if equal, the earlier order (smaller timestamp) comes first.
    std::sort(buyOrders.begin(), buyOrders.end(), [](const Order& a, const Order& b) {
        if (a.price == b.price)
            return a.timestamp < b.timestamp;
        return a.price > b.price;
        });

    // Sort sell orders: lowest price first; if equal, the earlier order comes first.
    std::sort(sellOrders.begin(), sellOrders.end(), [](const Order& a, const Order& b) {
        if (a.price == b.price)
            return a.timestamp < b.timestamp;
        return a.price < b.price;
        });

    // Matching loop.
    size_t i = 0, j = 0;
    while (i < buyOrders.size() && j < sellOrders.size()) {
        // A match is possible if the highest buy price is >= the lowest sell price.
        if (buyOrders[i].price >= sellOrders[j].price) {
            int tradeQuantity = std::min(buyOrders[i].quantity, sellOrders[j].quantity);
            double tradePrice = sellOrders[j].price;  // Using sell order's price for the trade.

            // Record the trade.
            trades.push_back({ buyOrders[i].orderID, sellOrders[j].orderID, tradeQuantity, tradePrice });

            // Adjust quantities.
            buyOrders[i].quantity -= tradeQuantity;
            sellOrders[j].quantity -= tradeQuantity;

            // Move to the next buy order if fully matched.
            if (buyOrders[i].quantity == 0)
                i++;
            // Move to the next sell order if fully matched.
            if (sellOrders[j].quantity == 0)
                j++;
        }
        else {
            // If the current highest buy order cannot match the current lowest sell order, break.
            break;
        }
    }
    return trades;
}
