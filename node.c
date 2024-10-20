#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    int destination;
    int next_hop;
} RoutingEntry;

typedef struct {
    int node_id;
    RoutingEntry routing_table[2];  // Adjust size based on network topology
} Node;

void initialize_winsock();
void* handle_client(void* client_args);
int forward_message(int next_hop_node_id, const char* message);
void forward_reply_to_client(int next_hop_node_id, const char* message, SOCKET original_client_socket);
void send_reply_to_client(SOCKET client_socket, int node_id, Node* current_node);

// Helper struct to pass multiple arguments to the thread function
typedef struct {
    SOCKET client_socket;
    Node* current_node;
} ClientHandlerArgs;

// Initialize Winsock
void initialize_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        exit(1);
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <node_id>\n", argv[0]);
        return 1;
    }

    int node_id = atoi(argv[1]);
    initialize_winsock();

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT + node_id);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Node %d listening on port %d...\n", node_id, PORT + node_id);

    // Routing table for each node
    Node node;
    node.node_id = node_id;

    if (node_id == 1) {
        node.routing_table[0] = (RoutingEntry){2, 2};  // Route to Node 2
        node.routing_table[1] = (RoutingEntry){3, 2};  // Route to Node 3 via Node 2
    } else if (node_id == 2) {
        node.routing_table[0] = (RoutingEntry){1, 1};  // Route to Node 1
        node.routing_table[1] = (RoutingEntry){3, 3};  // Route to Node 3
    } else if (node_id == 3) {
        node.routing_table[0] = (RoutingEntry){1, 2};  // Route to Node 1 via Node 2
        node.routing_table[1] = (RoutingEntry){2, 2};  // Route to Node 2
    }

    while (1) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed: %d\n", WSAGetLastError());
            closesocket(server_socket);
            WSACleanup();
            return 1;
        }

        // Prepare client handler arguments
        ClientHandlerArgs* args = (ClientHandlerArgs*)malloc(sizeof(ClientHandlerArgs));
        args->client_socket = client_socket;
        args->current_node = &node;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, (void*)args);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

// Function to handle client connections
void* handle_client(void* client_args) {
    ClientHandlerArgs* args = (ClientHandlerArgs*)client_args;
    SOCKET sock = args->client_socket;
    Node* current_node = args->current_node;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_received] = '\0';

        int dest_node_id, src_node_id;
        char message[BUFFER_SIZE];
        sscanf(buffer, "%d:%d:%[^\n]", &src_node_id, &dest_node_id, message);  // Parse "SRC_NODE_ID:DEST_NODE_ID:message"

        printf("Message received: %s (Source: %d, Destination Node %d)\n", message, src_node_id, dest_node_id);

        // If this is a reply, forward it back to the client via Node 1
        if (src_node_id != 0 && dest_node_id == 1) {
            printf("Forwarding reply to client: %s\n", message);
            send(sock, buffer, strlen(buffer), 0);  // Forward reply to client
        }
        else if (dest_node_id == current_node->node_id) {
            // Message reached its final destination
            printf("Message reached its final destination: Node %d\n", current_node->node_id);
            send_reply_to_client(sock, current_node->node_id, current_node);  // Send reply to client
        } else {
            // Forward the message to the next hop based on routing table
            for (int i = 0; i < 2; ++i) {
                if (current_node->routing_table[i].destination == dest_node_id) {
                    forward_message(current_node->routing_table[i].next_hop, buffer);
                    break;
                }
            }
        }
    }

    closesocket(sock);
    free(args);
    pthread_exit(NULL);
}

// Function to forward message to the next node
int forward_message(int next_hop_node_id, const char* message) {
    SOCKET forward_socket;
    struct sockaddr_in next_hop_addr;
    WSADATA wsaData;

    printf("Forwarding message to Node %d...\n", next_hop_node_id);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return -1;
    }

    forward_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (forward_socket == INVALID_SOCKET) {
        printf("Failed to create forwarding socket: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    next_hop_addr.sin_family = AF_INET;
    next_hop_addr.sin_port = htons(PORT + next_hop_node_id);
    next_hop_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(forward_socket, (struct sockaddr *)&next_hop_addr, sizeof(next_hop_addr)) == SOCKET_ERROR) {
        printf("Failed to connect to Node %d: %d\n", next_hop_node_id, WSAGetLastError());
        closesocket(forward_socket);
        WSACleanup();
        return -1;
    }

    printf("Connected to Node %d, forwarding message: %s\n", next_hop_node_id, message);
    send(forward_socket, message, strlen(message), 0);

    closesocket(forward_socket);
    WSACleanup();
    return 0;
}

// Send a reply to the client from the destination node
void send_reply_to_client(SOCKET client_socket, int node_id, Node* current_node) {
    char response[BUFFER_SIZE];
    // Explicitly set the source node ID as this node and the destination to Node 1
    sprintf(response, "%d:1:Hello client, from Node %d", node_id, node_id);

    // If this node is directly connected to the client (Node 1), send directly
    if (current_node->node_id == 1) {
        send(client_socket, response, strlen(response), 0);
        printf("Sent reply directly to client: %s\n", response);
    } else {
        // Forward the reply back via the appropriate route
        printf("Node %d forwarding reply back to Node 1...\n", current_node->node_id);

        // Look for the next hop towards Node 1 for reply routing
        for (int i = 0; i < 2; ++i) {
            if (current_node->routing_table[i].destination == 1) {
                forward_reply_to_client(current_node->routing_table[i].next_hop, response, client_socket);
                break;
            }
        }
    }
}

// Forward the reply to the client through intermediate nodes
void forward_reply_to_client(int next_hop_node_id, const char* message, SOCKET original_client_socket) {
    SOCKET forward_socket;
    struct sockaddr_in next_hop_addr;
    WSADATA wsaData;

    printf("Forwarding reply to Node %d...\n", next_hop_node_id);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return;
    }

    forward_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (forward_socket == INVALID_SOCKET) {
        printf("Failed to create forwarding socket: %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    next_hop_addr.sin_family = AF_INET;
    next_hop_addr.sin_port = htons(PORT + next_hop_node_id);
    next_hop_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(forward_socket, (struct sockaddr *)&next_hop_addr, sizeof(next_hop_addr)) == SOCKET_ERROR) {
        printf("Failed to connect to Node %d: %d\n", next_hop_node_id, WSAGetLastError());
        closesocket(forward_socket);
        WSACleanup();
        return;
    }

    printf("Connected to Node %d, forwarding reply: %s\n", next_hop_node_id, message);
    send(forward_socket, message, strlen(message), 0);

    closesocket(forward_socket);
    WSACleanup();
}
