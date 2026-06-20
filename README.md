# rpi_ethernet

# P2P Ethernet Communication: Windows to Raspberry Pi 4 (UDP)

Bu proje; arada bir yönlendirici (router/modem) veya DHCP sunucusu olmaksızın, bir Windows bilgisayar ile Raspberry Pi 4 arasında doğrudan (Peer-to-Peer) Ethernet kablosu üzerinden düşük gecikmeli UDP soket haberleşmesinin nasıl kurulacağını adım adım açıklamaktadır. 

Özellikle gömülü sistemler, telemetri hatları ve Yazılım Tanımlı Radyo (SDR) uygulamalarında ham veri akışı (data streaming) sağlamak için minimalist ve yüksek performanslı bir altyapı sunar.

```text
p2p-ethernet-udp/
├── .gitignore
├── LICENSE
├── README.md
├── src/
│   ├── win_server.c
│   └── pi_client.c
└── .vscode/
    └── tasks.json

## 🛠️ Donanım ve Ağ Mimarisi

Raspberry Pi 4 üzerinde yer alan Gigabit Ethernet kontrolcüsü **Auto-MDIX** (Automatic Crossover) desteğine sahiptir. Bu nedenle düz (straight-through) veya çapraz (crossover) Ethernet kablosu kullanımı fark etmeksizin fiziksel katman (OSI Layer 1) bağlantısı doğrudan kurulabilir.

Arada bir DHCP sunucusu bulunmadığı için ağ katmanı (OSI Layer 3) mimarisi statik IP blokları üzerinden aşağıdaki gibi yapılandırılmıştır:

* **Windows Host IP:** `192.168.1.10` / Alt Ağ Maskesi: `255.255.255.0`
* **Raspberry Pi IP:** `192.168.1.20` / Alt Ağ Maskesi: `255.255.255.0`
* **Hedef Port:** `8080` (UDP)

Bilgisayarınızın arama çubuğuna ncpa.cpl yazdıktan sonra gelen Ağ Bağlantıları ekranında, Ethernet'e sağ tıklayarak özelliklere girin ve orada bulunan IPv4 ifadesine sağ tık yaparak özelliklerini açın ve bilgisayar host adresini, Alt Ağ maskesini yazın.


---

## 🚀 Adım Adım Kurulum Kılavuzu

### 1. Raspberry Pi Statik IP ve SSH Yapılandırması (Headless)
1.1. Raspberry Pi Tarafını Hazırlama (Ekransız/Headless)
Raspberry Pi halihazırda açık ve çalışır vaziyetteyse, ekran bağlıysa terminalden; bağlı değilse SD kartı bilgisayara bağlayarak şu ayarlar yapılır:
SD Kart üzerinden açılışta IP Atama (cmdline.txt):
SD kartın bootfs (veya boot) bölümüne girin.
cmdline.txt dosyasını Not Defteri ile açın. Satırın en sonuna gidin, bir adet boşluk bırakın ve şu parametreyi ekleyin: ip=192.168.1.20
SSH servisini otomatik tetiklemek için aynı kök dizinde ssh adında (uzantısı olmayan) boş bir dosya oluşturun.

Pi Çalışırken Terminalden Geçici IP Atama:
sudo ifconfig eth0 192.168.1.20 netmask 255.255.255.0 up

Pi Çalışırken SSH Servisini Aktif Etme:
sudo systemctl enable ssh
sudo systemctl start ssh

1.2. Windows Ağ Kartı Ayarları (IP Tanımlama)
Klavyeden Win + R tuşlarına basarak Çalıştır penceresini açın.
ncpa.cpl yazıp Enter'a basın (Ağ Bağlantıları penceresi açılır).
Raspberry Pi'yi bağladığınız Ethernet kartına sağ tıklayıp Özellikler seçeneğini seçin.
Listeden İnternet Protokolü Sürüm 4 (TCP/IPv4) seçeneğine çift tıklayın.
"Aşağıdaki IP adresini kullan" seçeneğini aktif edin:
IP adresi: 192.168.1.10
Alt ağ maskesi: 255.255.255.0
Varsayılan ağ geçidi kısmını boş bırakabilirsiniz.
Tamam diyerek pencereleri kapatın.

Bağlantı Testi (Windows CMD) için ping 192.168.1.20 yazmalısınız ve eğer yanıt alıyorsanız, SSH bağlantısını gerçekleştirebilirsiniz.
Bağlantı gerçekleştirmek için CMD'ye ssh pi@192.168.1.20 yazmalısınız.

## 💻 Kaynak Kodlar ve Derleme

### 1. Windows UDP Sunucu (`src/win_server.c`)
Windows ortamında soket katmanı mimarisi **Winsock2** API'sine dayanır. Bu nedenle uygulamanın `WSAStartup` ile initialize edilmesi gerekir. GCC/MinGW ile derleme yaparken Windows Socket kütüphanesinin dinamik olarak bağlanması için `-lws2_32` bayrağı (linker flag) kullanılmalıdır. Microsoft MSVC derleyicilerine özgü olan `#pragma comment` direktifleri derleme taşınabilirliği (portability) açısından koddan kaldırılmıştır.

```c
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

    // Winsock initialize işlemi
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock baslatilamadi. Hata Kodu: %d\n", WSAGetLastError());
        return 1;
    }

    // UDP Soketi oluşturma (SOCK_DGRAM)
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Soket olusturulamadi. Hata Kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Port bağlama (Bind)
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

Windows Üzerinde Standart Derleme Komutu:
Kodu standart bir GNU/MinGW ortamında derlemek için komut satırından kaynak kodun bulunduğu dizine giderek şu komutu çalıştırın:
gcc -O2 src/win_server.c -o win_server.exe -lws2_32
(Not: Eğer sisteminizde gcc komutu tanınmıyorsa, kullandığınız C/C++ derleyicisinin yürütülebilir bin dizininin Windows Sistem Ortam Değişkenleri altındaki Path listesine eklendiğinden emin olun).
