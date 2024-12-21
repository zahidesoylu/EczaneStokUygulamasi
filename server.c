#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    char ilac[50];
    int stok;
} Ilac;

Ilac stoklar[] = {
    {"Parol", 20},
    {"Aspirin", 15},
    {"Ventolin", 10},
    {"Augmentin", 5}
};

//stoklar dizisinin kaç elemandan oluştugunu hesaplayarak ilaç sayısını verir
int ilacSayisi = sizeof(stoklar) / sizeof(stoklar[0]);

void* clientHandler(void* socketDesc) {
    int clientSock = *(int*)socketDesc;
    char buffer[256];
    int readSize;
    
    //recv fonksiyonu ile istemciden veri alınıyor ve alınan veri buffer dizisine yerleştirilir
    //cevap adında, istemciye gönderilecek yanıtı tutacak boş bir dizi oluşturulur
    while ((readSize = recv(clientSock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[readSize] = '\0';
        char cevap[256] = {0};

        char* komut = strtok(buffer, " ");
        char* ilacAdi = strtok(NULL, " ");
        char* miktarStr = strtok(NULL, " ");
        int miktar = miktarStr ? atoi(miktarStr) : 0;

        //Kullanıcıdan gelen "SORGULA" komutunu işler
        if (strcmp(komut, "SORGULA") == 0) {
            int bulundu = 0;
            for (int i = 0; i < ilacSayisi; i++) {
                if (strcmp(stoklar[i].ilac, ilacAdi) == 0) {
                    snprintf(cevap, sizeof(cevap), "%s stogu: %d adet", stoklar[i].ilac, stoklar[i].stok);
                    bulundu = 1;
                    break;
                }
            }
            if (!bulundu) {
                snprintf(cevap, sizeof(cevap), "%s bulunamadi.", ilacAdi);
            }
        } else if (strcmp(komut, "SATIN_AL") == 0) {
            int bulundu = 0;
            for (int i = 0; i < ilacSayisi; i++) {
                if (strcmp(stoklar[i].ilac, ilacAdi) == 0) {
                    bulundu = 1;
                    if (stoklar[i].stok >= miktar) {
                        stoklar[i].stok -= miktar;
                        snprintf(cevap, sizeof(cevap), "Satin alma başarili! Kalan %s stoğu: %d adet.", stoklar[i].ilac, stoklar[i].stok);
                    } else {
                        snprintf(cevap, sizeof(cevap), "Stok yetersiz! Kalan %s stoğu: %d adet.", stoklar[i].ilac, stoklar[i].stok);
                    }
                    break;
                }
            }
            if (!bulundu) {
                snprintf(cevap, sizeof(cevap), "%s bulunamadi.", ilacAdi);
            }
        } else {
            snprintf(cevap, sizeof(cevap), "Bilinmeyen komut.");
        }
        //istemciye cevap göndermek için
        send(clientSock, cevap, strlen(cevap), 0);
    }

    closesocket(clientSock); //socketi kapatır
    free(socketDesc); //bellegi temizler
    pthread_exit(NULL); // threadi sonlandırır 
}

int main() {
    WSADATA wsa; //Winsock kütüphanesinin başlatılması için kullanılan yapıdır
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock başlatilamadi. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    //TCP sunucusu için soketler ve adres yapıları tanımlanır
    int serverSock, clientSock, *newSock;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    //TCP/IP soketi oluşturulur,oluşturulamazsa hata mesajı yazdırılır
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == -1) {
        perror("Socket oluşturulamadi");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET; //IPv4 kullanılıyor
    serverAddr.sin_addr.s_addr = INADDR_ANY; //Sunucu, herhangi bir IP adresi üzerinden gelen bağlantıları kabul edecek
    serverAddr.sin_port = htons(8080); //Sunucu, 8080 portunu dinleyecek (htons(8080)).

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind başarisiz");
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    //soketin gelen bağlantılar için dinlemeye başlamasını sağlar
    listen(serverSock, 3);
    printf("Server dinlemede...\n");

    //sunucu soketi üzerinden gelen istemci bağlantılarını bekler
    while ((clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &addrLen))) {
        printf("Yeni baglanti kabul edildi.\n");

        pthread_t clientThread;
        newSock = malloc(sizeof(int));
        *newSock = clientSock;

        if (pthread_create(&clientThread, NULL, clientHandler, (void*)newSock) < 0) {
            perror("Thread oluşturulamadi");
            free(newSock);
            return 1;
        }

        pthread_detach(clientThread);
    }

    if (clientSock < 0) {
        perror("Bağlanti kabul edilmedi");
        closesocket(serverSock); //sunucunun tüm ağ bağlantılarını kapatarak soketi serbest bırakır.
        WSACleanup(); //Winsock kütüphanesini düzgün bir şekilde sonlandırır ve sistem kaynaklarını temizler.
        return 1;
    }

    closesocket(serverSock);
    WSACleanup();
    return 0;
}

/*
gcc -fdiagnostics-color=always -g C:\Users\zahidesoylu\Desktop\datacom\server.c -o C:\Users\zahidesoylu\Desktop\datacom\server.exe -lws2_32
C:\Users\zahidesoylu\Desktop\datacom\server.exe
*/