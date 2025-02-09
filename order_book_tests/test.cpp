#include "pch.h"

#include "gtest/gtest.h"
#include "../orderbook/order_book.h"

// Test that no match occurs when buy price is lower than sell price.
TEST(OrderBookTest, NoMatchWhenPricesDoNotOverlap) {
    OrderBook ob;
    // Buy order at $40 and Sell order at $45.
    Order buyOrder(1, 40.0, 100, OrderType::BUY);
    Order sellOrder(2, 45.0, 100, OrderType::SELL);
    ob.addOrder(buyOrder);
    ob.addOrder(sellOrder);

    std::vector<Trade> trades = ob.matchOrders();
    EXPECT_TRUE(trades.empty());
}

// Test that matching occurs with valid price overlap.
TEST(OrderBookTest, MatchingOccursWithValidPrices) {
    OrderBook ob;
    // Create a scenario:
    // - Buy order: 100 shares at $50.
    // - Sell order 1: 50 shares at $48.
    // - Sell order 2: 50 shares at $49.
    Order buyOrder(1, 50.0, 100, OrderType::BUY);
    Order sellOrder1(2, 48.0, 50, OrderType::SELL);
    Order sellOrder2(3, 49.0, 50, OrderType::SELL);
    ob.addOrder(buyOrder);
    ob.addOrder(sellOrder1);
    ob.addOrder(sellOrder2);

    std::vector<Trade> trades = ob.matchOrders();
    ASSERT_EQ(trades.size(), 2);

    // Verify the first trade.
    EXPECT_EQ(trades[0].buyOrderID, 1);
    EXPECT_EQ(trades[0].sellOrderID, 2);
    EXPECT_EQ(trades[0].quantity, 50);
    EXPECT_DOUBLE_EQ(trades[0].tradePrice, 48.0);

    // Verify the second trade.
    EXPECT_EQ(trades[1].buyOrderID, 1);
    EXPECT_EQ(trades[1].sellOrderID, 3);
    EXPECT_EQ(trades[1].quantity, 50);
    EXPECT_DOUBLE_EQ(trades[1].tradePrice, 49.0);
}

// Main function for running tests.
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
