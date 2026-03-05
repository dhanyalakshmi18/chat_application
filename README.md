# Chat Application

A high-performance, terminal-based distributed chat application that bridges the gap between **Erlang/OTP** and **C**. This project features a custom TUI (Terminal User Interface) and a fault-tolerant backend designed for reliability, structured data exchange, and horizontal scalability.

---

## 📖 Overview

**Chat Application** is a hybrid system built to demonstrate seamless integration between functional and procedural programming through network sockets.

*   **Backend:** Developed on the Erlang BEAM VM (OTP 27.1), leveraging **Mnesia** for real-time data persistence.
*   **Frontend:** A low-level C client utilizing the **Ncurses** library to provide a high-performance terminal experience.
*   **Communication:** Established via **TCP/IP Socket Programming**, using Erlang's `gen_tcp` module and standard C `socket` libraries.
*   **Serialization:** Data transmitted over the sockets is handled via **Protocol Buffers (Protobuf)**, ensuring that message exchange is structured, type-safe, and highly efficient.

---

## ✨ Features

### 1. Custom Terminal UI (TUI)
Built with **C and Ncurses**, the client provides a dedicated multi-window interface for registration, login, and messaging. It offers a lightweight alternative to graphical interfaces while maintaining a responsive user experience.

### 2. Socket-Based Bi-directional Communication
Enables simultaneous, low-latency message exchange between the C client and the Erlang server. The system uses a full-duplex TCP connection to ensure messages are delivered in real-time.

### 3. User Presence Detection
*   The server maintains the real-time online/offline status of all registered users in its memory.
*   Senders can verify the recipient's availability before initiating a message exchange.

### 4. Reliable Offline Storage
*   **Mnesia Integration:** Messages targeted at offline users are securely stored within the server's **Mnesia database**.
*   **Deferred Delivery:** Queued messages are automatically pushed to the recipient the moment they re-establish a socket connection.
*   **Message Expiry:** To optimize system resources, messages are automatically cleared if the recipient remains offline beyond a **1-hour timeout period** (this duration is fully configurable).

### 5. Robust Concurrency
The server utilizes Erlang’s lightweight process model to spawn a **dedicated process per user**. This ensures complete isolation; a failure in one user's connection or process does not affect the stability of other active users.

---

## 🏗️ Architecture

### Server Node (Erlang/OTP + gpb)
*   **Socket Handler:** Uses `gen_tcp` to listen for incoming connections and manage data transmission.
*   **Protobuf Engine:** Uses the `gpb` library to decode incoming byte streams from the C client into Erlang records/ encode outgoing byte streams to C client from Erlang.
*   **Registry:** Manages user identities and session states across the distributed node.
*   **Storage:** Utilizes Mnesia for persistent user profiles and offline message queuing.
*   **Process Management:** Implements OTP Supervision trees to maintain fault-tolerant user processes.

### Client Node (C + Protobuf-C)
*   **Socket Client:** Uses standard Linux C sockets to connect to the Erlang server.
*   **Interface:** Handles rendering and user input via the Ncurses library.
*   **Protobuf-C:** Manages the serialization of C structures into wire-format bytes before sending them through the socket, ensuring compatibility with the Erlang backend.

---

## 🛠️ Tech Stack

*   **Backend:** Erlang/OTP (GenServer, Mnesia, Supervision Trees)
*   **Frontend:** C Programming (Ncurses TUI)
*   **Communication:** TCP Sockets (`gen_tcp` in Erlang & `sys/socket.h` in C)
*   **Serialization:** Protocol Buffers (using `gpb` for Erlang and `protobuf-c` for C)
*   **Build System:** `rebar3` integrated with a custom `Makefile` via pre-compile hooks.

---

## 🚀 Getting Started

### Prerequisites
- **Erlang/OTP 27.1** and **rebar3**
- **GCC** and **GNU Make**
- **libncurses-dev** (Ncurses development headers)
- **libprotobuf-c-dev** and **protobuf-c-compiler**

### Build
The project uses `rebar3` to trigger a unified build of both the Erlang application and the C executable:

```bash
rebar3 compile