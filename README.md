# 💬 IRC-Style Chat Server (C++)

A **multi-client chat server written in C++** using low-level TCP
sockets.\
This project focuses on understanding **how network communication
works** and how servers manage multiple users simultaneously.

------------------------------------------------------------------------

# 🚀 Project Overview

The server allows multiple clients to connect and communicate in real
time.

Main features implemented:

-   👥 Multiple clients can connect at the same time\
-   🏷️ Users choose a nickname\
-   📢 Messages are broadcast to other connected users\
-   💬 Chat rooms allow group discussions\
-   🔌 Clients can connect from different machines on the network

The server uses an **event-driven architecture** to efficiently manage
many connections at once.

------------------------------------------------------------------------

# 🧠 What I Learned

### 🌐 Networking Fundamentals

This project helped me understand the basics of **TCP communication**,
including:

-   How clients connect to a server
-   How data travels across a network
-   How servers listen for incoming connections

### ⚡ Event-Driven Server Design

Instead of creating a thread for each user, the server monitors multiple
connections at the same time and reacts when something happens (new
connection, incoming message, disconnection).

This approach makes the server: - more efficient\
- easier to control\
- scalable for multiple users

### 📦 Message Handling

One important challenge was understanding that **network messages do not
always arrive exactly as they were sent**.

Messages may arrive: - split into several pieces\
- combined with other messages

A buffering system ensures that **messages are reconstructed correctly
before being processed**.

### 👤 User Management

Each client is registered with a **nickname**, allowing users to
identify who is sending messages.

Example:

Alice: Hello everyone!\
Bob: Hi Alice!

### 💬 Chat Rooms

To organize conversations, users can join **chat rooms**, allowing group
discussions and separating topics.

------------------------------------------------------------------------

# 🧪 Testing the Server

The server was tested using simple network tools such as:

-   nc (Netcat)
-   telnet

These tools simulate chat clients and allow testing multiple connections
from different machines.

------------------------------------------------------------------------

# 🧩 Challenges Faced

During development I had to solve several common networking problems:

-   managing multiple client connections
-   handling unexpected disconnections
-   ensuring messages are processed correctly
-   designing a clean communication structure

------------------------------------------------------------------------

# 📚 Skills Practiced

💻 C++ systems programming\
🌐 TCP networking\
⚡ event-driven architecture\
🧠 problem solving and debugging\
🔌 client-server communication

------------------------------------------------------------------------

# 🔮 Possible Improvements

Future improvements could include:

-   private messaging between users\
-   listing available chat rooms\
-   displaying users in a room\
-   better command handling\
-   compatibility with real IRC clients

------------------------------------------------------------------------

# 🎉 Conclusion

This project provided a deeper understanding of **how real-time
communication servers work**.

By building a chat server from scratch, I gained practical experience
with:

-   network protocols\
-   multi-client server design\
-   real-time communication systems

It was a great hands-on project for exploring **network programming and
system-level development** 🚀
