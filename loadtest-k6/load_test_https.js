import http from 'k6/http';
import { check, sleep } from 'k6';

// Configuration for the HTTP load test
export const options = {
    stages: [
        { duration: '15s', target: 10 },
        { duration: '20s', target: 50 },
        { duration: '15s', target: 0 },
    ],
};

// Simulate a POST request to create a new order
export default function () {
    // POST request to create an order
    const orderPayload = JSON.stringify({
        orderID: Math.floor(Math.random() * 1000000),
        price: Math.random() * 100,  // Directly generate a float between 0-100
        quantity: Math.floor(Math.random() * 50),  // Random integer for quantity
        orderType: Math.random() > 0.6 ? 'buy' : 'sell',
    });

    const postRes = http.post('http://localhost:8080/api/orders', orderPayload, {
        headers: { 'Content-Type': 'application/json' },
    });

    // Check that the response status code is 200
    check(postRes, {
        'order added successfully': (r) => r.status === 200,
    });

    // Add a short delay between requests
    sleep(3);
}
