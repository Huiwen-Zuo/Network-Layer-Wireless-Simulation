
# Network Layer and Wireless Communication Simulation

## Overview

This project simulates a simple network layer communication system where multiple nodes (Node 1, Node 2, and Node 3) are connected and can send messages to each other, simulating packet forwarding and routing. The system also handles replies being sent back to the client via the correct routes. The focus is on demonstrating basic routing functionality, message forwarding, and handling responses.

The project simulates the following:

- **Message Forwarding**: Messages from the client are sent through a multi-hop network, where each node forwards the message based on its routing table.
- **Reply Routing**: Replies are routed back from the destination node to the client following the same forwarding logic.
- **Simple Routing**: A predefined routing table is used by each node to determine the next hop for message forwarding.

## Components

1. **Node (node.c)**: 
   - Each node listens for incoming messages, processes them according to its routing table, and forwards them to the correct next hop.
   - Nodes also handle replies by sending them back to the client via Node 1.

2. **Client (tcp_client.c)**: 
   - The client sends messages to a target node through Node 1 and waits for a response, which is routed back via the correct path through the nodes.

## Features

- **Routing Tables**: Each node has a hardcoded routing table to determine where to forward the message based on the destination node.
- **Multi-Hop Communication**: Messages are forwarded from the client to intermediate nodes before reaching their final destination.
- **Reply Mechanism**: The destination node generates a reply, which is forwarded back to the client following the same multi-hop route.
- **Winsock-Based**: This project uses Winsock2 for networking in a local environment.

## System Requirements

- **Operating System**: Windows (as Winsock2 is used).
- **Compiler**: A C compiler with Winsock2 support (e.g., GCC with MinGW on Windows).
- **Development Tools**: Make sure you have GCC or any compatible compiler installed.

## How to Run the Project

### Step 1: Compile the Files

Use the following commands to compile the **node.c** and **tcp_client.c** files:

```bash
gcc -o node node.c -lws2_32
gcc -o tcp_client tcp_client.c -lws2_32
```

### Step 2: Start the Nodes

You need to start each node in a separate terminal window. Use the following commands to start the nodes:

```bash
./node 1  # Start Node 1
./node 2  # Start Node 2
./node 3  # Start Node 3
```

Each node will listen on a specific port and wait for incoming messages. For example, Node 1 will listen on port 8081, Node 2 on port 8082, and Node 3 on port 8083.

### Step 3: Run the Client

In another terminal, start the client using the following command:

```bash
./tcp_client
```

### Step 4: Send Messages

Once the client is running, you will be prompted to enter:

- The **destination node** (Node 1, Node 2, or Node 3).
- The **message** you want to send.

For example, you can enter the following:

```
Enter destination node (1, 2, or 3): 2
Enter message to send to Node 2: Hello, Node 2!
```

The message will be forwarded through Node 1 (and possibly other nodes) until it reaches the destination node. The destination node will generate a reply, which will be sent back to the client through Node 1.

### Step 5: View Responses

The client will display the response once the reply reaches Node 1 and is forwarded back to the client.

## Known Issues

- **Reply from Node 2/3**: There might be issues with correctly forwarding replies from Node 2 and Node 3 back to the client. Currently, Node 1 handles all replies directly.
- **Message Format**: All messages and replies are sent in a specific format that includes both source and destination node IDs. This format ensures that messages are correctly routed.

## Future Enhancements

- Improve the routing logic to better handle multiple hops in different network topologies.
- Add dynamic routing table updates to simulate real-time network changes.
- Enhance the error handling and introduce timeouts for network communication failures.

## Conclusion

This project demonstrates a basic network layer simulation with message forwarding and reply routing in a multi-hop network. It is a simple yet effective demonstration of network routing principles and message handling in a wireless communication scenario.
