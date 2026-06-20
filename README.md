# P2P Ethernet Communication: Windows to Raspberry Pi 4 (UDP)

Bu proje; arada bir yönlendirici (router/modem) veya DHCP sunucusu olmaksızın, bir Windows bilgisayar ile Raspberry Pi 4 arasında doğrudan (Peer-to-Peer) Ethernet kablosu üzerinden düşük gecikmeli UDP soket haberleşmesinin nasıl kurulacağını adım adım açıklamaktadır. 

Özellikle gömülü sistemler, telemetri hatları ve Yazılım Tanımlı Radyo (SDR) uygulamalarında ham veri akışı (data streaming) sağlamak için minimalist ve yüksek performanslı bir altyapı sunar.

---

## 🛠️ Donanım ve Ağ Mimarisi

Raspberry Pi 4 üzerinde yer alan Gigabit Ethernet kontrolcüsü **Auto-MDIX** (Automatic Crossover) desteğine sahiptir. Bu nedenle düz (straight-through) veya çapraz (crossover) Ethernet kablosu kullanımı fark etmeksizin fiziksel katman (OSI Layer 1) bağlantısı doğrudan kurulabilir.

Arada bir DHCP sunucusu bulunmadığı için ağ katmanı (OSI Layer 3) mimarisi statik IP blokları üzerinden aşağıdaki gibi yapılandırılmıştır:

* **Windows Host IP:** `192.168.1.10` / Alt Ağ Maskesi: `255.255.255.0`
* **Raspberry Pi IP:** `192.168.1.20` / Alt Ağ Maskesi: `255.255.255.0`
* **Hedef Port:** `8080` (UDP)

---

## 🚀 Adım Adım Kurulum Kılavuzu

### 1. Raspberry Pi Statik IP ve SSH Yapılandırması (Headless)
Raspberry Pi halihazırda açık ve çalışır vaziyetteyse, ekran bağlıysa terminalden; bağlı değilse SD kartı bilgisayara bağlayarak şu ayarlar yapılır:

* **SD Kart üzerinden açılışta IP Atama (cmdline.txt):**
  1. SD kartın `bootfs` (veya `boot`) bölümüne girin.
  2. `cmdline.txt` dosyasını Not Defteri ile açın. Satırın en sonuna gidin, bir adet boşluk bırakın ve şu parametreyi ekleyin: `ip=192.168.1.20`
  3. SSH servisini otomatik tetiklemek için aynı kök dizinde `ssh` adında (uzantısı olmayan) boş bir dosya oluşturun.

* **Pi Çalışırken Terminalden Geçici IP Atama:**
  ```bash
  sudo ifconfig eth0 192.168.1.20 netmask 255.255.255.0 up

* **Pi Çalışırken SSH Servisini Aktif Etme:**
sudo systemctl enable ssh
sudo systemctl start ssh


**2. Windows Ağ Kartı Ayarları (IP Tanımlama)**
Klavyeden Win + R tuşlarına basarak Çalıştır penceresini açın.
ncpa.cpl yazıp Enter'a basın (Ağ Bağlantıları penceresi açılır).
Raspberry Pi'yi bağladığınız Ethernet kartına sağ tıklayıp Özellikler seçeneğini seçin.
Listeden İnternet Protokolü Sürüm 4 (TCP/IPv4) seçeneğine çift tıklayın.
"Aşağıdaki IP adresini kullan" seçeneğini aktif edin:
IP adresi: 192.168.1.10
Alt ağ maskesi: 255.255.255.0
Tamam diyerek pencereleri kapatın.
Bağlantıyı test etmek için Windows terminaline (cmd) ping 192.168.1.20 yazın. Yanıt alıyorsanız ssh pi@192.168.1.20 komutuyla Pi terminaline bağlanın.

*💻 **Derleme ve Çalıştırma**
1. Windows Tarafı (Sunucu)
PowerShell terminalini açın ve sistemdeki derleyici yolunu tanıtarak kodu derleyin:

PowerShell
$env:PATH = 'C:\Program Files\GNU Octave\Octave-11.1.0\mingw64\bin;' + $env:PATH
gcc -O2 win_server.c -o win_server.exe -lws2_32
.\win_server.exe
2. Raspberry Pi Tarafı (İstemci)
SSH üzerinden Pi terminaline bağlanın ve kodu derleyip çalıştırın:

Bash
gcc -O2 pi_client.c -o pi_client
./pi_client
🔍 Sorun Giderme (Troubleshooting)
Veri Akışı Yoksa: Windows Gelişmiş Güvenlik Duvarı harici UDP trafiğini engelliyor olabilir. Geçici olarak ağ profilinin duvarını kapatın veya gelişmiş ayarlardan 8080 portu için bir "Gelen Kuralı" (Inbound Rule) tanımlayın.
