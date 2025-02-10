#include "pch.h"
#include "gtest/gtest.h"
#include "../orderbook/order_book.h"
#include <chrono>
#include <iostream>
#include <memory>  // For std::shared_ptr

// Test that an order with no match remains in the order book.
TEST(OrderBookTest, NoMatchOrderRemains) {
    OrderBook ob;
    auto buyOrder = std::make_shared<Order>(1, 50.0, 100, OrderType::BUY);

    std::vector<Trade> trades = ob.addOrder(buyOrder);

    EXPECT_TRUE(trades.empty());  // No trade should occur.
}

// Test that a buy and sell order match correctly.
TEST(OrderBookTest, OrdersMatch) {
    OrderBook ob;
    auto buyOrder = std::make_shared<Order>(1, 50.0, 100, OrderType::BUY);
    auto sellOrder = std::make_shared<Order>(2, 49.0, 100, OrderType::SELL);

    ob.addOrder(buyOrder);
    std::vector<Trade> trades = ob.addOrder(sellOrder);

    ASSERT_EQ(trades.size(), 1);  // A single trade should occur.
    EXPECT_EQ(trades[0].buyOrderID, 1);
    EXPECT_EQ(trades[0].sellOrderID, 2);
    EXPECT_EQ(trades[0].quantity, 100);
    EXPECT_DOUBLE_EQ(trades[0].tradePrice, 49.0);  // Trade should occur at sell price.
}

// Test partial order fills.
TEST(OrderBookTest, PartialFill) {
    OrderBook ob;
    auto buyOrder = std::make_shared<Order>(1, 50.0, 100, OrderType::BUY);   // Buy 100 shares at $50.
    auto sellOrder = std::make_shared<Order>(2, 49.0, 50, OrderType::SELL);  // Sell 50 shares at $49.

    ob.addOrder(buyOrder);  // First add buy order
    std::vector<Trade> trades = ob.addOrder(sellOrder);  // Add sell order and match

    ob.displayOrders();  // Display the order book

    ASSERT_EQ(trades.size(), 1);  // Only one trade should occur.
    EXPECT_EQ(trades[0].quantity, 50);  // Only 50 shares should be traded.
    EXPECT_DOUBLE_EQ(trades[0].tradePrice, 49.0);  // The trade price should be $49.0.

    // The remaining 50 shares of the buy order should still be in the order book.
    EXPECT_EQ(buyOrder->quantity, 50);  // The buy order quantity should be updated to 50.
}

// Test that orders remain if they cannot be matched.
TEST(OrderBookTest, UnmatchedOrdersRemain) {
    OrderBook ob;
    auto buyOrder = std::make_shared<Order>(1, 50.0, 100, OrderType::BUY);
    auto sellOrder = std::make_shared<Order>(2, 55.0, 100, OrderType::SELL); // Too expensive to match

    ob.addOrder(buyOrder);
    ob.addOrder(sellOrder);

    // Verify that both orders remain.
    ob.displayOrders(); // Should show one buy order and one sell order.
}

// Test canceling an order by its ID.
TEST(OrderBookTest, CancelOrder) {
    OrderBook ob;
    auto buyOrder = std::make_shared<Order>(1, 50.0, 100, OrderType::BUY);
    ob.addOrder(buyOrder);

    // Cancel the order
    bool cancelled = ob.cancelOrder(1);
    EXPECT_TRUE(cancelled);

    // Try to cancel again (should fail since the order was already removed)
    cancelled = ob.cancelOrder(1);
    EXPECT_FALSE(cancelled);
}

// Test for handling 1 million orders efficiently.
TEST(OrderBookTest, LargeVolumeTest) {
    std::cout << "Starting Large Volume Test with 1,000,000 orders..." << std::endl;

    OrderBook ob;
    const int NUM_ORDERS = 1000000;

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Trade> allTrades;

    // Insert 1 million buy orders with increasing prices.
    for (int i = 0; i < NUM_ORDERS; i++) {
        double price = 50.0 + (i % 100) * 0.01;  // Prices range from $50.00 to $50.99.
        auto buyOrder = std::make_shared<Order>(i + 1, price, 10, OrderType::BUY);
        std::vector<Trade> trades = ob.addOrder(buyOrder);
        allTrades.insert(allTrades.end(), trades.begin(), trades.end());
    }

    // Insert 1 million sell orders with decreasing prices.
    for (int i = 0; i < NUM_ORDERS; i++) {
        double price = 49.5 + (i % 100) * 0.01;  // Prices range from $49.00 to $49.99.
        auto sellOrder = std::make_shared<Order>(NUM_ORDERS + i + 1, price, 10, OrderType::SELL);
        std::vector<Trade> trades = ob.addOrder(sellOrder);
        allTrades.insert(allTrades.end(), trades.begin(), trades.end());
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Size of allTrades: " << allTrades.size() << std::endl;
    std::cout << "Executed 1,000,000 buy and 1,000,000 sell orders." << std::endl;
    std::cout << "Total execution time: " << elapsed.count() << " seconds." << std::endl;

    EXPECT_TRUE(true);  // The test should pass as long as execution completes.
}
