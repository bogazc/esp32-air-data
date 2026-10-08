# System Monitorowania Mikroklimatu IoT

Ten projekt jest systemem do monitorowania warunków mikroklimatu (temperatury, wilgotności, ciśnienia, jakości powietrza) przy użyciu czujników podłączonych do ESP32. Dane są przesyłane za pomocą protokołu MQTT, zbierane przez dedykowany serwis i udostępniane do wizualizacji w notatniku Jupyter.

## Architektura i Przepływ Danych

System działa w oparciu o architekturę mikroserwisów orkiestrowaną przez `docker-compose`.

1.  **Urządzenie ESP32 (`firmware_esp32`)**: Zbiera dane z czujników (np. BME280, PMS5003).
2.  **Publikacja MQTT**: Oprogramowanie na ESP32 publikuje odczyty w formacie JSON do odpowiednich tematów na brokerze MQTT.
3.  **Broker Mosquitto (`mosquitto`)**: Odbiera wiadomości od ESP32 i przekazuje je do subskrybentów.
4.  **Kolektor Danych (`data_collector`)**: Serwis w Pythonie subskrybuje tematy MQTT, odbiera dane, przetwarza je i zapisuje (np. do bazy danych lub pliku CSV).
5.  **Analiza Danych (`notebooks`)**: Notatnik Jupyter służy do analizy zebranych danych historycznych.


## Komponenty Projektu

-   `firmware_esp32/`: Oprogramowanie dla mikrokontrolera ESP32 (PlatformIO).
-   `mosquitto/`: Konfiguracja brokera MQTT.
-   `data_collector/`: Kolektor danych w Pythonie wraz z analizą `air_quality_analysis.ipynb`.
-   `dashboard/`: Aplikacja do wizualizacji danych - **DO ZROBIENIA**.
-   `docker-compose.yml`: Plik orkiestrujący wszystkie usługi.

## Sprzęt

Do budowy części sprzętowej wykorzystano:
-   Mikrokontroler: **ESP32 z modułem ESP-WROOM-32 zgodny z ESP32-DevKit**
-   8 przewodów połączeniowych męsko-żeńskich
-   Czujniki:
    -   **BME280**: Temperatura, wilgotność, ciśnienie.
    -   **G5 PMS5003**: Stężenie pyłów zawieszonych (PM1.0, PM2.5, PM10).
-   Zasilanie: Zasilacz USB 5V.

## Schemat Połączeń

<p align="center"><img width="4296" height="2484" alt="image" src="https://github.com/user-attachments/assets/ae74a20a-7c72-499e-894f-1850e1160fee" />
</p>

#### Czujnik PMS5003 (Jakość Powietrza)

Czujnik komunikuje się przez interfejs UART.

| Pin PMS5003 | Pin ESP32 | Opis                  |
| :---------- | :-------- | :-------------------- |
| **VCC**     | **VIN**    | Zasilanie             |
| **GND**     | **GND**   | Masa                  |
| **TXD**     | **GPIO16 (RX2)** | Transmisja danych (TX czujnika -> RX ESP32) |
| **RXD**     | **GPIO17 (TX2)** | Odbiór danych (RX czujnika <- TX ESP32)  |

#### Czujnik BME280 (Temperatura, Wilgotność, Ciśnienie)

Czujnik komunikuje się przez interfejs I2C.

| Pin BME280 | Pin ESP32 | Opis                  |
| :--------- | :-------- | :-------------------- |
| **VIN**    | **VCC**  | Zasilanie             |
| **GND**    | **GND**   | Masa                  |
| **SCL**    | **GPIO22**| Linia zegara I2C      |
| **SDA**    | **GPIO21**| Linia danych I2C      |

## Aby uruchomić projekt, postępuj zgodnie z poniższymi krokami.
### Krok 1: Klonowanie Repozytorium

