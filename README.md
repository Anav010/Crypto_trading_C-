1️⃣ Project Overview
This project is a C++-based trading system that interacts with the Deribit API for cryptocurrency trading. It provides functionalities such as:
✅ Placing Orders (Limit & Market)
✅ Canceling Orders
✅ Modifying Orders
✅ Retrieving Order Book Data
✅ Viewing Current Positions
✅ (Optional) WebSocket-based Live Data Streaming

The system is built using C++17, cURL for API requests, and nlohmann/json for JSON handling.


2️⃣ Technologies Used
C++17 → Core language
cURL → HTTP requests for API communication
nlohmann/json → JSON parsing
Boost.Asio & Boost.Beast → WebSockets (optional, for real-time data streaming)
Deribit API → Crypto trading API

3️⃣ System Functionality
📌 Step 1: User Authentication
User enters client_id and client_secret.
The system authenticates with Deribit and fetches an access_token.
If authentication is successful, the main menu appears.

MAIN MENU
1️⃣ Place Order  
2️⃣ Cancel Order  
3️⃣ Modify Order  
4️⃣ Get Order Book  
5️⃣ View Current Positions  
6️⃣ Exit  

4️⃣ API Interaction
send_post_request() is used to send API requests.
Responses are parsed using nlohmann/json and errors are handled gracefully.
