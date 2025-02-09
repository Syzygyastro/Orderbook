#include "pch.h"
#include "gtest/gtest.h"
#include "../orderbook/order_book.h"

// Test for partial fill.
TEST(OrderBookTest, PartialFill) {
    OrderBook ob;
    // Buy order: 100 shares at $50.
    Order buyOrder(1, 50.0, 100, OrderType::BUY);
    // Sell order: 50 shares at $48.
    Order sellOrder(2, 48.0, 50, OrderType::SELL);

    ob.addOrder(buyOrder);
    ob.addOrder(sellOrder);

    std::vector<Trade> trades = ob.matchOrders();

    ASSERT_EQ(trades.size(), 1);  // One trade should occur.
    EXPECT_EQ(trades[0].quantity, 50);  // Verify trade quantity.
    EXPECT_DOUBLE_EQ(trades[0].tradePrice, 48.0);  // Verify trade price.

    // Remaining buy order should still exist with 50 shares.
    ob.displayOrders();  // Optional: Visual debug check.
}

// Test for multiple matches.
TEST(OrderBookTest, MultipleMatches) {
    OrderBook ob;
    // Buy order: 100 shares at $50.
    Order buyOrder(1, 50.0, 100, OrderType::BUY);
    // Sell order 1: 50 shares at $48.
    Order sellOrder1(2, 48.0, 50, OrderType::SELL);
    // Sell order 2: 50 shares at $49.
    Order sellOrder2(3, 49.0, 50, OrderType::SELL);

    ob.addOrder(buyOrder);
    ob.addOrder(sellOrder1);
    ob.addOrder(sellOrder2);

    std::vector<Trade> trades = ob.matchOrders();

    ASSERT_EQ(trades.size(), 2);  // Two trades should occur.

    // Verify first trade.
    EXPECT_EQ(trades[0].quantity, 50);  // First trade quantity.
    EXPECT_DOUBLE_EQ(trades[0].tradePrice, 48.0);  // First trade price.
    EXPECT_EQ(trades[0].buyOrderID, 1);  // Buy Order ID for the first trade.
    EXPECT_EQ(trades[0].sellOrderID, 2);  // Sell Order ID for the first trade.

    // Verify second trade.
    EXPECT_EQ(trades[1].quantity, 50);  // Second trade quantity.
    EXPECT_DOUBLE_EQ(trades[1].tradePrice, 49.0);  // Second trade price.
    EXPECT_EQ(trades[1].buyOrderID, 1);  // Buy Order ID for the second trade.
    EXPECT_EQ(trades[1].sellOrderID, 3);  // Sell Order ID for the second trade.

    // Ensure no remaining orders.
    ob.displayOrders();
}

// Test for handling 1 million orders.
TEST(OrderBookTest, LargeVolumeTest) {
    OrderBook ob;

    const int NUM_ORDERS = 1000000;

    for (int i = 0; i < NUM_ORDERS; i++) {
        double price = 50.0 + (i % 100) * 0.01;
        ob.addOrder(Order(i + 1, price, 10, OrderType::BUY));
    }

    for (int i = NUM_ORDERS; i < 2 * NUM_ORDERS; i++) {
        double price = 49.0 + (i % 100) * 0.01;
        ob.addOrder(Order(i + 1, price, 10, OrderType::SELL));
    }

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Trade> trades = ob.matchOrders();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Executed " << trades.size() << " trades in " << elapsed.count() << " seconds." << std::endl;

    ASSERT_EQ(trades.size(), NUM_ORDERS);
}
