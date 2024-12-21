#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Winsock kütüphanesi otomatik olarak dahil edilir
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET clientSock;
    struct sockaddr_in serverAddr;
    char buffer[256], cevap[256];

    // Winsock başlat
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock başlatilamadi. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    // Socket oluştur
    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock == INVALID_SOCKET) {
        printf("Socket oluşturulamadi. Hata kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Sunucu adres bilgilerini ayarla
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Sunucuya bağlan
    if (connect(clientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Server'a bağlanilamadi. Hata kodu: %d\n", WSAGetLastError());
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    printf("Server'a baglanildi. Cikmak icin 'EXIT' yazin.\n");

    while (1) {
        printf("Komut girin (SORGULA [ilac_adi] veya SATIN_AL [ilac_adi] [miktar]):\n> ");
        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = '\0'; // Yeni satır karakterini kaldır

        // Çıkış kontrolü
        if (strcmp(buffer, "EXIT") == 0) {
            printf("Baglanti sonlandiriliyor...\n");
            break;
        }

        // Komutu gönder
        if (send(clientSock, buffer, strlen(buffer), 0) < 0) {
            printf("Komut gönderilemedi. Hata kodu: %d\n", WSAGetLastError());
            break;
        }

        // Sunucudan yanıt al
        int readSize = recv(clientSock, cevap, sizeof(cevap), 0);
        if (readSize > 0) {
            cevap[readSize] = '\0';
            printf("Server'dan cevap: %s\n", cevap);
        } else {
            printf("Server bağlantisi kesildi.\n");
            break;
        }
    }

    // Kaynakları serbest bırak
    closesocket(clientSock);
    WSACleanup();
    return 0;
}

/*
gcc -fdiagnostics-color=always -g C:\Users\zahidesoylu\Desktop\datacom\client.c -o C:\Users\zahidesoylu\Desktop\datacom\client.exe -lws2_32 
C:\Users\zahidesoylu\Desktop\datacom\client.exe
*/