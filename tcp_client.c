#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

void initialize_winsock();
void send_message_to_node(int node_id, const char* message);

int main() {
    initialize_winsock();

    while (1) {
        int destination_node;
        char message[BUFFER_SIZE];

        printf("Enter destination node (1, 2, or 3): ");
        scanf("%d", &destination_node);

        printf("Enter message to send to Node %d: ", destination_node);
        getchar();  // To consume newline left by scanf
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';  // Remove newline character

        send_message_to_node(destination_node, message);
    }

    WSACleanup();
    return 0;
}

// Initialize Winsock
void initialize_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        exit(1);
    }
}

// Send message to a specific node
void send_message_to_node(int node_id, const char* message) {
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT + 1);  // Connect to Node 1 first
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Failed to connect to Node 1: %d\n", WSAGetLastError());
        closesocket(client_socket);
        return;
    }

    // Send the message in the format: "SRC_NODE_ID:DEST_NODE_ID:message"
    char formatted_message[BUFFER_SIZE];
    sprintf(formatted_message, "0:%d:%s", node_id, message);  // '0' is the source ID for the client
    send(client_socket, formatted_message, strlen(formatted_message), 0);

    // Expect response from Node 1, since all replies will come through it
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';  // Null terminate the string
        printf("Response from Node 1: %s\n", buffer);
    } else {
        printf("No response from Node 1\n");
    }
    closesocket(client_socket);
}
