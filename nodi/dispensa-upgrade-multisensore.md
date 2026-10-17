# Upgrade multisensore: SDS011, CCS811 e MPL3115A2 sul nodo LoRaWAN

Questa dispensa descrive l'**upgrade** del nodo LoRaWAN low-power: al sensore di CO₂ SCD41 e al GPS L76K si aggiungono

- **SDS011** (Nova Fitness): polveri sottili **PM2.5 e PM10**;
- **CJMCU-811** (chip **CCS811** di ScioSense/ams): composti organici volatili **TVOC** ed **eCO₂** stimata;
- **MPL3115A2** (NXP): **pressione atmosferica** e temperatura.

Vale per tutte e tre le schede del progetto, con un firmware per ciascuna:

| Scheda | Dispensa di base | Firmware dell'upgrade |
|---|---|---|
| Heltec WiFi LoRa 32 V4 | [dispensa-firmware-lorawan-lowpower.md](dispensa-firmware-lorawan-lowpower.md) | [`FWs/heltec_v4_multisensor_lorawan.ino`](FWs/heltec_v4_multisensor_lorawan.ino) |
| Heltec Wireless Stick Lite V3 | [dispensa-firmware-lorawan-lowpower-wsl-v3.md](dispensa-firmware-lorawan-lowpower-wsl-v3.md) | [`FWs/heltec_wsl_v3_multisensor_lorawan.ino`](FWs/heltec_wsl_v3_multisensor_lorawan.ino) |
| Heltec WiFi Kit 32 V3 + RFM95W | [dispensa-firmware-lorawan-lowpower-kit32-v3.md](dispensa-firmware-lorawan-lowpower-kit32-v3.md) | [`FWs/heltec_kit32_v3_rfm95_multisensor_lorawan.ino`](FWs/heltec_kit32_v3_rfm95_multisensor_lorawan.ino) |

Il **codec ChirpStack** è unico: [`codecs/chirpstack_codec.js`](codecs/chirpstack_codec.js), aggiornato in modo da decodificare sia i payload vecchi (0x42) sia quelli nuovi (0x44).

Tutto ciò che non è descritto qui (LoRaWAN, OTAA, persistenza, watchdog, downlink, menu seriale, protezione della batteria) funziona esattamente come nella dispensa di base della propria scheda.

## Indice

