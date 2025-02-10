#include "order_book.h"
#include <algorithm>
#include <iostream>
#include <memory>  // For std::make_shared


// -------------------------
// OrderBook Class Functions
// -------------------------
std::vector<Trade> OrderBook::addOrder(std::shared_ptr<Order>& order) {
    std::vector<Trade> trades;

    // Pass the order by reference to matchOrders
    matchOrders(order, trades);

    // After matching, if there are remaining quantities, add the order to the appropriate book
    if (order->orderType == OrderType::BUY && order->quantity > 0) {
        buyOrders[order->price].emplace(order->timestamp, order);
    }
    else if (order->orderType == OrderType::SELL && order->quantity > 0) {
        sellOrders[order->price].emplace(order->timestamp, order);
    }

    // Store the order by its ID for cancellation purposes
    ordersById[order->orderID] = order;

    return trades;
}

void OrderBook::matchOrders(std::shared_ptr<Order>& order, std::vector<Trade>& trades) {
    if (order->orderType == OrderType::BUY) {
        // An incoming BUY order will match with resting SELL orders.
        while (order->quantity > 0 && !sellOrders.empty()) {
            auto bestSell = sellOrders.begin()->second.begin();
            std::shared_ptr<Order> sellOrder = bestSell->second;

            // A match occurs only if the resting sell order's price is less than or equal to the buy order's price.
            if (sellOrder->price > order->price) {
                break;  // No more matches possible
            }

            int tradeQuantity = std::min(order->quantity, sellOrder->quantity);
            trades.push_back({ order->orderID, sellOrder->orderID, tradeQuantity, sellOrder->price });

            // Update remaining quantities
            order->quantity -= tradeQuantity;
            sellOrder->quantity -= tradeQuantity;

            // Remove the sell order if it's fully matched
            if (sellOrder->quantity == 0) {
                sellOrders.begin()->second.erase(bestSell);
                if (sellOrders.begin()->second.empty()) {
                    sellOrders.erase(sellOrders.begin());
                }
            }
        }
    }
    else {  // Incoming SELL order
        while (order->quantity > 0 && !buyOrders.empty()) {
            auto bestBuy = buyOrders.begin()->second.begin();
            std::shared_ptr<Order> buyOrder = bestBuy->second;

            // A match occurs if the resting buy order's price is at least as high as the sell order's price.
            if (buyOrder->price < order->price) {
                break;  // No more matches possible
            }

            int tradeQuantity = std::min(order->quantity, buyOrder->quantity);
            trades.push_back({ buyOrder->orderID, order->orderID, tradeQuantity, order->price });

            // Update remaining quantities
            order->quantity -= tradeQuantity;
            buyOrder->quantity -= tradeQuantity;

            // Remove the buy order if it's fully matched
            if (buyOrder->quantity == 0) {
                buyOrders.begin()->second.erase(bestBuy);
                if (buyOrders.begin()->second.empty()) {
                    buyOrders.erase(buyOrders.begin());
                }
            }
        }
    }
}

bool OrderBook::cancelOrder(int orderId) {
    auto it = ordersById.find(orderId);
    if (it != ordersById.end()) {
        std::shared_ptr<Order> order = it->second;

        if (order->orderType == OrderType::BUY) {
            auto& priceOrders = buyOrders[order->price];
            auto orderIt = priceOrders.find(order->timestamp);
            if (orderIt != priceOrders.end()) {
                priceOrders.erase(orderIt);
                if (priceOrders.empty()) {
                    buyOrders.erase(order->price);
                }
                ordersById.erase(orderId);
                return true;
            }
        }
        else { // SELL order
            auto& priceOrders = sellOrders[order->price];
            auto orderIt = priceOrders.find(order->timestamp);
            if (orderIt != priceOrders.end()) {
                priceOrders.erase(orderIt);
                if (priceOrders.empty()) {
                    sellOrders.erase(order->price);
                }
                ordersById.erase(orderId);
                return true;
            }
        }
    }
    return false;  // Order ID not found
}

// Get raw order book data (used by the server for converting to JSON)
std::pair<std::vector<std::shared_ptr<Order>>, std::vector<std::shared_ptr<Order>>> OrderBook::getRawOrderBookData() const {
    std::vector<std::shared_ptr<Order>> buyOrdersList;
    std::vector<std::shared_ptr<Order>> sellOrdersList;

    // Add buy orders to the buy orders list
    for (const auto& priceOrders : buyOrders) {
        for (const auto& orderEntry : priceOrders.second) {
            buyOrdersList.push_back(orderEntry.second);
        }
    }

    // Add sell orders to the sell orders list
    for (const auto& priceOrders : sellOrders) {
        for (const auto& orderEntry : priceOrders.second) {
            sellOrdersList.push_back(orderEntry.second);
        }
    }

    return { buyOrdersList, sellOrdersList };
}

void OrderBook::displayOrders() const {
    std::cout << "Buy Orders:\n";
    for (const auto& priceOrders : buyOrders) {
        for (const auto& orderEntry : priceOrders.second) {
            const std::shared_ptr<Order>& order = orderEntry.second;
            std::cout << "  ID: " << order->orderID << ", Price: $" << order->price << ", Quantity: " << order->quantity << "\n";
        }
    }

    std::cout << "Sell Orders:\n";
    for (const auto& priceOrders : sellOrders) {
        for (const auto& orderEntry : priceOrders.second) {
            const std::shared_ptr<Order>& order = orderEntry.second;
            std::cout << "  ID: " << order->orderID << ", Price: $" << order->price << ", Quantity: " << order->quantity << "\n";
        }
    }
}
