# Firmware Arduino per sensore LoRaWAN a bassissimo consumo

**Dispensa didattica** — analisi delle parti che compongono un firmware per un end-device LoRaWAN in Class A ABP con deep sleep, sensori I²C e periferiche seriali. Il codice di riferimento è lo sketch per Heltec WiFi LoRa 32 V4 con Sensirion SCD41 (aria) e Quectel L76K (GNSS), ma i pattern architetturali qui descritti sono riusabili in qualsiasi progetto con vincoli energetici stringenti.

---

## 1. Perché "un firmware così"? Il contesto

Un end-device LoRaWAN alimentato a batteria (o solare) deve rispettare vincoli molto stringenti:

- **Consumo medio ≤ pochi mA** per durare mesi/anni su batteria.
- **Payload piccoli** (decine di byte) per rispettare il duty cycle EU868 (1% per sotto-banda).
- **Ciclo TX raro** (minuti/decine di minuti).
- **Frame counter** che deve crescere in modo monotono anche dopo reset ↔ persistenza.
- **Attivazione ABP**: chiavi statiche, niente handshake radio.

Il pattern che ne deriva è sempre lo stesso:

```
                    ┌──────────────┐
                    │ Deep sleep   │  ← ~50 µA (target)
                    │ (99% tempo)  │
                    └──────┬───────┘
                           │  timer RTC
                           ▼
                    ┌──────────────┐
                    │ Wake         │
                    │ Alimenta HW  │
                    │ Leggi sensori│
                    │ Trasmetti    │
                    │ Salva stato  │
                    └──────┬───────┘
                           │  ~1-60 s
                           ▼
                    (deep sleep)
```

Chiamiamo questo pattern **"wake-do-sleep"**. Tutta la nostra architettura serve a implementarlo bene.

---

## 2. Anatomia dello sketch

Uno sketch pensato bene per questo pattern ha 8 sezioni ordinate, sempre le stesse. Il nostro firmware le rispetta e le vediamo una per una.

```
1. Include e librerie
2. Configurazione (macro, credenziali, intervallo)
3. Definizioni pin
4. Definizione del payload (struct binaria)
5. Oggetti globali (radio, sensori, storage)
6. Utility (LED, alimentazioni, ADC, print)
7. Funzioni di init e read per ogni componente
8. setup() = ciclo completo di lavoro
```

Non c'è quasi niente in `loop()`: il device fa un giro di lavoro dentro `setup()` e poi torna a dormire. Il classico `void loop()` di Arduino resta vuoto.

---

## 3. Include e librerie

```cpp
#include <RadioLib.h>              // LoRa + LoRaWAN
#include <TinyGPSPlus.h>           // parser NMEA
#include <SensirionI2CScd4x.h>     // driver SCD41
#include <Wire.h>                  // I2C
#include <Preferences.h>           // NVS wrapper
#include <esp_sleep.h>             // deep sleep API
#include <driver/rtc_io.h>         // gpio_hold_en / gpio_deep_sleep_hold_en
```

**Perché queste scelte:**

- **RadioLib**: libreria "cross-chip" che astrae SX1262 / SX1276 / SX1268 / LR11xx con la stessa API. È la più mantenuta oggi per Arduino e ha uno stack LoRaWAN 1.0.3 e 1.1 integrato.
- **TinyGPSPlus**: parser NMEA header-only, senza allocazioni dinamiche, adatto a MCU con RAM limitata.
- **SensirionI2CScd4x**: driver ufficiale del produttore. Meglio dei fork community perché segue la sequenza di comandi documentata.
- **Preferences.h**: wrapper sopra la NVS (Non-Volatile Storage) di ESP-IDF. Serve a persistere valori tra un reset e l'altro (FCnt, timestamp ultima misura, ecc).
- **esp_sleep.h / driver/rtc_io.h**: API native ESP-IDF per il deep sleep. Non c'è equivalente Arduino portable, sono ESP32-specifiche.

**Riusabilità:** in un progetto con sensori diversi, le prime tre linee cambiano; le altre restano.

---

## 4. Configurazione: separare il "cosa" dal "come"

La parte iniziale del file espone al programmatore tutte le manopole che vorrà toccare senza dover navigare nel codice:

