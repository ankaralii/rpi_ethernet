#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define WINDOWS_IP "192.168.1.10"
#define PORT 8080

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char *telemetry_data = "Telemetri Paketi: Gömülü Sistem Aktif.";

    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Soket olusturma hatasi");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, WINDOWS_IP, &server_addr.sin_addr) <= 0) {
        perror("Gecersiz IP Adresi");
        exit(EXIT_FAILURE);
    }

    while (1) {
        sendto(client_socket, (const char *)telemetry_data, strlen(telemetry_data), 0, 
               (const struct sockaddr *)&server_addr, sizeof(server_addr));
        printf("Windows'a paket gonderildi...\n");
        sleep(1);
    }

    close(client_socket);
    return 0;
}
