import ws from 'k6/ws';
import { check, sleep } from 'k6';

export const options = {
  stages: [
    { duration: '10s', target: 50 },  // Ramp up to 50 WebSocket users over 30 seconds
    { duration: '20', target: 500 }, // Stay at 200 users for 1 minute
    { duration: '10s', target: 0 },  // Ramp down to 0 users over 30 seconds
  ],
};

export default function () {
  const url = 'ws://localhost:8080/orderbook';
  const params = { tags: { my_tag: 'orderbook_connection' } };

  const response = ws.connect(url, params, function (socket) {
    // Event when WebSocket connection is opened
    socket.on('open', function open() {
    });

    // Event handler for receiving messages (updates from the server)
    socket.on('message', function (msg) {
      const message = JSON.parse(msg);
      console.log('Received message:', message);

      // Check if the message contains order book data or updates
      check(message, {
        'Received valid update': (msg) => msg.status === 'update' || msg.type === 'snapshot',
      });
    });

    // Event handler for errors
    socket.on('error', (e) => {
      if (e.error() != 'websocket: close sent') {
        console.log('An unexpected error occurred:', e.error());
      }
    });

    // Keep the WebSocket open for 10 seconds
    sleep(10);

    // Close the WebSocket connection after the specified time
    socket.close();
  });

  // Check WebSocket connection response status
  check(response, { 'status is 101': (r) => r && r.status === 101 });
}
