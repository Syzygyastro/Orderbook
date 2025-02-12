#include "order_book.h"
#include <algorithm>
#include <iostream>
#include <iterator>

// Helper: Matching orders.
// For a BUY order, we try to match with the best (lowest-price) asks.
// For a SELL order, we match with the best (highest-price) bids.
void OrderBook::matchOrders(OrderPointer order, std::vector<Trade>& trades) {
    if (order->GetSide() == OrderType::BUY) {
        // For a BUY order, match with asks (lowest price first).
        while (order->quantity > 0 && !asks_.empty()) {
            auto bestAskIt = asks_.begin(); // Lowest ask price.
            Price askPrice = bestAskIt->first;
            if (askPrice > order->GetPrice()) {
                break; // Cannot match: best ask is above the buy price.
            }
            // Get the FIFO order from this price level.
            auto& askList = bestAskIt->second;
            OrderPointer sellOrder = askList.front();
            int tradeQuantity = std::min(order->quantity, sellOrder->quantity);
            trades.push_back({ order->GetOrderId(), sellOrder->GetOrderId(), tradeQuantity, askPrice });
            order->quantity -= tradeQuantity;
            sellOrder->quantity -= tradeQuantity;

            // If the sell order is fully executed, remove it.
            if (sellOrder->quantity == 0) {
                // Look up its stored location.
                auto entryIt = orders_.find(sellOrder->GetOrderId());
                if (entryIt != orders_.end()) {
                    askList.erase(entryIt->second.location_);
                    orders_.erase(entryIt);
                }
                if (askList.empty()) {
                    asks_.erase(bestAskIt);
                }
            }
        }
    }
    else {
        // For a SELL order, match with bids (highest price first).
        while (order->quantity > 0 && !bids_.empty()) {
            auto bestBidIt = bids_.begin(); // Highest bid price.
            Price bidPrice = bestBidIt->first;
            if (bidPrice < order->GetPrice()) {
                break; // Cannot match: best bid is below the sell price.
            }
            auto& bidList = bestBidIt->second;
            OrderPointer buyOrder = bidList.front();
            int tradeQuantity = std::min(order->quantity, buyOrder->quantity);
            trades.push_back({ buyOrder->GetOrderId(), order->GetOrderId(), tradeQuantity, bidPrice });
            order->quantity -= tradeQuantity;
            buyOrder->quantity -= tradeQuantity;

            if (buyOrder->quantity == 0) {
                auto entryIt = orders_.find(buyOrder->GetOrderId());
                if (entryIt != orders_.end()) {
                    bidList.erase(entryIt->second.location_);
                    orders_.erase(entryIt);
                }
                if (bidList.empty()) {
                    bids_.erase(bestBidIt);
                }
            }
        }
    }
}

// Add a new order to the order book.
std::vector<Trade> OrderBook::addOrder(OrderPointer order) {
    std::vector<Trade> trades;
    // Check if the order ID already exists.
    if (orders_.find(order->GetOrderId()) != orders_.end()) {
        return trades;  // Returning empty trades because we rejected the order.
    }
    // First, try to match the order.
    matchOrders(order, trades);

    // If the order still has remaining quantity, add it to the appropriate side.
    if (order->quantity > 0) {
        if (order->GetSide() == OrderType::BUY) {
            auto& orderList = bids_[order->GetPrice()];
            orderList.push_back(order);
            // Save iterator to the newly added order.
            auto iter = std::prev(orderList.end());
            orders_.insert({ order->GetOrderId(), OrderEntry{ order, iter } });
        }
        else {
            auto& orderList = asks_[order->GetPrice()];
            orderList.push_back(order);
            auto iter = std::prev(orderList.end());
            orders_.insert({ order->GetOrderId(), OrderEntry{ order, iter } });
        }
    }
    return trades;
}

// Cancel an order using the stored iterator.
bool OrderBook::cancelOrder(OrderId orderId) {
    auto it = orders_.find(orderId);
    if (it != orders_.end()) {
        OrderPointer order = it->second.order_;
        if (order->GetSide() == OrderType::BUY) {
            auto& orderList = bids_[order->GetPrice()];
            orderList.erase(it->second.location_);
            if (orderList.empty()) {
                bids_.erase(order->GetPrice());
            }
        }
        else {
            auto& orderList = asks_[order->GetPrice()];
            orderList.erase(it->second.location_);
            if (orderList.empty()) {
                asks_.erase(order->GetPrice());
            }
        }
        orders_.erase(it);
        return true;
    }
    return false;
}

// Display the order book.
void OrderBook::displayOrders() const {
    std::cout << "Bids:\n";
    for (const auto& priceOrders : bids_) {
        for (const auto& order : priceOrders.second) {
            std::cout << "  ID: " << order->orderID
                << ", Price: " << order->price
                << ", Qty: " << order->quantity << "\n";
        }
    }
    std::cout << "Asks:\n";
    for (const auto& priceOrders : asks_) {
        for (const auto& order : priceOrders.second) {
            std::cout << "  ID: " << order->orderID
                << ", Price: " << order->price
                << ", Qty: " << order->quantity << "\n";
        }
    }
}

// Get raw order book data.
std::pair<std::vector<OrderPointer>, std::vector<OrderPointer>> OrderBook::getRawOrderBookData() const {
    std::vector<OrderPointer> bidOrders, askOrders;
    for (const auto& priceOrders : bids_) {
        for (const auto& order : priceOrders.second) {
            bidOrders.push_back(order);
        }
    }
    for (const auto& priceOrders : asks_) {
        for (const auto& order : priceOrders.second) {
            askOrders.push_back(order);
        }
    }
    return { bidOrders, askOrders };
}
