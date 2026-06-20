#include <stdio.h>
#include <winsock2.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET server_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock baslatilamadi. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Soket olusturulamadi. Hata Kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind hatasi. Hata Kodu: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Windows UDP Sunucusu %d portunda dinlemede...\n", PORT);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int received_bytes = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, 
                                      (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (received_bytes > 0) {
            buffer[received_bytes] = '\0';
            printf("Pi [%s] -> %s\n", inet_ntoa(client_addr.sin_addr), buffer);
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