```cpp
#define TX_INTERVAL_PRESET  3        // 0..5 = 10s/20s/1m/5m/10m/30m
#define GPS_FIX_TIMEOUT_COLD_S  90
#define GPS_FIX_TIMEOUT_WARM_S  30
#define LORAWAN_SF  9
#define LORAWAN_FPORT  1
#define SCHEMA_ID  0x42

uint32_t devAddr = 0x260B262C;
uint8_t nwkSKey[16] = { /* ... */ };
uint8_t appSKey[16] = { /* ... */ };
```

**Regole di stile importanti:**

1. **Tutto ciò che è "tuning" sta in cima al file**. Chi riusa lo sketch non deve leggere il codice per scoprire dove cambiare le chiavi o l'intervallo.
2. **Preset numerici invece di secondi liberi**: `TX_INTERVAL_PRESET = 3` è più leggibile di `TX_INTERVAL_S = 300`, e impedisce di scrivere per errore `50` (secondi) invece di `500` (millisecondi).
3. **Il preset è tradotto in una tabella `const`**:
   ```cpp
   const uint32_t TX_INTERVAL_SECONDS[] = { 10, 20, 60, 300, 600, 1800 };
   const uint32_t TX_INTERVAL_S = TX_INTERVAL_SECONDS[TX_INTERVAL_PRESET];
   ```
   La tabella è calcolata a compile-time, non occupa RAM.
4. **Credenziali ABP come `uint8_t[]`**: le librerie LoRaWAN si aspettano array di byte, non stringhe. Mantenere il formato originale evita conversioni runtime.

**Riusabilità:** in un progetto diverso rifai solo il primo blocco. Il resto del file sopravvive.

---

## 5. Definizioni pin: raggruppate e commentate

```cpp
// --- LoRa SX1262 ---
#define PIN_LORA_NSS    8
#define PIN_LORA_SCK    9
// ...

// --- I2C SCD41 ---
#define PIN_I2C_SDA     7
#define PIN_I2C_SCL     6

// --- GPS L76K ---
#define PIN_GPS_RX     39   // GPS TX -> ESP RX
#define PIN_GPS_TX     38   // ESP TX -> GPS RX
#define PIN_VGNSS_CTRL 34   // HIGH = GPS acceso
```

**Principi di scrittura:**

- **Sempre un commento con il verso del segnale** per UART e I²C. È l'errore più comune: incrociare TX e RX.
- **Aggiungi il livello di attivazione** per i pin di controllo (`HIGH = acceso`, `LOW = attivo`). Vext_Ctrl sulla Heltec V4 è **active-low**, il che confonde sempre.
- **Raggruppa per periferica**, non per numero di pin. Cerca "GPS" e trovi tutto quello che serve.

**Attenzione tipica su ESP32-S3:**

- Non tutti i GPIO sono RTC-capable (importanti per il wake da deep sleep esterno).
- I GPIO 33-37 spesso hanno funzioni riservate (SPI flash, USB). Sulla V4 sono liberi grazie al package PSRAM.
- I GPIO 41-46 sono sull'header J3 della V4 e liberi per periferiche esterne.

**Riusabilità:** ogni scheda ha una sua pin map. Questa sezione va rifatta per intero, ma il pattern (nome logico → GPIO fisico) resta.

---

## 6. Payload: la struttura binaria come contratto

Il payload LoRaWAN è una sequenza di byte, non un JSON. La sua struttura è **il contratto** tra device e ricevente. Serve un modo per definirla in modo esplicito e verificabile.

```cpp
#pragma pack(push, 1)
struct Payload_v0x42 {
    uint8_t  schema_id;     // 0x42
    uint8_t  fix_quality;
    uint8_t  satellites;
    uint8_t  battery_pct;
    uint64_t timestamp;
    uint16_t co2_ppm;
    int16_t  temp_c100;
    uint16_t hum_pct100;
    uint16_t vbat_mv;
    int32_t  lat_e7;
    int32_t  lon_e7;
    int16_t  alt_m;
    uint16_t hdop_x100;
};
#pragma pack(pop)

static_assert(sizeof(Payload_v0x42) == 32, "Payload deve essere 32 byte");
```

**Elementi essenziali:**