1. [Upgrade e downgrade: un solo firmware](#upgrade-e-downgrade-un-solo-firmware)
2. [I tre sensori in breve](#i-tre-sensori-in-breve)
3. [Alimentazione e bus: le scelte di progetto](#alimentazione-e-bus-le-scelte-di-progetto)
4. [Collegamenti per ogni scheda](#collegamenti-per-ogni-scheda)
5. [Il ciclo del firmware con i nuovi sensori](#il-ciclo-del-firmware-con-i-nuovi-sensori)
6. [I driver: come si parla con i sensori](#i-driver-come-si-parla-con-i-sensori)
7. [Il payload 0x44 a blocchi](#il-payload-0x44-a-blocchi)
8. [Payload state 0x43 esteso](#payload-state-0x43-esteso)
9. [Nuovi comandi downlink](#nuovi-comandi-downlink)
10. [Il codec ChirpStack](#il-codec-chirpstack)
11. [Consumi e autonomia](#consumi-e-autonomia)
12. [Limiti delle misure](#limiti-delle-misure)
13. [Verifiche al banco](#verifiche-al-banco)

---

## Upgrade e downgrade: un solo firmware

Invece di scrivere un firmware diverso per ogni combinazione di sensori, il firmware dell'upgrade li contiene **tutti**, ognuno con il proprio interruttore di compilazione:

```cpp
#define ENABLE_SCD41       1   // CO2 / T / RH (Sensirion SCD41, I2C su Vext)
#define ENABLE_GPS         1   // posizione (Quectel L76K, UART1)
#define ENABLE_SDS011      1   // PM2.5 / PM10 (Nova SDS011, UART2, 5 V da boost)
#define ENABLE_CCS811      1   // TVOC / eCO2 (CJMCU-811, I2C Wire1, sempre acceso)
#define ENABLE_MPL3115A2   1   // pressione / T (MPL3115A2, I2C su Vext)
```

Il **downgrade** consiste nel mettere a 0 i sensori che non hai montato:

- i tre sensori nuovi a 0 **spariscono del tutto** dal binario (`#if ENABLE_... #endif`);
- SCD41 e GPS a 0 restano nel codice ma non vengono mai accesi. Per questo richiedono `ENABLE_SENSOR_DISABLE = 1`, altrimenti la compilazione si ferma con un `#error`;
- un sensore escluso **non compare nel payload**, che diventa più corto (vedi [Il payload 0x44 a blocchi](#il-payload-0x44-a-blocchi)).

Ogni sensore compilato si può poi spegnere anche **a runtime**, con un downlink, senza riflashare (vedi [Nuovi comandi downlink](#nuovi-comandi-downlink)).

I firmware con lo schema 0x42 restano disponibili e invariati: sono la versione "stabile" di partenza. Il codec li decodifica entrambi, quindi un nodo vecchio e uno aggiornato possono convivere sullo stesso Network Server.

Tutti e tre i firmware dell'upgrade sono stati **compilati** con il core esp32 3.3.12 e RadioLib 7.8.1, sia nella configurazione completa sia in quella minima (solo GPS), senza errori né avvisi. Il codec è stato provato con payload generati dalla stessa funzione del firmware. Sul campo, però, i collegamenti vanno verificati al banco (vedi [Verifiche al banco](#verifiche-al-banco)).

---

## I tre sensori in breve

| | SDS011 | CJMCU-811 (CCS811) | MPL3115A2 |
|---|---|---|---|
| Misura | PM2.5 e PM10 (µg/m³) | TVOC (ppb), eCO₂ stimata (ppm) | pressione (hPa), temperatura |
| Principio | diffusione laser, con ventola | ossido metallico riscaldato (MOX) | piezoresistivo MEMS |
| Interfaccia | UART 9600 baud, livelli 3,3 V | I²C, indirizzo **0x5A** (ADD a GND) | I²C, indirizzo **0x60** (fisso) |
| Alimentazione | **5 V**, circa 70 mA (fino a 100 mA) | 1,8–3,6 V | 1,95–3,6 V |
| Riscaldamento | ~30 s di ventilazione a ogni misura | **20 minuti** dopo ogni accensione | nessuno |
| Come lo spegniamo | togliendo l'alimentazione (interruttore + boost) | **non lo spegniamo**: modo a basso consumo | con Vext, insieme al SCD41 |
| Nel payload | PM2.5, PM10 (×10) | eCO₂, TVOC, codice d'errore | pressione (hPa ×10), temperatura (×100) |

Il CJMCU-811 ha anche i pin **WAK**, **INT**, **RST** e **ADD**: WAK e ADD vanno a massa, RST e INT restano scollegati. Sulla scheda dell'MPL3115A2 i due pin in più sono **INT1** e **INT2**, che non servono.

---

## Alimentazione e bus: le scelte di progetto

Ogni sensore ha un'esigenza diversa, e il collegamento nasce da queste.

### SDS011: 5 V solo durante la misura

L'SDS011 vuole **5 V** e assorbe circa 70 mA per tutta la misura, con picchi alla partenza della ventola. Il pin 5V delle Heltec ha tensione solo con l'USB collegato, e Vext dà 3,3 V. Per questo serve un **boost a 5 V**.

Il boost non va collegato ai 3,3 V della scheda: chiederebbe al regolatore della Heltec oltre 100 mA in più, insieme ai picchi del WiFi, della radio e del SCD41. Va collegato **alla batteria** (VBAT), attraverso un **interruttore lato alto**:

- un **P-MOSFET AO3401** con il source su VBAT e il drain sull'ingresso del boost, con 100 kΩ tra gate e source;
- un **N-MOSFET 2N7002** che porta a massa il gate del P quando **SDS_EN** è alto, con 100 kΩ tra il suo gate e GND.

Con SDS_EN alto il SDS011 è alimentato; con SDS_EN basso, o con il pin flottante durante il deep sleep, è spento del tutto: non resta nemmeno la piccola corrente del boost a vuoto. È lo stesso circuito dell'uscita sensore dello stampato ([PCB.md](PCB.md), ponticello JP1 su BAT e JP2 su BST) e del dimensionamento visto in [MOSFET.md](MOSFET.md). Se all'accensione la scheda si resetta, aggiungi il soft-start descritto lì.

Sull'uscita del boost serve un condensatore da **470 µF** vicino al sensore, per i picchi della ventola.

Del SDS011 si collega solo il **TX** (verso un ingresso dell'ESP32): il firmware non gli manda comandi, perché lo spegne togliendo l'alimentazione. Alla riaccensione il sensore riparte da solo nella modalità che invia una misura al secondo.

### CCS811: sempre acceso, su un bus tutto suo

Il CCS811 è un sensore a ossido metallico con un piccolo riscaldatore. Dopo ogni accensione, e dopo ogni cambio di modalità, ha bisogno di **20 minuti di condizionamento** prima di dare valori affidabili. Se lo spegnessimo a ogni deep sleep come il SCD41, non uscirebbe mai da quei 20 minuti.

Per questo:

- è alimentato **sempre** dal 3V3 della scheda, non da Vext;
- lavora nel **modo 3**: fa da solo una misura ogni 60 secondi, anche mentre l'ESP32 dorme. Secondo il datasheet consuma in media circa 1,2 mW, cioè meno di 1 mA;
- il pin **WAK** è a massa: il sensore resta sempre raggiungibile sull'I²C.

Il CCS811 non può stare sullo stesso bus I²C del SCD41. Quel bus è alimentato da Vext, che durante il deep sleep è spento. I pull-up del CCS811, alimentati sempre, terrebbero alte SDA e SCL, e la corrente entrerebbe nel SCD41 spento attraverso i diodi di protezione dei suoi pin. Il sensore resterebbe mezzo alimentato, con consumi e comportamenti imprevedibili. L'ESP32-S3 ha due controller I²C: il CCS811 usa il secondo, **Wire1**, su due pin dedicati.

### MPL3115A2: insieme al SCD41

L'MPL3115A2 non ha bisogno di riscaldarsi: si accende, fa una misura in circa mezzo secondo e torna in standby. Va quindi **su Vext**, sullo **stesso bus Wire** del SCD41 (GPIO 7 e 6), e si spegne con lui. Gli indirizzi sono diversi (0x60 contro 0x62), quindi non c'è conflitto. Le resistenze di pull-up della sua schedina sono alimentate da Vext come quelle del SCD41, e si spengono insieme.

---

## Collegamenti per ogni scheda

I pin sono stati scelti tra quelli liberi su ciascuna scheda, evitando i pin di strapping, l'USB e quelli già usati dalla scheda o dallo schema di base.

| Segnale | Heltec V4 | Wireless Stick Lite V3 | WiFi Kit 32 V3 + RFM95W |
|---|---|---|---|
| MPL3115A2 SDA / SCL (bus Wire, con il SCD41) | GPIO 7 / 6 | GPIO 7 / 6 | GPIO 7 / 6 |
| MPL3115A2 VIN | Vext | Vext | Vext |
| CCS811 SDA (Wire1) | GPIO 4 (J3.15) | GPIO 4 (JP3.20) | GPIO 4 (J2.15) |
| CCS811 SCL (Wire1) | GPIO 5 (J3.16) | GPIO 5 (JP3.19) | GPIO 2 (J2.17) |
| CCS811 VCC | 3V3 | 3V3 | 3V3 |
| SDS011 TX → ESP32 (SDS_RX) | GPIO 48 (J2.14) | GPIO 48 (JP3.5) | GPIO 3 (J2.16) |
| Interruttore SDS011 (SDS_EN) | GPIO 47 (J2.13) | GPIO 47 (JP3.4) | GPIO 26 (J3.4) |

Sulla Kit 32 V3 i GPIO 47 e 48 sono occupati dall'RFM95W, per questo l'SDS011 usa il 3 e il 26. Il GPIO 3 è un pin di strapping solo se è stato programmato un fusibile apposito, cosa che sulle Heltec non succede; in più il SDS011 è spento all'avvio, quindi non può disturbare. Il GPIO 26 è libero perché l'ESP32-S3FN8 della Kit non ha PSRAM (sulla V4, invece, il GPIO 26 è usato dalla PSRAM e non va toccato).

Gli schemi completi:

<p align="center">
  <img src="img/nodo_wiring_upgrade_v4.svg" alt="Upgrade multisensore sulla Heltec V4" width="1100">
</p>

<p align="center">
  <img src="img/nodo_wiring_upgrade_wsl.svg" alt="Upgrade multisensore sulla Wireless Stick Lite V3" width="1100">
</p>

<p align="center">
  <img src="img/nodo_wiring_upgrade_kit.svg" alt="Upgrade multisensore sulla WiFi Kit 32 V3" width="1100">
</p>

> ⚠️ **Heltec V4 e GPIO 7.** Il datasheet della V4.2 indica il GPIO 7 anche come **VFEM_Control**, il comando di alimentazione del front-end radio a 28 dBm. Il firmware di base lo usa come SDA del bus Wire, e funziona perché la linea, a riposo, è tenuta alta dai pull-up. Se noti trasmissioni che non partono o una portata ridotta, sposta il bus Wire su altri due pin liberi (per esempio GPIO 2 e 3) cambiando `PIN_I2C_SDA` e `PIN_I2C_SCL`.

---

## Il ciclo del firmware con i nuovi sensori

Il ciclo resta quello della dispensa di base. I nuovi sensori si inseriscono così:

1. **Boot e rilascio degli hold** dei GPIO, compreso SDS_EN.
2. **Alimentazione**: Vext (SCD41 e MPL3115A2), GPS e, **subito**, il SDS011. La sua ventilazione di 30 secondi avviene così **in parallelo** all'attesa del fix GPS e alle misure del SCD41.
3. Inizializzazione del SCD41 e attesa del fix GPS, come prima.
4. Lettura del SCD41.
5. **Nuovo**: lettura dell'**MPL3115A2** (una misura one-shot).
6. **Nuovo**: lettura del **CCS811**, dopo avergli passato temperatura e umidità misurate dal SCD41 per la compensazione.
7. **Nuovo**: lettura del **SDS011**. Se il GPS ha già impiegato più di 30 secondi non si aspetta nulla, altrimenti si attende la fine della ventilazione. Poi si mediano 5 pacchetti e **si spegne il SDS011**.
8. Lettura della batteria, che avviene con il SDS011 già spento, quindi senza il calo di tensione dovuto alla ventola.
9. Composizione del payload 0x42 come prima, poi **traduzione nel formato 0x44** con l'aggiunta dei nuovi sensori.
10. Trasmissione, finestre RX, deep sleep. Prima del sleep SDS_EN viene messo basso e bloccato con `gpio_hold_en()`.

Il passo 9 è una scelta precisa: tutta la logica già collaudata per batteria, SCD41 e GPS (GPS SKIP, posizione stale, sensori disattivati) continua a riempire la struttura `Payload_v0x42`. La funzione `buildPayload44()` la prende così com'è e la traduce nel formato nuovo, aggiungendo i blocchi dei sensori nuovi. Il codice vecchio non va riscritto, e il rischio di introdurre errori è minimo.

```cpp
uint8_t p44[PAYLOAD44_MAX_LEN];
size_t  p44len = buildPayload44(p44, payload, doScdThisCycle, scdOk,
                                cfgGpsEnabled, timeEpoch, xr);
sendPayload(p44, p44len);
```

`xr` è una struttura `ExtraReadings` che raccoglie le letture dei sensori nuovi, con un flag "presente" e un flag "lettura riuscita" per ciascuno.

In **modalità identify** il nodo resta sveglio e trasmette spesso: il payload contiene solo batteria e SCD41, senza GPS né SDS011.

---

## I driver: come si parla con i sensori

Per i tre sensori nuovi il firmware **non usa librerie esterne**: bastano poche funzioni scritte direttamente sui registri. Il motivo principale riguarda il CCS811: le librerie più diffuse (Adafruit, SparkFun) nella funzione `begin()` fanno un **reset software** del sensore. A ogni risveglio dal deep sleep il CCS811 ripartirebbe da capo, e i 20 minuti di condizionamento non finirebbero mai. Per gli altri due sensori scrivere il driver costa poche righe, ed è utile vedere come si parla davvero con un dispositivo I²C o seriale.

### Lettura I²C con repeated start

L'MPL3115A2 richiede che la lettura di un registro avvenga con un **repeated start**: si scrive l'indirizzo del registro e si legge senza rilasciare il bus in mezzo. In Arduino si ottiene con `endTransmission(false)`:

```cpp
bool i2cReadReg(TwoWire& bus, uint8_t addr, uint8_t reg, uint8_t* data, size_t n) {
    bus.beginTransmission(addr);
    bus.write(reg);
    if (bus.endTransmission(false) != 0) return false;   // repeated start
    if (bus.requestFrom((uint16_t)addr, (uint8_t)n) != n) return false;
    for (size_t i = 0; i < n; i++) data[i] = bus.read();
    return true;
}
```

La stessa funzione serve per entrambi i bus: le si passa `Wire` o `Wire1`.

### MPL3115A2: una misura one-shot

| Registro | Indirizzo | Uso |
|---|---|---|
| WHO_AM_I | 0x0C | vale 0xC4: verifica che il sensore risponda |
| PT_DATA_CFG | 0x13 | 0x07: abilita i flag "dato pronto" |
| CTRL_REG1 | 0x26 | 0x38 = barometro, oversampling 128, standby; 0x3A = idem + avvia una misura (OST) |
| STATUS | 0x00 | bit 3 (PTDR) = pressione e temperatura pronte |
| OUT_P_MSB… | 0x01–0x05 | 3 byte di pressione + 2 di temperatura |

La pressione è un numero a 20 bit in formato Q18.2 (18 bit interi e 2 frazionari, in pascal), allineato a sinistra nei 3 byte. Letti i 3 byte come un intero a 24 bit, si divide per 64 per avere i pascal, o per 6400 per gli hPa:

```cpp
uint32_t raw = ((uint32_t)d[0] << 16) | ((uint32_t)d[1] << 8) | d[2];
pressHpa = raw / 6400.0f;                          // raw/64 Pa
tempC    = (int16_t)((d[3] << 8) | d[4]) / 256.0f; // Q8.8 in gradi
```

Con oversampling 128 la misura dura circa mezzo secondo: il firmware interroga STATUS ogni 20 ms, con un timeout di 1,5 s.

### CCS811: avviarlo solo se serve

Il CCS811 parte nel suo **bootloader** e va portato nella modalità applicazione con il comando `APP_START` (0xF4). Il firmware lo fa **solo se necessario**, leggendo il registro STATUS:

```cpp
i2cReadReg(Wire1, CCS811_ADDR, CCS_REG_STATUS, &st, 1);
if (!(st & 0x80)) {             // bit 7 FW_MODE = 0: ancora nel bootloader
    // APP_START, poi annota l'inizio del condizionamento
}
i2cReadReg(Wire1, CCS811_ADDR, CCS_REG_MEAS_MODE, &mm, 1);
if (((mm >> 4) & 0x07) != CCS811_DRIVE_MODE) {
    // imposta il modo 3, e anche questo riavvia il condizionamento
}
```

Al primo avvio dopo l'accensione della batteria il sensore è nel bootloader e viene avviato. Ai risvegli successivi è già in modalità applicazione e già nel modo 3: il firmware **non lo tocca**, legge soltanto.

L'istante di avvio viene salvato in RTC memory (`ccsStartUptimeS`). Per i primi 20 minuti il payload porta il flag **CCS811 in riscaldamento** e il codec lo espone come `ccs_warmup: true`: la dashboard può mostrare i valori come indicativi.

Prima di ogni lettura il firmware scrive nel registro ENV_DATA la **temperatura e l'umidità misurate dal SCD41**: l'algoritmo del CCS811 le usa per compensare la misura. Il formato è in 1/512 di grado e di %RH, con la temperatura spostata di 25 °C:

```cpp
uint16_t h = humRH * 512 + 0.5;
uint16_t t = (tempC + 25) * 512 + 0.5;
```

Il risultato si legge dal registro ALG_RESULT_DATA (0x02): eCO₂ (2 byte), TVOC (2 byte), STATUS e ERROR_ID, in big-endian. Nel primo minuto dopo l'avvio il sensore restituisce 0: il firmware lo considera "nessuna misura".

Se il CCS811 viene disattivato via downlink, il firmware lo mette in **idle** (MEAS_MODE = 0), perché non può spegnerlo. Quando lo si riattiva riparte il condizionamento.

### SDS011: leggere i pacchetti seriali

Il SDS011 manda un pacchetto di 10 byte al secondo:

```
AA C0 PM25_L PM25_H PM10_L PM10_H ID_L ID_H CHECKSUM AB
```

- `AA C0` è l'intestazione, `AB` la coda;
- PM2.5 e PM10 sono interi a 16 bit little-endian, in **decimi** di µg/m³;
- il checksum è la somma dei 6 byte di dati, modulo 256.

Il driver è una piccola macchina a stati che scarta i byte finché non trova `AA C0`, raccoglie 10 byte, controlla coda e checksum, e accumula i valori. Ne media 5, con un timeout di 10 s. Usa la **UART2** dell'ESP32-S3, in sola ricezione:

```cpp
SdsSerial.begin(9600, SERIAL_8N1, PIN_SDS_RX, -1);   // -1 = nessun pin TX
```

---

## Il payload 0x44 a blocchi

Lo schema 0x42 aveva una lunghezza fissa di 32 byte. Con cinque sensori, una struttura fissa conterrebbe sempre anche i campi dei sensori assenti: un nodo con il solo SCD41 trasmetterebbe una trentina di byte inutili a ogni ciclo. Lo schema 0x44 è invece **a blocchi**: un'intestazione dice quali sensori sono presenti, e seguono solo i loro dati.

### Intestazione (10 byte)

| Offset | Byte | Tipo | Campo | Note |
|---|---|---|---|---|
| 0 | 1 | u8 | schema_id | 0x44 |
| 1 | 1 | u8 | sensor_mask | bit 0 SCD41, bit 1 GPS, bit 2 SDS011, bit 3 CCS811, bit 4 MPL3115A2 |
| 2 | 1 | u8 | flags | bit 0 timestamp dal GPS, bit 1 posizione non aggiornata, bit 2 CCS811 in riscaldamento |
| 3 | 1 | u8 | battery_pct | % |
| 4 | 2 | u16 | vbat_mv | mV |
| 6 | 4 | u32 | timestamp | epoch UTC dal GPS, oppure secondi dal boot |

Il timestamp passa da 8 a 4 byte: un `uint32` di secondi basta fino al 2106. Il flag del bit 0 dice se è un'ora vera (dal GPS) o un contatore.

### Blocchi, nell'ordine dei bit

| Bit | Sensore | Byte | Campi |
|---|---|---|---|
| 0 | SCD41 | 6 | co2_ppm u16, temp_c100 i16, hum_pct100 u16 |
| 1 | GPS | 14 | fix_quality u8, satellites u8, lat_e7 i32, lon_e7 i32, alt_m i16, hdop_x100 u16 |
| 2 | SDS011 | 4 | pm25_x10 u16, pm10_x10 u16 |
| 3 | CCS811 | 5 | eco2_ppm u16, tvoc_ppb u16, error_id u8 |
| 4 | MPL3115A2 | 4 | press_hpa10 u16 (hPa ×10), temp_c100 i16 |

La lunghezza va da **10 byte** (solo intestazione) a **43 byte** (tutti i sensori). 43 byte entrano anche nel data rate più lento dell'EU868 (DR0, SF12), il cui limite è di 51 byte.

### Sensore assente e lettura fallita

Sono due situazioni diverse, e il payload le distingue:

- **sensore assente** (escluso in compilazione o spento via downlink): il suo bit nella maschera è 0 e **il blocco non c'è**;
- **sensore presente ma lettura fallita**: il bit è 1, il blocco c'è, ma contiene i valori sentinella **0xFFFF** (campi senza segno) o **0x7FFF** (campi con segno). Il codec li trasforma in `null`.

Per il GPS valgono le stesse convenzioni dello schema 0x42: `hdop_x100 = 9999` indica una posizione vecchia (GPS SKIP o fix fallito), `0xFFFF` indica che non c'è nessuna posizione nota.

### Un esempio

Con tutti i sensori presenti, il firmware produce 43 byte:

```
44 1F 05 58 AC0F 0078E768        intestazione: maschera 0x1F, flag 0x05,
                                 88 %, 4012 mV, timestamp 1760000000 (epoch)
6402 6A08 0613                   SCD41: 612 ppm, 21,54 °C, 48,70 %
01 07 00EE5B16 4D88FE08 2A00 6E00   GPS: fix, 7 satelliti, 37,5123456 / 15,0898765, 42 m, HDOP 1,10
7B00 F500                        SDS011: PM2.5 12,3 · PM10 24,5 µg/m³
3403 4000 00                     CCS811: eCO2 820 ppm · TVOC 64 ppb · nessun errore
9427 A208                        MPL3115A2: 1013,2 hPa · 22,10 °C
```

Il flag 0x05 è la somma di 0x01 (timestamp dal GPS) e 0x04 (CCS811 ancora nei primi 20 minuti).

---

## Payload state 0x43 esteso

Il payload state (FPort 2) cresce da 26 a **31 byte**. I primi 26 sono identici a prima, quindi una webapp vecchia continua a leggerli. In coda ci sono:

| Offset | Campo | Note |
|---|---|---|
| 26 | cfgSdsEnabled | 0/1 |
| 27 | cfgCcsEnabled | 0/1 |
| 28 | cfgMplEnabled | 0/1 |
| 29 | cfgSdsWarmupS | secondi di ventilazione del SDS011 |
| 30 | sensorsCompiled | maschera dei sensori compilati, stessi bit della sensor_mask |

Il campo `sensorsCompiled` serve alla webapp per sapere quali interruttori mostrare: inutile proporre di accendere il SDS011 su un nodo compilato senza.

---

## Nuovi comandi downlink

Si aggiungono quattro comandi di configurazione su **FPort 20**, salvati in NVS come gli altri:

| Comando | Byte | Argomento | Base64 di esempio |
|---|---|---|---|
| `SET_SDS_ENABLED` | 0x1A | 0 / 1 | `GgA=` spegne, `GgE=` accende |
| `SET_CCS_ENABLED` | 0x1B | 0 / 1 | `GwA=` spegne (idle), `GwE=` accende |
| `SET_MPL_ENABLED` | 0x1C | 0 / 1 | `HAA=` spegne, `HAE=` accende |
| `SET_SDS_WARMUP` | 0x1D | secondi 5–120 | `HR4=` 30 s, `HTw=` 60 s |

Esempio con mosquitto: spegnere il SDS011 di un nodo installato in un luogo dove le polveri non interessano, per risparmiare batteria.

```bash
mosquitto_pub -h <IP-broker> -p 1883 \
  -t "application/<app-uuid>/device/<devEUI>/command/down" \
  -m '{"devEui":"<devEUI>","fPort":20,"confirmed":false,"data":"GgA="}'
```

Come per gli altri comandi di configurazione, dopo l'applicazione il nodo invia un payload state al ciclo successivo, come conferma. Un comando che accende un sensore **non compilato** viene salvato ma non ha effetto: al boot successivo il firmware lo forza di nuovo a spento.

---

## Il codec ChirpStack

Il codec [`codecs/chirpstack_codec.js`](codecs/chirpstack_codec.js) è stato esteso, non sostituito:

- gli schemi **0x41, 0x42 e 0x43** si decodificano come prima;
- lo schema **0x43** da 31 byte espone in più `cfg_sds_enabled`, `cfg_ccs_enabled`, `cfg_mpl_enabled`, `cfg_sds_warmup_s` e `sensors_compiled`. Con 26 byte quei campi valgono `null`;
- lo schema **0x44** viene letto blocco per blocco seguendo la maschera. Prima di tutto il codec controlla che la lunghezza ricevuta corrisponda alla maschera, e in caso contrario restituisce un errore leggibile invece di valori sbagliati;
- il downlink accetta ora i comandi fino a **0x1D** su FPort 20.

È stato corretto anche un piccolo difetto della funzione `readU32LE`: in JavaScript gli operatori sui bit lavorano con interi **con segno**, e un valore oltre 2³¹ (per esempio un uptime molto lungo) sarebbe diventato negativo. Ora il risultato passa per `>>> 0`, che lo rende senza segno.

L'oggetto decodificato per il payload d'esempio è:

```json
{
  "schema_id": 68, "payload_type": "measure",
  "sensors": { "scd41": true, "gps": true, "sds011": true, "ccs811": true, "mpl3115a2": true },
  "battery_pct": 88, "vbat_mv": 4012,
  "timestamp": 1760000000, "timestamp_is_epoch": true,
  "co2_ppm": 612, "temp_c": 21.54, "hum_pct": 48.7,
  "fix_quality": 1, "satellites": 7, "alt_m": 42, "hdop": 1.1, "gps_stale": false,
  "pm25": 12.3, "pm10": 24.5,
  "eco2_ppm": 820, "tvoc_ppb": 64, "ccs_error": 0, "ccs_warmup": true,
  "pressure_hpa": 1013.2, "mpl_temp_c": 22.1,
  "location": { "latitude": 37.5123456, "longitude": 15.0898765, "altitude": 42,
                "source": "GPS", "accuracy": 6 },
  "scd_disabled": false, "gps_disabled": false
}
```

I campi che c'erano già nello schema 0x42 hanno lo **stesso nome** (`co2_ppm`, `temp_c`, `hum_pct`, `battery_pct`, `vbat_mv`, `location`, `scd_disabled`, `gps_disabled`, …): le dashboard esistenti continuano a funzionare. Per vedere PM, TVOC e pressione bisogna aggiungere i relativi widget. Anche la webapp di configurazione va estesa con i quattro comandi nuovi, per esempio aggiungendo a `mqtt_downlink_encoder.js` funzioni che restituiscano i byte `[0x1A, v]`, `[0x1B, v]`, `[0x1C, v]` e `[0x1D, s]`.

---

## Consumi e autonomia

I nuovi sensori cambiano molto il bilancio energetico, ed è giusto saperlo prima di installare il nodo.

| Sensore | Quando consuma | Quanto | Effetto |
|---|---|---|---|
| SDS011 | ~35 s a ogni ciclo (ventilazione + lettura) | ~110–130 mA dalla batteria (5 V × 70 mA attraverso il boost) | **circa 1,2 mAh per ciclo** |
| CCS811 | sempre, anche in deep sleep | circa 0,7 mA di media nel modo 3 (1,2 mW secondo il datasheet) | **circa 17 mAh al giorno** |
| MPL3115A2 | ~0,5 s a ogni ciclo, poi spento con Vext | pochi mA per mezzo secondo | trascurabile |

Il SDS011 domina. Con una batteria da 1500 mAh utili e solo il consumo del SDS011:

| Intervallo TX | Cicli al giorno | SDS011 al giorno | Autonomia (solo SDS011) |
|---|---|---|---|
| 1 min | 1440 | ~1700 mAh | meno di un giorno |
| 10 min | 144 | ~170 mAh | ~9 giorni |
| 30 min | 48 | ~58 mAh | ~3–4 settimane |
| 1 h | 24 | ~29 mAh | ~7 settimane |

In pratica:

- **con il SDS011 acceso** usa un intervallo di almeno 10–30 minuti, oppure un **pannello solare** dimensionato di conseguenza;
- l'intervallo di 1 minuto della configurazione di fabbrica va bene solo per le prove al banco, con l'USB collegato;
- il SDS011 si può spegnere da remoto con `SET_SDS_ENABLED = 0` (base64 `GgA=`) quando le polveri non servono;
- i 17 mAh al giorno del CCS811 da soli scaricherebbero 1500 mAh in circa tre mesi: se il TVOC non serve, è meglio non montarlo o compilare con `ENABLE_CCS811 = 0`. Con `SET_CCS_ENABLED = 0` va in idle, dove consuma quasi nulla, ma riaccendendolo riparte il condizionamento.

Il ventilatore e il laser del SDS011 hanno una vita di circa 8000 ore di funzionamento. Con 35 secondi ogni 10 minuti lavorano circa il 6 % del tempo: la vita utile si allunga di oltre dieci volte rispetto all'uso continuo.

---

## Limiti delle misure

**SDS011.** Con umidità relativa sopra il 70–80 % le goccioline d'acqua vengono contate come particelle e i valori di PM salgono molto. È un limite di tutti i sensori ottici a basso costo. L'umidità misurata dal SCD41 permette di riconoscere questi casi in dashboard, per esempio segnalando come poco affidabili i valori di PM con RH > 80 %.

**CCS811.** Non misura la CO₂: la **eCO₂** è una stima ricavata dai composti organici volatili, che ha senso solo in ambienti chiusi abitati. Con il SCD41 sul nodo, la CO₂ vera è quella del SCD41, e il dato utile del CCS811 è il **TVOC**. Altri punti:
- secondo il datasheet le letture si possono usare dopo **60 minuti** di funzionamento dalla prima accensione, e il sensore si stabilizza nelle prime **48 ore**;
- la correzione automatica della linea di base lavora su tempi di almeno 24 ore. Il registro BASELINE (0x11) permetterebbe di salvarla e ripristinarla dopo un'interruzione di alimentazione: è un'estensione possibile, non implementata in questa versione.

**MPL3115A2.** Misura la pressione **alla quota del sensore**, non quella riportata al livello del mare dei bollettini meteo. Per confrontarla con quella dei bollettini si converte in dashboard, con la quota del GPS o una quota fissa nota:

```
p_mare = p · (1 − h / 44330)^(−5,255)
```

dove h è la quota in metri. A 100 m di quota la differenza è di circa 12 hPa. La sua temperatura (`mpl_temp_c`) è utile come controllo, ma risente del calore della scheda: per la temperatura dell'aria vale quella del SCD41.

---

## Verifiche al banco

Prima di chiudere il nodo nel contenitore:

1. **Scansione I²C.** Con Vext acceso, sul bus Wire (GPIO 7/6) devono rispondere **0x62** (SCD41) e **0x60** (MPL3115A2); sul bus Wire1 **0x5A** (CCS811). Uno sketch di scansione va eseguito sui due bus separatamente.
2. **Log del primo ciclo.** Con il Monitor Seriale aperto devono comparire, in ordine:
   ```
   SDS011: alimentato, ventilazione in corso
   ...
   MPL3115A2: P=1012.84 hPa  T=23.40 C
   CCS811: avviato (inizio condizionamento di 20 minuti)
   SDS011: PM2.5=8.4  PM10=11.2 ug/m3 (5 pacchetti)
   TX 43 byte: 441F05...
   ```
   Al primo ciclo il CCS811 può non dare ancora una misura: nel primo minuto dopo l'avvio restituisce 0.
3. **SDS011 spento in sleep.** Durante il deep sleep la ventola deve essere ferma. Se gira, l'interruttore lato alto non si spegne: controlla la resistenza da 100 kΩ tra gate e source del P-MOSFET.
4. **Consumo in deep sleep.** Con un multimetro in serie alla batteria (portata µA o mA): al consumo della dispensa di base si aggiunge quasi solo il CCS811, quindi qualche centinaio di µA di media, con picchi ogni 60 secondi. Valori di decine di mA indicano il SDS011 o il boost ancora alimentati.
5. **Payload su ChirpStack.** Nella scheda *Events* del device, l'oggetto decodificato deve mostrare `sensors` con i sensori montati a `true` e nessun errore del codec. Un errore "la maschera richiede N byte" indica un firmware e un codec non allineati.

---

## Fonti

- [Datasheet CCS811 (ams/ScioSense)](https://www.mouser.com/datasheet/2/1081/CCS811_Datasheet-1889753.pdf): modi di misura, consumi, condizionamento, registri
- [Adafruit CCS811 library](https://github.com/adafruit/Adafruit_CCS811): mappa dei registri e formato di ENV_DATA usati come riferimento
- [Adafruit MPL3115A2 library](https://github.com/adafruit/Adafruit_MPL3115A2_Library): registri e conversione della pressione usati come riferimento
- [Datasheet Heltec WiFi LoRa 32 V4.2](https://resource.heltec.cn/download/WiFi_LoRa_32_V4/datasheet/WiFi_LoRa_32_V4.2.0.pdf): pin dei pettini, GPIO 7 VFEM_Control, GPIO usati dalla scheda
- Protocollo seriale del SDS011: già descritto nel [README](README.md#sensore-pesante-sds011) del progetto di alimentazione
