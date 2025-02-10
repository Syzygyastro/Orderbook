import React, { useEffect, useState } from "react";

function App() {
  const [bids, setBids] = useState([]);
  const [asks, setAsks] = useState([]);

  // Add Order Form State
  const [orderID, setOrderID] = useState("");
  const [price, setPrice] = useState("");
  const [quantity, setQuantity] = useState("");
  const [orderType, setOrderType] = useState("buy");

  // Cancel Order Form
  const [cancelOrderID, setCancelOrderID] = useState("");

  useEffect(() => {
    // 1. Fetch initial order book
    fetch("/api/orderbook")
      .then((res) => res.json())
      .then((data) => {
        setBids(data.bids || []);
        setAsks(data.asks || []);
      })
      .catch((error) => {
        console.error("Error fetching orderbook:", error);
      });

    // 2. Open WebSocket
    const socketUrl = `ws://localhost:8080/orderbook`;
    const socket = new WebSocket(socketUrl);

    socket.onopen = () => {
      console.log("WebSocket connected");
    };

    socket.onmessage = (event) => {
      const msg = JSON.parse(event.data);
      // If it's a snapshot
      if (msg.type === "snapshot") {
        // It's the entire book
        setBids(msg.bids || []);
        setAsks(msg.asks || []);
      } 
      // If it's an update
      else if (msg.status === "update") {
        // The new data is in msg.data
        const data = msg.data;
        setBids(data.bids || []);
        setAsks(data.asks || []);
      }
    };

    socket.onclose = () => {
      console.log("WebSocket disconnected");
    };

    // Cleanup on unmount
    return () => {
      socket.close();
    };
  }, []);

  // =======================
  //  ADD ORDER (POST /api/orders)
  // =======================
  const handleAddOrder = (e) => {
    e.preventDefault();
    const payload = {
      orderID: parseInt(orderID, 10),
      price: parseFloat(price),
      quantity: parseInt(quantity, 10),
      orderType
    };

    fetch("/api/orders", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(payload),
    })
      .then((res) => res.json())
      .then((data) => {
        console.log("Order added. Trades executed:", data.trades);
        // The orderbook will update automatically via WebSocket
      })
      .catch((err) => console.error("Failed to add order:", err));
  };

  // =======================
  // CANCEL ORDER (DELETE /api/order/<id>)
  // =======================
  const handleCancelOrder = (e) => {
    e.preventDefault();
    fetch(`/api/order/${cancelOrderID}`, {
      method: "DELETE",
    })
      .then((res) => {
        if (res.ok) {
          console.log("Order cancelled");
        } else {
          console.warn("Order not found or could not cancel");
        }
        // The orderbook will update automatically via WebSocket
      })
      .catch((err) => console.error("Failed to cancel order:", err));
  };

  // ==============================
  //  Rendering the Order Book UI
  // ==============================
  return (
    <div style={{ margin: "20px" }}>
      <h1>Order Book</h1>

      {/* Order Book Tables */}
      <div style={{ display: "flex", gap: "50px" }}>
        <div>
          <h2>Bids</h2>
          <table border="1" cellPadding="5">
            <thead>
              <tr>
                <th>Order ID</th>
                <th>Price</th>
                <th>Quantity</th>
              </tr>
            </thead>
            <tbody>
              {bids.map((bid) => (
                <tr key={bid.orderID}>
                  <td>{bid.orderID}</td>
                  <td>{bid.price}</td>
                  <td>{bid.quantity}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        <div>
          <h2>Asks</h2>
          <table border="1" cellPadding="5">
            <thead>
              <tr>
                <th>Order ID</th>
                <th>Price</th>
                <th>Quantity</th>
              </tr>
            </thead>
            <tbody>
              {asks.map((ask) => (
                <tr key={ask.orderID}>
                  <td>{ask.orderID}</td>
                  <td>{ask.price}</td>
                  <td>{ask.quantity}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      {/* Form to Add an Order */}
      <div style={{ marginTop: "20px" }}>
        <h2>Add Order</h2>
        <form onSubmit={handleAddOrder}>
          <div>
            <label>Order ID: </label>
            <input
              type="number"
              required
              value={orderID}
              onChange={(e) => setOrderID(e.target.value)}
            />
          </div>
          <div>
            <label>Price: </label>
            <input
              type="number"
              step="0.01"
              required
              value={price}
              onChange={(e) => setPrice(e.target.value)}
            />
          </div>
          <div>
            <label>Quantity: </label>
            <input
              type="number"
              required
              value={quantity}
              onChange={(e) => setQuantity(e.target.value)}
            />
          </div>
          <div>
            <label>Order Type: </label>
            <select
              value={orderType}
              onChange={(e) => setOrderType(e.target.value)}
            >
              <option value="buy">Buy</option>
              <option value="sell">Sell</option>
            </select>
          </div>
          <button type="submit" style={{ marginTop: "10px" }}>
            Add Order
          </button>
        </form>
      </div>

      {/* Form to Cancel an Order */}
      <div style={{ marginTop: "20px" }}>
        <h2>Cancel Order</h2>
        <form onSubmit={handleCancelOrder}>
          <div>
            <label>Order ID to cancel: </label>
            <input
              type="number"
              required
              value={cancelOrderID}
              onChange={(e) => setCancelOrderID(e.target.value)}
            />
          </div>
          <button type="submit" style={{ marginTop: "10px" }}>
            Cancel
          </button>
        </form>
      </div>
    </div>
  );
}

export default App;