1. **`#pragma pack(push, 1)`**: forza il compilatore a **non inserire byte di padding** tra i campi. Senza, il compilatore allineerebbe `uint64_t` a 8 byte, aggiungendo padding invisibile che romperebbe la compatibilità col decoder lato server. Questa direttiva serializza esattamente 1 byte alla volta.
2. **`#pragma pack(pop)`**: ripristina il pack di default dopo la struct, così non influenzi il resto del codice.
3. **`static_assert(sizeof(...) == 32)`**: verifica **a compile-time** che la struct sia esattamente della dimensione attesa. Se un giorno modifichi un campo senza aggiornare la costante, il compilatore ti ferma. Questo è un errore che a runtime sarebbe difficile da diagnosticare.
4. **`schema_id` come primo byte**: convenzione per **payload versionati/multi-schema**. Il ricevente può fare dispatch sul primo byte:  
   `if (buf[0] == 0x42) { decode_scd41_gps(); }`  
   Se un domani cambi la struct, aumenti l'id (`0x43`, `0x44`, …), e vecchi e nuovi device coesistono.
5. **Scale intere invece di float**: `temp_c100` è `int16_t` con valore = temperatura × 100. Occupa 2 byte invece di 4, ha 0.01 °C di risoluzione (più che sufficiente per l'aria), e non richiede la libreria float sull'MCU quando si scrive.
6. **Little-endian implicito**: ESP32 e la maggior parte dei processori embedded sono little-endian. Il decoder JavaScript nella dashboard legge esplicitamente con `getUint16(..., true)` per essere sicuri.

**Come tabella di riferimento nel codice:**

```
| offset | size | tipo | campo         | note                    |
|--------|------|------|---------------|-------------------------|
| 0      | 1    | u8   | schema_id     | 0x42                    |
| 1      | 1    | u8   | fix_quality   | 0=no, 1=GPS, 2=DGPS     |
| 2      | 1    | u8   | satellites    |                         |
| ...    | ...  | ...  | ...           | ...                     |
```

Tenerla come commento vicino alla struct rende la manutenzione molto più semplice.

**Riusabilità:** ogni progetto ha il suo payload. Il pattern (versione + campi + static_assert + tabella) resta identico.

---

## 7. Oggetti globali e RTC memory

```cpp
SX1262 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO1, PIN_LORA_RST, PIN_LORA_BUSY);
LoRaWANNode node(&radio, &EU868);
TinyGPSPlus gps;
HardwareSerial GpsSerial(1);
SensirionI2CScd4x scd4x;
Preferences prefs;
```

Standard, un oggetto per ogni componente. Nulla di speciale.

Ma poi c'è la parte importante:

```cpp
RTC_DATA_ATTR uint32_t bootCount   = 0;
RTC_DATA_ATTR uint32_t lastFCntUp  = 0;
RTC_DATA_ATTR int32_t  lastLat_e7  = 0;
RTC_DATA_ATTR int32_t  lastLon_e7  = 0;
RTC_DATA_ATTR bool     hasWarmData = false;
```

**Cosa è `RTC_DATA_ATTR`?**

L'ESP32-S3 ha **8 KB di RTC RAM** che restano alimentati anche durante il deep sleep. Le variabili marcate con `RTC_DATA_ATTR` vengono piazzate lì dal linker. Il valore sopravvive al deep sleep ma **si perde a power-off** o a reset hardware.

Questa memoria è perfetta per:

- **Frame counter LoRaWAN** (`lastFCntUp`): fondamentale, se torna a 0 il Network Server scarta i pacchetti come replay.
- **Contatore di boot** (`bootCount`): utile per il debug e le statistiche.
- **Ultima posizione GPS valida** (`lastLat/lastLon`): se un ciclo non ottiene il fix, si usa l'ultima nota.
- **Flag di "primo boot"** (`hasWarmData`): `false` alla prima esecuzione dopo power-on, `true` dai deep sleep successivi. Serve a decidere il timeout del GPS: 90 s a freddo, 30 s a caldo.

**Attenzione:** RTC memory è **volatile su power loss**. Se il device ha una batteria che si scarica del tutto e poi rientra col solare, il FCnt torna a 0. Per resistere anche a questo scenario servono le **Preferences NVS** (flash), a costo di scritture aggiuntive che consumano gli scarsi cicli di scrittura della flash.

**Regola pratica:**
- RTC memory → tutto ciò che è "stato del ciclo di lavoro"
- NVS → tutto ciò che deve resistere anche a power-off

Nel nostro sketch usiamo solo RTC memory per semplicità. In un progetto solare "vero" andrebbe aggiunta la NVS.

---

## 8. Utility: alimentazione, ADC, LED

Le funzioni di utility sono banali ma fondamentali per rendere leggibile il ciclo principale:

```cpp
void vextOn()   { pinMode(PIN_VEXT_CTRL, OUTPUT); digitalWrite(PIN_VEXT_CTRL, LOW); }
void vextOff()  { digitalWrite(PIN_VEXT_CTRL, HIGH); }
void gpsPowerOn()  { pinMode(PIN_VGNSS_CTRL, OUTPUT); digitalWrite(PIN_VGNSS_CTRL, HIGH); }
void gpsPowerOff() { digitalWrite(PIN_VGNSS_CTRL, LOW); }
```

**Perché estrarre in funzioni?** Perché il codice del `setup()` deve leggersi come una **narrazione dell'algoritmo**, non come dettagli hardware:

```cpp
vextOn();
gpsPowerOn();
delay(100);   // stabilizzazione
```

è molto più chiaro di:

```cpp
pinMode(36, OUTPUT); digitalWrite(36, LOW);
pinMode(34, OUTPUT); digitalWrite(34, HIGH);
delay(100);
```

E se un giorno la pin map cambia, cambi solo la funzione, non tutti i punti dello sketch.

### Lettura della batteria con partitore

```cpp
uint16_t readBatteryMv() {
    pinMode(PIN_ADC_CTRL, OUTPUT);
    digitalWrite(PIN_ADC_CTRL, LOW);   // abilita partitore
    delay(10);

    analogReadResolution(12);
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogReadMilliVolts(PIN_VBAT_READ);
    }
    uint32_t adcMv = sum / 16;

    digitalWrite(PIN_ADC_CTRL, HIGH);  // disabilita partitore
    return (uint16_t)(adcMv * 49 / 10);  // ratio 4.9x
}
```

Tre punti didattici:

1. **Partitore commutabile**: sulla Heltec V4 il partitore che divide `VBAT` per portarlo nel range ADC è attivato da un MOSFET comandato da `ADC_Ctrl`. È **normalmente disattivato** per non consumare (VBAT che fluisce attraverso il partitore = corrente sprecata). Si attiva solo per la lettura.
2. **Media di 16 letture**: l'ADC di ESP32 è rumoroso. Una singola lettura può oscillare di ~50 mV. Con 16 campioni la deviazione standard scende di un fattore 4. Il costo temporale è ~5 ms, trascurabile.
3. **Conversione intera**: `adcMv * 49 / 10` invece di `adcMv * 4.9`. Evita la libreria float, occupa meno flash e RAM. Funziona finché `adcMv < 2^32 / 49`, cioè fino a ~87 V (non ti succederà).

**Riusabilità:** il pattern "commuta partitore → media → converti → disattiva" è universale per ADC di batteria su schede a bassissimo consumo.

---

## 9. SCD41: driver di alto livello

```cpp
bool initScd41() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
    scd4x.begin(Wire);

    scd4x.stopPeriodicMeasurement();   // reset "sporco"
    delay(500);

    uint16_t sn0, sn1, sn2;
    if (scd4x.getSerialNumber(sn0, sn1, sn2)) return false;

    if (scd4x.startPeriodicMeasurement()) return false;
    return true;
}
```

**Note didattiche:**

- **`stopPeriodicMeasurement()` prima di tutto**: se il chip stava già misurando (perché il ciclo precedente non ha spento con calma), i comandi successivi verrebbero rifiutati. Fermare tutto prima è sempre sicuro.
- **Il ritardo di 500 ms**: richiesto dal datasheet dopo `stopPeriodicMeasurement`. Molti bug su SCD41 vengono da qui: chi non legge il datasheet non lo mette e ottiene errori intermittenti.
- **Verifica seriale**: chiedere il serial number all'inizio è il modo più semplice di verificare che il chip risponde. Se ritorna errore, l'I²C è rotto (cavi, pull-up, indirizzo).
- **Avvio misure periodiche**: da questo momento il chip fa una misura ogni 5 secondi, in autonomia. Non devi "chiedere" ogni volta.

```cpp
bool readScd41(uint16_t& co2, float& tempC, float& humRH, uint32_t timeout_ms = 10000) {
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        bool ready = false;
        if (!scd4x.getDataReadyFlag(ready) && ready) {
            return !scd4x.readMeasurement(co2, tempC, humRH);
        }
        delay(200);
    }
    return false;
}
```

**Pattern:**

- **Polling con timeout**: il modo pulito di aspettare qualcosa che non si sa quando arriverà. Il SCD41 impiega 5 secondi per la prima misura dopo lo start; se stiamo aspettando anche il GPS non è un problema, il tempo è "gratis".
- **Timeout esplicito**: mai lasciare cicli `while(true)` senza uscita. In produzione, un chip guasto o un cavo scollegato bloccherebbero tutto il device.
- **Passaggio per reference**: `uint16_t& co2, float& tempC, float& humRH`. Convenzione C++ per "output multipli". Se non ti piace, ritorna una struct.

**Riusabilità:** questo pattern (`initXxx` con verifica seriale + `readXxx` con timeout) funziona per praticamente qualsiasi sensore I²C con acquisizione periodica.

---

## 10. GPS: parser NMEA in polling

```cpp
void initGps() {
    GpsSerial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
}

bool waitForGpsFix(uint32_t timeout_s) {
    uint32_t start = millis();
    while (millis() - start < timeout_s * 1000UL) {
        while (GpsSerial.available()) {
            gps.encode(GpsSerial.read());
        }
        if (gps.location.isValid() && gps.location.isUpdated()
            && gps.hdop.isValid() && gps.hdop.hdop() < 50.0) {
            return true;
        }
        delay(50);
    }
    return false;
}
```

**Elementi didattici:**

1. **UART hardware, non software**: `HardwareSerial GpsSerial(1)` usa la seconda UART hardware dell'ESP32-S3. È molto più affidabile di SoftwareSerial, che a 9600 baud comunque funzionerebbe ma sprecherebbe CPU.
2. **`gps.encode(byte)` in loop stretto**: TinyGPSPlus è un parser a stati. Gli mandi un byte alla volta e lui aggiorna internamente lat/lon/fix/ecc. È non-bloccante: se non ha ancora abbastanza dati, ritorna e riprovi.
3. **Doppia validità**: `isValid() && isUpdated()`. `isValid()` è "abbiamo mai avuto un fix"; `isUpdated()` è "questo fix è nuovo dall'ultima volta che l'hai letto". Usare entrambe evita di considerare valido un fix vecchio conservato in memoria.
4. **HDOP < 50**: filtro di qualità basso. HDOP (Horizontal Dilution Of Precision) è un indicatore di geometria satellitare. Valori sotto 2-5 sono ottimi, sopra 10 sono scarsi, sopra 50 è quasi rumore. Rigettiamo i fix veramente inutili.
5. **`delay(50)` invece di busy loop**: cede tempo al task scheduler di FreeRTOS. Su ESP32 il busy loop stretto può causare il **watchdog reset**.

**Riusabilità:** il pattern "leggi UART fino a evento" con timeout si applica a qualsiasi periferica seriale (modem GSM, lidar, sensori ultrasonici).

---

## 11. LoRaWAN con RadioLib

```cpp
bool initLoRaWAN() {
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);

    int16_t state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) return false;

    state = node.beginABP(devAddr, nwkSKey, nullptr, appSKey);
    if (state != RADIOLIB_ERR_NONE) return false;

    node.setDatarate(LORAWAN_SF <= 12 ? (12 - LORAWAN_SF) : 0);
    return true;
}
```

**Note:**

- **`SPI.begin(...)` esplicito**: sull'ESP32-S3 SPI non ha pin fissi come sull'AVR. Vanno specificati.
- **`radio.begin()`** ← inizializza il chip radio a livello fisico (registri, PA, modulazione).
- **`node.beginABP(...)`** ← inizializza lo stack LoRaWAN in modalità ABP. Il `nullptr` in posizione 3 è per il **JoinEUI**, non usato in ABP.
- **DR = 12 - SF**: RadioLib usa i "Data Rate" LoRaWAN (DR0..DR5 per EU868), che vanno mappati dagli SF (SF12 = DR0, SF7 = DR5).

```cpp
bool sendPayload(const uint8_t* buf, size_t len) {
    int16_t state = node.sendReceive((uint8_t*)buf, len, LORAWAN_FPORT);
    return state >= RADIOLIB_ERR_NONE;
}
```

**Perché `sendReceive` e non solo `send`?**

`sendReceive` fa uplink + apre le due finestre di ricezione RX1 e RX2 come previsto da Class A. Anche se non ti aspetti un downlink, questo è il modo corretto di trasmettere: il Network Server potrebbe mandarti MAC commands (LinkADRReq, DevStatusReq, ecc.) e vanno raccolti nelle finestre RX.

**Il valore ritornato:**
- `RADIOLIB_ERR_NONE` (0) → uplink ok, nessun downlink
- `RADIOLIB_LORAWAN_DOWNLINK_RECEIVED` (> 0) → uplink ok, downlink ricevuto
- valori negativi → errore

**Riusabilità:** il pattern `init radio → init stack → send/sendReceive` è identico per SX1276, SX1268, LR1121. Cambia solo la classe di oggetto radio.

---

## 12. Deep sleep: il cuore del risparmio energetico

```cpp
void enterDeepSleep(uint32_t seconds) {
    Serial.flush();

    gpsPowerOff();
    vextOff();
    ledOff();

    gpio_hold_en((gpio_num_t)PIN_VEXT_CTRL);
    gpio_hold_en((gpio_num_t)PIN_VGNSS_CTRL);
    gpio_deep_sleep_hold_en();

    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}
```

Questo è il pezzo che decide se il tuo device dura giorni o settimane. Analizziamo ogni riga.

**`Serial.flush()`**: forza lo svuotamento del buffer UART prima di andare in sleep. Senza, l'ultima riga di log si perderebbe.

**`gpsPowerOff() / vextOff() / ledOff()`**: spengono tutte le periferiche. Ogni mA risparmiato conta.

**`gpio_hold_en(pin)`**: congela il livello attuale del pin. Quando il chip entra in deep sleep i registri GPIO vengono spenti; senza hold, i pin diventano floating e le periferiche potrebbero riaccendersi (specialmente su pin con pull-up debole). Senza questa riga, spegnere Vext è inutile perché tornerebbe HIGH in modo imprevedibile.

**`gpio_deep_sleep_hold_en()`**: conferma "l'hold vale anche in deep sleep, non solo in light sleep". Riga globale che si applica a tutti i pin con `hold_en` attivo.

**`esp_sleep_enable_timer_wakeup(microsec)`**: configura il timer RTC per il risveglio. L'unità è **microsecondi come uint64_t**: fondamentale usare `1000000ULL` (unsigned long long) altrimenti overflow per sleep sopra ~35 minuti.

**`esp_deep_sleep_start()`**: la funzione da cui **non si torna mai**. Il chip si spegne fino al prossimo timer trigger, dopo il quale ripartirà eseguendo `setup()` dall'inizio.

**Costi tipici:**

| Componente | Consumo |
|---|---|
| ESP32-S3 in deep sleep | ~10 µA |
| SX1262 in sleep | ~1 µA |
| SCD41 spento (Vext off) | 0 |
| L76K spento (VGNSS off) | 0 |
| **Totale target** | **~15 µA** |

Se qualcosa non viene spento correttamente:

| Errore | Costo aggiuntivo |
|---|---|
| GPS resta acceso | +25 mA (letale) |
| SCD41 in idle | +0.5 mA |
| LED acceso | +5-15 mA |
| Regolatore inefficiente a vuoto | +1-5 mA |

Su una batteria da 1000 mAh, la differenza tra 15 µA e 25 mA è tra **7 anni** e **40 ore**. Il codice del deep sleep va fatto bene.

---

## 13. Il setup() come narrazione dell'algoritmo

Grazie alle sezioni precedenti, il `setup()` legge come uno pseudocodice italiano:

```cpp
void setup() {
    Serial.begin(115200);
    delay(200);
    bootCount++;

    // rilascia gli hold dal deep sleep precedente
    gpio_hold_dis(...);

    pinMode(PIN_LED, OUTPUT);
    ledOn();

    uint32_t cycleStart = millis();

    // 1) alimenta periferiche
    vextOn();
    gpsPowerOn();
    delay(100);

    // 2) init GPS
    initGps();

    // 3) init SCD41
    initScd41();

    // 4) attendi fix GPS (nel frattempo SCD41 misura in autonomia)
    bool gpsOk = waitForGpsFix(hasWarmData ? 30 : 90);

    // 5) leggi SCD41
    uint16_t co2; float tempC, humRH;
    bool scdOk = readScd41(co2, tempC, humRH);

    // 6) leggi batteria
    uint16_t vbat_mv = readBatteryMv();

    // 7) costruisci payload
    Payload_v0x42 payload = {};
    payload.schema_id = SCHEMA_ID;
    // ... (assegnazioni)

    // 8) trasmetti
    if (initLoRaWAN()) sendPayload(...);

    // 9) spegni sensori
    scd4x.stopPeriodicMeasurement();

    // 10) deep sleep per (intervallo - tempo trascorso)
    uint32_t elapsed_s = (millis() - cycleStart) / 1000;
    uint32_t sleep_s = TX_INTERVAL_S > elapsed_s ? TX_INTERVAL_S - elapsed_s : 1;
    enterDeepSleep(sleep_s);
}

void loop() {}
```

**Punti didattici:**

1. **`bootCount++` all'inizio**: statistica gratis, utile per capire quanto sta durando la batteria e diagnosticare reset anomali.
2. **Rilascio degli hold prima di tutto**: se non lo fai, `digitalWrite()` non ha effetto sui pin ancora congelati dal deep sleep.
3. **`cycleStart = millis()`**: registra l'inizio del ciclo per calcolare quanto abbiamo speso. Fondamentale per dormire "l'esatto tempo residuo" invece di un intervallo fisso: se un ciclo di lavoro dura 30 s ma l'intervallo è 60 s, dobbiamo dormire 30, non 60.
4. **Parallelismo naturale tra GPS e SCD41**: mentre TinyGPSPlus aspetta il fix, il SCD41 acquisisce in autonomia. Quando arriviamo a leggere il SCD41 (passo 5), i suoi dati sono pronti "per magia". È un pattern classico: fai partire le operazioni lente prima, quelle veloci dopo.
5. **`loop()` vuoto**: sappiamo che non tornerà mai. Il device fa un giro completo dentro `setup()` e va in deep sleep. Al risveglio, `setup()` gira di nuovo da capo.

---

## 14. Pattern architetturali generali

Da questo firmware si estraggono principi validi per qualsiasi progetto embedded a bassi consumi.

### 14.1 Il ciclo "cold path"

Il device passa il 99% del tempo in deep sleep. Il "cold path" (l'esecuzione da wake a nuovo sleep) deve:

- **Essere il più corto possibile** (secondi, non minuti). Ogni secondo attivo = più batteria consumata.
- **Fare tutto in un colpo** (leggi tutti i sensori, spedisci tutto, dormi).
- **Non usare interrupt asincroni** salvo per operazioni brevi (es. IRQ radio). Il codice sincrono è più facile da ragionare.

### 14.2 Alimentazioni gestite dal firmware

Se hai una periferica che consuma > 100 µA in idle, la alimenti da un pin commutabile e la spegni quando non serve. Regola:

- Cavo VCC del sensore → pin GPIO o linea Vext controllata (non 3V3 fisso).
- Prima del deep sleep: spegni la periferica.
- `gpio_hold_en()` sulla linea di comando dell'alimentazione, per essere sicuro che resti spenta.

### 14.3 Stato persistente

Tre livelli di persistenza in ordine di robustezza:

1. **RAM ordinaria**: perduta a ogni deep sleep. Solo per stato di un singolo ciclo.
2. **RTC memory (`RTC_DATA_ATTR`)**: sopravvive al deep sleep, perduta al power-off. Ottima per FCnt, contatori, ultime letture.
3. **NVS (`Preferences.h`)**: sopravvive a tutto, ma ogni scrittura consuma un ciclo dei ~100k della flash. Usare solo per dati critici che devono resistere anche a batteria scarica.

Regola: **RTC memory di default, NVS solo quando serve davvero**.

### 14.4 Payload versionati

Ogni volta che il tuo firmware trasmette un formato di dati diverso, il primo byte del payload deve essere un **schema_id**. Il vantaggio è enorme: puoi aggiornare il firmware di alcuni device senza aggiornare quelli vecchi, e il decoder sul cloud sa distinguere da solo. Vecchi e nuovi device coesistono nella stessa rete.

### 14.5 Config in testa al file

Chiunque riusi il tuo sketch deve trovare in cima al file tutte le manopole (intervallo, credenziali, pin critici, ID). Se deve navigare il codice per cambiare l'intervallo TX, il tuo file è mal scritto.

### 14.6 Timeout su tutto

Nessun `while(true)` senza uscita. Nessun `while(sensor.notReady())` senza timeout. Un cavo scollegato non deve bloccare il device: al peggio manda un payload con valori nulli e riprova al prossimo ciclo.

---

## 15. Checklist per un nuovo progetto

Quando parti da questo scheletro per un progetto diverso, ecco cosa cambia (in ordine):

- [ ] **Include librerie**: rimuovi/aggiungi driver dei sensori
- [ ] **Config in cima**: intervalli, credenziali ABP, SF, schema_id
- [ ] **Pin definitions**: rifai dalla pin map della tua scheda
- [ ] **Struct payload**: nuovi campi, nuovo schema_id, `static_assert` corretto
- [ ] **Oggetti globali**: sostituisci gli oggetti dei sensori
- [ ] **Utility**: le funzioni di power/led/adc restano quasi identiche
- [ ] **initXxx / readXxx**: una coppia per ogni sensore, secondo il pattern polling+timeout
- [ ] **initLoRaWAN / sendPayload**: quasi invariati (cambia solo se cambi regione)
- [ ] **enterDeepSleep**: quasi invariato, verifica quali pin fare `gpio_hold_en`
- [ ] **setup()**: rileggilo come narrazione, deve tornare
- [ ] **Test energetico** con multimetro in serie: verifica che il consumo in deep sleep sia sotto 100 µA
- [ ] **Test di durata**: fai girare per 24 h e verifica che boot count coincida con l'intervallo

---

## 16. Errori tipici da evitare

1. **Dimenticare `#pragma pack(1)`** sulla struct payload → 4-8 byte di padding nascosto → decoder rotto.
2. **Non fare `gpio_hold_en`** prima del deep sleep → periferiche riaccese in sleep → batteria scarica.
3. **`while(true)` senza timeout** su sensori → device bloccato al primo cavo che si allenta.
4. **Frame counter reset** dopo ogni power-off → Network Server rifiuta i pacchetti come replay.
5. **Chiamare `Serial.print` in un ciclo stretto** → USB CDC blocca fino a receiver → falso appeared-to-hang.
6. **Non calcolare "sleep residuo"** → intervallo effettivo = (attivo + interval), non interval.
7. **ADC lettura singola** → valori rumorosi, batteria letta male.
8. **Partitore ADC sempre attivo** → 100 µA sprecati continuamente.
9. **`analogRead` senza `analogReadResolution`** → dipendenze dal core Arduino version.
10. **Payload con float diretti** → dimensione doppia, richiede libreria float sul decoder.

---

## 17. Riferimenti

- **RadioLib** — https://github.com/jgromes/RadioLib (Jan Gromeš)
- **TinyGPSPlus** — https://github.com/mikalhart/TinyGPSPlus
- **Sensirion I2C SCD4x** — https://github.com/Sensirion/arduino-i2c-scd4x
- **ESP32 deep sleep API** — https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/sleep_modes.html
- **LoRa Alliance Regional Parameters** — https://lora-alliance.org/resource_hub/rp2-1-0-3-lorawan-regional-parameters/
- **Heltec V4 pin map** — https://docs.heltec.org/en/node/esp32/wifi_lora_32_v4/
