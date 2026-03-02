# Chat Application in Erlang

## Overview

This project implements a distributed chat system using Erlang processes.  
It supports:

- Real-time messaging
- Offline message storage
- User presence tracking

---

## Features

### 1. Bi-directional Communication
Enables simultaneous message exchange between two users.

### 2. User Presence Detection
- The server maintains online/offline status of users.
- The sender can verify recipient availability before sending a message.

### 3. Server-Mediated Messaging
All messages are routed through a central server node.

### 4. Offline Message Storage
Messages are stored when the recipient is offline.

### 5. Deferred Delivery
Stored messages are delivered automatically when the recipient reconnects.

### 6. Message Expiry
Messages are deleted if the recipient remains offline beyond a defined timeout period.

### 7. Dedicated Process Per User
The server spawns and maintains a separate Erlang process for each connected user.

---

## Architecture

### 1. Server Node

Responsibilities:

- Maintain user registry
- Track online/offline status
- Store queued messages
- Spawn and manage a dedicated process per user

### 2. Client Node

Responsibilities:

- Connect to the server
- Register user identity
- Send messages via the server
- Receive messages from the server

---

## Technologies Used

- Erlang
- Distributed Erlang nodes
- Message passing
- OTP processes
- C programming (client implementation)
- Socket programming

## Build

    $ rebar3 compile
