#include "order_book.h"
#include <iostream>

//int main() {
//    // Create an instance of the OrderBook.
//    OrderBook orderBook;
//
//    // Create some sample orders:
//    // - Buy Order: 100 shares at $50.
//    Order order1(1, 50.0, 100, OrderType::BUY);
//    // - Sell Order: 50 shares at $48.
//    Order order2(2, 48.0, 50, OrderType::SELL);
//    // - Sell Order: 200 shares at $52.
//    Order order3(3, 52.0, 200, OrderType::SELL);
//
//    // Add orders to the order book.
//    orderBook.addOrder(order1);
//    orderBook.addOrder(order2);
//    orderBook.addOrder(order3);
//
//    // Display current orders.
//    std::cout << "Displaying Orders in the Order Book:\n";
//    orderBook.displayOrders();
//
//    // Perform order matching.
//    std::vector<Trade> trades = orderBook.matchOrders();
//
//    // Display the resulting trades.
//    std::cout << "\n--- Trades Executed ---\n";
//    if (trades.empty()) {
//        std::cout << "No trades executed." << std::endl;
//    }
//    else {
//        for (const auto& trade : trades) {
//            std::cout << "Trade executed: " << trade.quantity
//                << " shares at $" << trade.tradePrice
//                << " (Buy Order ID: " << trade.buyOrderID
//                << ", Sell Order ID: " << trade.sellOrderID << ")\n";
//        }
//    }
//
//    return 0;
//}
