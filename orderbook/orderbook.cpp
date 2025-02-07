#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <algorithm>

// Enum to represent Buy or Sell order type
enum class OrderType {
    BUY,
    SELL
};

// Class to represent an Order
class Order {
public:
    int orderID;
    double price;
    int quantity;
    OrderType orderType;
    std::chrono::system_clock::time_point timestamp;

    Order(int id, double p, int qty, OrderType type)
        : orderID(id), price(p), quantity(qty), orderType(type), timestamp(std::chrono::system_clock::now()) {
    }

    // Function to display the order details
    void displayOrder() const {
        std::time_t timestamp_time = std::chrono::system_clock::to_time_t(timestamp);
        char timeStr[26]; // Buffer for the formatted time string
        // ctime_s is the safe version of ctime.
        errno_t err = ctime_s(timeStr, sizeof(timeStr), &timestamp_time);
        if (err == 0) {
            std::cout << "Order ID: " << orderID
                << ", Type: " << (orderType == OrderType::BUY ? "Buy" : "Sell")
                << ", Price: $" << price
                << ", Quantity: " << quantity
                << ", Timestamp: " << timeStr;
        }
        else {
            std::cerr << "Error converting time" << std::endl;
        }
    }
};

// Class to represent the OrderBook
class OrderBook {
private:
    std::vector<Order> orders;

public:
    // Method to add an order to the order book
    void addOrder(const Order& order) {
        orders.push_back(order);
    }

    // Method to display all orders in the order book
    void displayOrders() const {
        if (orders.empty()) {
            std::cout << "Order book is empty!" << std::endl;
            return;
        }

        for (const auto& order : orders) {
            order.displayOrder();
        }
    }

    void matchOrders() {
        std::vector<Order> buyOrders;
        std::vector<Order> sellOrders;
        for (const auto& order : orders) {
            if (order.orderType == OrderType::BUY) {
                buyOrders.push_back(order);
            }
            else {
                sellOrders.push_back(order);
            }
        }

        // Sort buy orders: highest price first, and for equal prices, earlier timestamp first
        std::sort(buyOrders.begin(), buyOrders.end(), [](const Order& a, const Order& b) {
            if (a.price == b.price)
                return a.timestamp < b.timestamp;
            return a.price > b.price;
            });

        // Sort sell orders: lowest price first, and for equal prices, earlier timestamp first
        std::sort(sellOrders.begin(), sellOrders.end(), [](const Order& a, const Order& b) {
            if (a.price == b.price)
                return a.timestamp < b.timestamp;
            return a.price < b.price;
            });

        std::cout << "\n--- Matching Orders ---\n";

        size_t i = 0, j = 0;

        while (i < buyOrders.size() && j < sellOrders.size()) {
            if (buyOrders[i].price >= sellOrders[j].price) {
                int tradeQuantity = std::min(buyOrders[i].quantity, sellOrders[j].quantity);
                double tradePrice = sellOrders[j].price;

                std::cout << "Trade executed: " << tradeQuantity
                    << " shares at $" << tradePrice
                    << " (Buy Order ID: " << buyOrders[i].orderID
                    << ", Sell Order ID: " << sellOrders[j].orderID << ")\n";

                buyOrders[i].quantity -= tradeQuantity;
                sellOrders[j].quantity -= tradeQuantity;

                if (buyOrders[i].quantity == 0) {
                    i++;
                }
                if (sellOrders[j].quantity == 0) {
                    j++;
                }
            }else{
                break;
            }

        }
        std::cout << "--- Matching Complete ---\n";
    }
};

int main() {
    OrderBook orderBook;

    Order order1(1, 50.0, 100, OrderType::BUY);
    Order order2(2, 48.0, 50, OrderType::SELL);
    Order order3(3, 52.0, 200, OrderType::SELL);

    orderBook.addOrder(order1);
    orderBook.addOrder(order2);
    orderBook.addOrder(order3);

    std::cout << "Displaying Orders in the Order Book:" << std::endl;
    orderBook.displayOrders();

    return 0;
}