Najpierw sklonuj repozytorium na swój lokalny komputer.
```bash
git clone https://github.com/bogazc/esp32-air-data.git
cd esp32-air-data
```

### Krok 2: Konfiguracja Oprogramowania ESP32

Przed wgraniem oprogramowania na mikrokontroler, musisz skonfigurować swoje dane uwierzytelniające do sieci Wi-Fi oraz brokera MQTT.

1.  Przejdź do katalogu `firmware_esp32/sensors/include/`.
2.  Znajdź plik `secret_example.h` i zmień jego nazwę na `secret.h`.
3.  Otwórz plik `secret.h` i uzupełnij go swoimi danymi:
    ```c
    const char* WIFI_SSID = "YOUR_SSID";
    const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
    const char* MQTT_SERVER = "YOUR_BROKER_IP";
    ```
4.  Wgraj oprogramowanie na ESP32 używając PlatformIO.

### Krok 3: Uruchomienie Usług Docker

Gdy część sprzętowa jest gotowa i skonfigurowana, możesz uruchomić wszystkie usługi backendowe (broker MQTT, kolektor danych, pulpit).

Upewnij się, że jesteś w głównym katalogu projektu.
```bash
# To polecenie pobierze obrazy (jeśli ich nie ma), zbuduje i uruchomi kontenery
docker-compose up -d
```

### Zatrzymywanie Aplikacji

Aby zatrzymać i usunąć kontenery, użyj polecenia:
```bash
docker-compose down
```

## Wstępna Analiza Danych (EDA)

W katalogu `data_collector/` znajduje się notatnik Jupyter (`air_quality_analysis.ipynb`), który posłużył do przeprowadzenia wstępnej analizy eksploracyjnej (EDA) i sprawdzenia integralności zebranych danych. Poniżej znajduje się krótkie streszczenie wyników oparte na przykładowej próbce z okna czasowego wynoszącego niespełna 4 godziny. **Pełna analiza, wraz z kodem i wizualizacjami, dostępna jest we wspomnianym pliku `.ipynb`.**

**Główne wnioski z analizy wstępnej:**

1. **Wysoka niezawodność systemu (Brak utraty danych)**
   Analiza wykazała **absolutną kompletność zbioru**. W badanym okresie nie wystąpiły żadne wartości brakujące (`NaN`) ani duplikaty w indeksie czasowym. Oznacza to, że ESP32, broker MQTT oraz skrypt zapisujący działają niezwykle stabilnie, bez przerw w zasilaniu, utraty pakietów w sieci Wi-Fi czy zjawiska tzw. *double-loggingu*. Zapis następował z idealną, 30-sekundową regularnością.

2. **Jakość powietrza (PMS5003)**
   Odczyty pyłów zawieszonych (PM1.0, PM2.5, PM10) oscylowały wokół bardzo niskich wartości (średnio 5-7 µg/m³). Potwierdza to doskonałą jakość powietrza w monitorowanym pomieszczeniu w trakcie badania (brak zadymienia czy napływu zanieczyszczeń z zewnątrz).

3. **Analiza profilu termicznego (BME280)**
   Wizualizacja odczytów temperatury pozwoliła zaobserwować dwa wyraźne reżimy termiczne:
   * **Swobodne wychładzanie:** Powolny, liniowy spadek temperatury zgodnie z prawem stygnięcia Newtona. Minimum zjawiska zanotowano o 16:01 (20,66°C).
   * **Wymuszony wzrost temperatury:** Po godzinie 16:30 zaobserwowano nagły, bardzo stromy wzrost, sugerujący aktywację zewnętrznego źródła ciepła w niewielkiej kubaturze (np. uruchomienie kaloryfera). W ciągu niespełna 3 godzin temperatura wzrosła o ponad 2,1°C (osiągając maksimum 22,83°C o 18:44).

Powyższe wyniki potwierdzają poprawność montażu sprzętu, prawidłową kalibrację sensorów oraz skuteczność całego pipeline'u przesyłania danych.
