# Real-Time High-Performance Order Book Matching Engine

## Overview

This project implements a low-latency C++ order book matching engine designed for high-frequency trading environments. It enforces strict price-time priority, supports partial fills, and utilizes custom advanced data structures to optimize market and limit order operations. Real-time market data is broadcast via Crow-based WebSocket and HTTPS endpoints.

## Features

- **Low-Latency Execution:** Processes orders with average latencies under **50 µs** and handles over **100K orders/sec**.
- **Price-Time Priority & Partial Fills:** Enforces price-time priority for fair order execution and supports partial fills for both market and limit orders.
- **Advanced Data Structures:** Uses custom data structures (optimized maps, lists, and iterator caching) for constant average-time duplicate detection and O(1) cancellation.
- **Real-Time Updates:** Integrates Crow’s WebSocket and HTTPS endpoints to deliver sub-100 ms real-time updates to over **500 concurrent users**.
- **Scalable & Robust API:** Rigorous load testing with k6 validates performance at **10K+ concurrent connections** while maintaining 95th percentile response times below **150 ms**.

## Architecture

- **Matching Engine:**  
  Written in C++ and leverages STL along with custom data structures to achieve low latency and high throughput.
- **Web Interface:**  
  Exposes RESTful and WebSocket endpoints using Crow for real-time market data dissemination and trading operations.
- **Testing & Performance:**  
  Includes extensive unit testing and load testing (via k6) to ensure thread safety, scalability, and consistent high performance under load.

## Requirements

- **Compiler:** C++11 or later
- **Libraries:**  
  - [Crow](https://github.com/CrowCpp/Crow) (for WebSocket and REST API)  
  - STL (Standard Template Library)
- **Build Tools:** CMake or your preferred build system
- **Testing:** k6 for load testing
- **Platform:** Linux, macOS, or Windows with an appropriate toolchain

## Build & Run Instructions

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/yourusername/orderbook-matching-engine.git
   cd orderbook-matching-engine
Build the Project:

bash
Copy
mkdir build && cd build
cmake ..
make
Run the Engine:

bash
Copy
./orderbook_engine
Usage
REST API Endpoints:
Submit orders and cancel orders via http://localhost:8080/api/orders.

WebSocket Endpoint:
Connect to ws://localhost:8080/orderbook to receive real-time order book updates.

Load Testing:
Run load tests with k6:

bash
Copy
k6 run load_test.js
