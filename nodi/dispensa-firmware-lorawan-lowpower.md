# Dispensa: firmware LoRaWAN low-power per stazione ambientale

Analisi del firmware `heltec_v4_scd41_gps_lorawan.ino`, un end-device LoRaWAN autonomo che campiona CO₂, temperatura, umidità e posizione GPS, e li trasmette via LoRaWAN in classe A con deep sleep.

## Indice

1. [Contesto e obiettivi](#contesto-e-obiettivi)
2. [Architettura hardware](#architettura-hardware)
3. [Il ciclo di vita del firmware](#il-ciclo-di-vita-del-firmware)
4. [Schema del payload versionato](#schema-del-payload-versionato)
5. [FPort: il discriminatore di tipo LoRaWAN](#fport-il-discriminatore-di-tipo-lorawan)
6. [Gestione dell'alimentazione delle periferiche](#gestione-dellalimentazione-delle-periferiche)
7. [Sensore SCD41: I²C e misure periodiche](#sensore-scd41-ic-e-misure-periodiche)
8. [GPS L76K: parsing NMEA e timeout adattivo](#gps-l76k-parsing-nmea-e-timeout-adattivo)
9. [LoRaWAN con RadioLib 7.x](#lorawan-con-radiolib-7x)
10. [Persistenza del frame counter](#persistenza-del-frame-counter)
11. [Deep sleep e GPIO hold](#deep-sleep-e-gpio-hold)
12. [Protezione batteria da under-discharge](#protezione-batteria-da-under-discharge)
13. [Watchdog hardware](#watchdog-hardware)
14. [Downlink handler e comandi remoti](#downlink-handler-e-comandi-remoti)
15. [Modalità debug e produzione](#modalità-debug-e-produzione)
16. [Diagnostica e osservabilità](#diagnostica-e-osservabilità)

---

## Contesto e obiettivi

Il device è pensato per campionare la qualità dell'aria (CO₂, temperatura, umidità) e la posizione, e trasmettere periodicamente questi dati via **LoRaWAN** verso una rete condivisa. I vincoli principali di progetto sono tre:

**Autonomia energetica**. Il device deve poter funzionare per mesi o anni a batteria (opzionalmente con pannello solare). Questo comporta l'uso di deep sleep aggressivo e lo spegnimento fisico delle periferiche tra un ciclo e l'altro.

**Robustezza in caso di power-loss**. Non si può fare affidamento sulla continuità dell'alimentazione: la batteria può scaricarsi, il pannello solare può essere ombreggiato, il device può essere spostato e spento. Il firmware deve riprendersi da queste situazioni senza intervento umano e senza rompere le sessioni LoRaWAN attive.

**Semplicità di manutenzione**. Il codice deve essere leggibile, con separazione chiara delle responsabilità e con flag di configurazione ben esposti in cima al file. Sviluppo e debug devono essere possibili senza tool esoterici, con solo Arduino IDE e Serial Monitor.

Non è un progetto commerciale: la finalità è didattica. Molte scelte sono commentate esplicitamente nel codice per rendere trasparenti i motivi delle decisioni.

---

## Architettura hardware

Il device è basato sulla **Heltec WiFi LoRa 32 V4**, che integra:

<p align="center">
  <img src="img/heltec_pinout.png" alt="Heltec WiFi LoRa 32 pinout" width="600">
</p>

*Figura 1: pinout della Heltec WiFi LoRa 32. L'immagine mostra la V3, ma la V4 R8 usa la stessa piedinatura sugli header J2/J3 con l'aggiunta del connettore SH1.25-8P per il modulo di espansione (GPS/batteria).*

- **ESP32-S3R2**: microcontrollore dual-core Xtensa LX7 a 32 bit con PSRAM da 2 MB, USB-CDC integrato (senza chip UART bridge esterno), WiFi e Bluetooth
- **SX1262**: radio LoRa Semtech per la banda EU868 (~868 MHz), con PA integrato fino a +22 dBm (limitato per legge a +14 dBm in EU)

Alla scheda base si aggiungono, via header:

- **Sensirion SCD41**: sensore CO₂ NDIR fotoacustico su bus I²C, con temperatura e umidità
- **Quectel L76K**: ricevitore GNSS multi-costellazione (GPS + BeiDou) su UART, con controllo di alimentazione
- Batteria LiPo 1S opzionale, con partitore su ADC per il monitoraggio della tensione

Sulla V4 esistono due pin di controllo dell'alimentazione periferica, entrambi comandati dal microcontrollore:

- **Vext_Ctrl (GPIO 36)** — pilota un MOSFET P-channel che alimenta la linea *Vext* (usata per SCD41). Attivo-basso: LOW = MOSFET conduce = periferiche accese.
- **VGNSS_Ctrl (GPIO 34)** — pilota l'alimentazione del solo modulo GPS. Nella particolare release del PCB usata qui la logica è invertita rispetto alla documentazione base: LOW = GPS acceso.

Questo pattern MOSFET P active-low è tipico di tutte le schede low-power: durante il deep sleep, il micro non ha bisogno di "spegnere attivamente" niente perché il pin che comanda il MOSFET si trova già nello stato che spegne il gate (attivo alto quando spento). L'unico accorgimento è mantenere questo livello anche durante il sleep — vedi la sezione sui GPIO hold.

---

## Il ciclo di vita del firmware

A differenza di un firmware Arduino tradizionale con `setup()` una volta e `loop()` continuo, questo firmware è strutturato come **ciclo singolo**: dopo ogni trasmissione entra in deep sleep. Al wake, la CPU riparte da capo con un nuovo `setup()`. Il `loop()` è vuoto perché non viene mai eseguito — il flusso finisce sempre in `esp_deep_sleep_start()` prima.

Un ciclo tipico dura ~30-60 secondi di attività seguiti da ~1 minuto (o più) di sleep, per un duty cycle attivo del 30-50%. Le fasi in ordine:

1. **Boot e rilascio hold GPIO**. La CPU si sveglia, rilascia i lock sui GPIO che erano stati "congelati" prima del sleep precedente.
2. **Alimentazione periferiche**. `vextOn()` accende il SCD41, `gpsPowerOn()` accende il GPS.
3. **Inizializzazione I²C e sensore SCD41**. Reset del sensore, verifica del serial number, avvio delle misure periodiche.
4. **Attesa fix GPS in parallelo alle misure SCD41**. Il GPS può richiedere 30-90 secondi al primo fix; nel frattempo il SCD41 sta già misurando (una misura ogni 5 secondi).
5. **Lettura di tutti i dati**: valori SCD41, coordinate GPS, tensione batteria.
6. **Composizione payload** in una struct C impacchettata (schema versionato).
7. **Inizializzazione radio LoRa e attivazione LoRaWAN ABP**. Ripristino del session buffer da NVS o RTC memory.
8. **Trasmissione via `sendReceive()` su FPort 1**. Il device parla, poi apre due finestre RX (obbligatorie in classe A) per ricevere eventuali downlink.
9. **Aggiornamento persistenza**. FCnt salvato in RTC memory sempre, in NVS ogni N cicli.
10. **Deep sleep**. Spegnimento periferiche, hold dei GPIO, timer di wake.

Ogni fase è confinata in una funzione dedicata. Il `setup()` è essenzialmente una sequenza di chiamate al livello superiore, senza logica di controllo complessa.

---

## Schema del payload versionato

Il payload LoRaWAN è progettato per essere **compatto**, **fisso**, e **riconoscibile**. Compatto perché ogni byte in più aumenta l'airtime e quindi il consumo energetico e il duty cycle usato. Fisso perché il codec sul Network Server deve poter decodificare senza dover interpretare struttura variabile. Riconoscibile perché in futuro potremmo voler aggiungere altri sensori (nuovi schemi) allo stesso Network Server, e serve un modo per distinguerli.

Tutti gli uplink applicativi partono su **FPort 1** (definito dal `#define LORAWAN_FPORT`). All'interno del payload, un **primo byte "schema_id"** identifica univocamente la struttura seguente:

```c
#define SCHEMA_ID  0x42     // SCD41 + L76K + batteria

#pragma pack(push, 1)
struct Payload_v0x42 {
    uint8_t  schema_id;      // 0x42
    uint8_t  fix_quality;    // 0=no, 1=GPS, 2=DGPS
    uint8_t  satellites;
    uint8_t  battery_pct;
    uint64_t timestamp;
    uint16_t co2_ppm;
    int16_t  temp_c100;      // T x 100 (centesimi °C)
    uint16_t hum_pct100;     // RH x 100
    uint16_t vbat_mv;
    int32_t  lat_e7;         // latitudine x 1e7
    int32_t  lon_e7;
    int16_t  alt_m;
    uint16_t hdop_x100;
};
#pragma pack(pop)

static_assert(sizeof(Payload_v0x42) == 32, "Payload deve essere 32 byte");
```

Alcuni dettagli notevoli:

**Interi al posto di float**. Temperatura e umidità sono moltiplicate per 100 e trasmesse come interi. Latitudine e longitudine sono moltiplicate per 10⁷. Questo risparmia byte (`int16_t` invece di `float32`) senza perdere precisione utile.

**`#pragma pack(push, 1)`**. Senza questa direttiva il compilatore inserisce padding tra i campi per allineare le word alle boundary del processore, portando la struct a più di 32 byte. Con `pack(1)` i campi sono impacchettati senza spazi.

**`static_assert(sizeof(...) == 32, ...)`**. Se il compilatore per qualsiasi motivo produce una struct di dimensione diversa (per esempio se un futuro sviluppatore modifica lo schema senza aggiornare l'`assert`), il build fallisce subito, prima ancora di scaricare il firmware sul device. È una salvaguardia difensiva a costo zero.

**Big picture: pacchetti versionati**. Il pattern "primo byte come discriminatore di schema" è usato anche da protocolli come CBOR, MessagePack e altri codec versionati. Consente di far coesistere versioni multiple senza ambiguità e senza dover cambiare il formato di trasporto.

**Schema id vs FPort — due discriminatori sovrapposti**. Nel nostro progetto abbiamo **due meccanismi** di discriminazione: il FPort a livello LoRaWAN, e il schema_id come primo byte del payload applicativo. Sembra ridondante, ma i due discriminatori operano a **livelli diversi**: il FPort è visibile al Network Server per instradare i pacchetti al codec giusto, mentre il schema_id è interno al codec e distingue formati concreti che condividono lo stesso "canale" LoRaWAN. La prossima sezione entra nel dettaglio del ruolo di FPort.

---

## FPort: il discriminatore di tipo LoRaWAN

Ogni pacchetto LoRaWAN che viaggia tra device e gateway contiene, oltre alle chiavi crittografiche e header MAC, due campi applicativi fondamentali:

```
+--------+---------------------------+
| FPort  | FRMPayload (0-242 byte)   |
+--------+---------------------------+
```

- **FPort**: 1 byte, valore 0-255
- **FRMPayload**: il payload applicativo vero e proprio

Il **FPort** è un campo del protocollo LoRaWAN definito dalla LoRa Alliance. **Non è una feature di ChirpStack**: esiste da sempre nella specifica, e tutti i Network Server LoRaWAN (ChirpStack, TTN, Loriot, Actility) lo espongono nello stesso modo.

### A cosa serve

Concettualmente, FPort è l'equivalente di:

- Il **numero di porta** in TCP/UDP (che discrimina i servizi: 80=HTTP, 443=HTTPS, 22=SSH)
- Il campo **type** in un pacchetto Ethernet (0x0800=IPv4, 0x86DD=IPv6)
- L'**endpoint URL** in un'API REST (`/api/temperature` vs `/api/config`)

Serve a **discriminare tra tipi di payload** senza dover ispezionare il contenuto. Un Network Server che vede arrivare un pacchetto con FPort=3 sa che è (per esempio) un heartbeat, senza doverne decodificare i byte per capirlo.

### I range riservati dalla specifica

La specifica LoRaWAN riserva alcuni range di valori con significati particolari:

| FPort | Uso |
|-------|-----|
| **0** | Riservato per MAC commands puri (comandi di gestione del layer LoRaWAN, non applicativi) |
| **1 - 223** | **Applicazione**: liberi per l'utente |
| **224** | Riservato per test conformità LoRaWAN |
| **225 - 255** | Riservati per future estensioni della specifica |

Abbiamo quindi **223 valori disponibili** per organizzare i nostri messaggi.

### Le convenzioni del nostro progetto

Nel nostro firmware abbiamo scelto tre FPort:

| FPort | Direzione | Uso | Formato interno |
|-------|-----------|-----|-----------------|
| **1** | Uplink | Misure ambientali | Struct binaria versionata (byte 0 = `schema_id`) |
| **10** | Downlink | Comandi ordinari (azione) | Byte 0 = `command_id`, resto = argomenti |
| **20** | Downlink | Configurazione persistente | Byte 0 = `config_id`, resto = argomenti |

Sono numeri **arbitrari** ma scelti seguendo una piccola convenzione visiva:

- Uplink su FPort bassi (1, 2, 3, ...)
- Downlink su FPort a **decine tonde** (10, 20, 30, ...) per essere riconoscibili a colpo d'occhio nei log del Network Server

Se un giorno aggiungiamo un nuovo tipo di uplink (per esempio un heartbeat separato dalle misure complete), useremo FPort 2. Se aggiungiamo un downlink per aggiornamento OTA, useremo FPort 30. Il pattern si scala facilmente.

### Il flusso end-to-end del FPort in un downlink

Il ciclo di vita del FPort per un downlink attraversa più componenti:

**1. Origine — chi lo decide**

Quando l'utente accoda un downlink, deve specificare il FPort. Ci sono tre modi:

- Via **API REST** di ChirpStack, con il campo `fPort`
- Via **MQTT**, pubblicando su `application/<id>/device/<eui>/command/down` con `{"fPort": N, "object": {...}}`
- Via **ChirpStack UI**: tab "Queue" del device, campo "FPort"

**2. Codec — può sovrascriverlo (ma noi non lo facciamo)**

Il codec `encodeDownlink` in ChirpStack **può** ritornare un `fPort` nel valore di ritorno, sovrascrivendo quello ricevuto. Sarebbe possibile inferire il FPort dal contenuto del comando (es. `reboot` → FPort 10 automaticamente).

Nel nostro codec abbiamo scelto di **non farlo**: il client MQTT deve specificare il FPort corretto per la categoria del comando, e il codec si limita a validare la coerenza. Il motivo di questa scelta:

- **Il messaggio MQTT è auto-descrittivo**: guardando il `fPort` nel JSON capisci subito la categoria del comando
- **Se un giorno vuoi bypassare il codec** (per esempio inviando `data` binaria pre-encoded invece di `object`), il FPort giusto è già lì
- **Il debug è più semplice**: se il messaggio arriva sul FPort sbagliato, sai dove guardare

**3. Trasmissione radio**

ChirpStack accoda il messaggio con il FPort finale (quello scelto dal codec, se presente). Il gateway lo trasmette via LoRa nel pacchetto radio, dove FPort è un campo del PHY payload cifrato con la AppSKey.

**4. Device — legge il FPort**

Sul device, RadioLib decifra il pacchetto ed espone il FPort come parametro separato dal payload:

```cpp
uint8_t buf[64];
size_t  len = sizeof(buf);
uint8_t port = 0;
node.getDownlinkData(buf, &len, &port);
// port = 20, buf = [0x11, 0x03]
```

Il firmware fa il dispatch in base a `port`, esattamente come facciamo con `handleDownlinkPort10()` e `handleDownlinkPort20()`.

### Perché non usare un solo FPort per tutto

Si potrebbe pensare: perché non usare un solo FPort (es. 1) e distinguere tutto tramite il primo byte del payload, come fa il `schema_id`? Funzionerebbe, ma perdiamo alcuni vantaggi:

- **Filtraggio lato Network Server**: puoi configurare regole di routing che agiscono in base al FPort (per esempio: uplink su FPort 1 vanno al dashboard, uplink su FPort 2 vanno al monitoring)
- **Diagnostica veloce nei log**: un frame LoRaWAN visualizzato in ChirpStack mostra il FPort a colpo d'occhio, senza dover decodificare il payload
- **Chiarezza semantica**: nel codec, `input.fPort === 1` è più immediato di "guarda il primo byte se è 0x41 o 0x42"
- **Compatibilità con integrations esistenti**: molti tool assumono che un tipo di messaggio corrisponda a un FPort

Il costo è nullo: il FPort occupa 1 byte già presente nel protocollo, non toglie spazio al payload utile.

---

## Gestione dell'alimentazione delle periferiche

Il consumo del device è dominato non dal microcontrollore (che sta in deep sleep la maggior parte del tempo) ma dalle periferiche:

- **SCD41 in idle low-power**: ~0.5 mA
- **SCD41 durante misura**: picchi ~200 mA (fotoacustico!)
- **L76K con fix attivo**: ~25 mA continui
- **L76K in acquisizione (cold start)**: ~35 mA per 60-90 secondi
- **SX1262 in TX a 14 dBm**: ~130 mA per 100-300 ms

Lasciare queste periferiche sempre accese vuol dire consumare come minimo 30 mA sempre, che significa una batteria LiPo da 2000 mAh scarica in ~66 ore. Con lo spegnimento aggressivo durante il sleep, il consumo medio scende di 10-50 volte.

La logica di controllo è a tre livelli:

1. **Vext_Ctrl** attiva la linea Vext che alimenta il SCD41. Prima del sleep, `vextOff()` porta il pin a HIGH e il MOSFET P interrompe la corrente al sensore. Il SCD41 si "spegne" completamente e non consuma più nulla.

2. **VGNSS_Ctrl** fa lo stesso per il GPS. Utile perché il GPS è di gran lunga la periferica che consuma di più durante l'acquisizione.

3. **ADC_Ctrl (GPIO 37)** abilita il partitore resistivo per leggere la tensione batteria. Anche il partitore consuma corrente (~30 μA con il rapporto 4.9:1). Lo si abilita solo per il tempo della lettura ADC, poi lo si spegne.

Il pattern generale è: **accendi la periferica solo quando serve, spegnila appena hai finito**. Nel `setup()` la sequenza è deterministica: prima si alimenta, poi si aspetta la stabilizzazione (delay 100 ms), poi si comunica.

---

## Sensore SCD41: I²C e misure periodiche

<p align="center">
  <img src="img/scd41.png" alt="Sensore SCD41 Sensirion" width="350">
</p>

*Figura 3: sensore CO₂ Sensirion SCD41. Include misura di CO₂ (400-5000 ppm) via fotoacustica, oltre a temperatura e umidità. Comunica via I²C all'indirizzo 0x62.*

Il SCD41 è un sensore a **misure periodiche**: gli si dà un comando `startPeriodicMeasurement`, e da quel momento in poi produce una misura ogni 5 secondi. Non c'è "on-demand": non puoi chiedere "misura ora, dammi il valore adesso".

Il flusso è quindi:

```cpp
scd4x.stopPeriodicMeasurement();   // per sicurezza dopo reset "sporco"
delay(500);
scd4x.getSerialNumber(serialNumber);
scd4x.startPeriodicMeasurement();  // parte il campionamento

// [nel frattempo aspetto il GPS...]

// Ora leggo l'ultima misura pronta
while (!ready) {
    scd4x.getDataReadyStatus(ready);
    if (ready) scd4x.readMeasurement(co2, tempC, humRH);
    delay(200);
}

scd4x.stopPeriodicMeasurement();   // spengo prima di sleep
```

**Perché stopPeriodicMeasurement prima di startPeriodicMeasurement?** Se il device fa un reset software mentre il sensore stava misurando, il SCD41 mantiene lo stato "periodic measurement active". Al boot successivo, senza uno `stopPeriodicMeasurement`, il comando `startPeriodicMeasurement` fallisce perché il sensore è già in quello stato. Lo stop iniziale è una salvaguardia.

**Perché `getDataReadyStatus`?** Il sensore ci mette ~5 secondi tra una misura e l'altra. Se leggi troppo presto, ricevi valori nulli o vecchi. Il polling di `getDataReadyStatus` (con delay tra un tentativo e l'altro) è il modo pulito di aspettare senza bloccare.

**Conversione a interi scalati**. Le API SCD41 restituiscono `float` per temperatura e umidità. Il payload usa interi scalati x100. La conversione avviene una sola volta al momento di comporre il payload:

```cpp
payload.temp_c100  = (int16_t)(tempC * 100.0f);
payload.hum_pct100 = (uint16_t)(humRH * 100.0f);
```

I decimali oltre il centesimo vengono tagliati. Un `int16_t` copre da -327.68°C a +327.67°C, ben oltre il range fisiologico del sensore. Sufficiente.

---

## GPS L76K: parsing NMEA e timeout adattivo

Il L76K comunica via UART inviando **stringhe NMEA**, uno standard testuale della marina militare americana che è diventato il de facto standard per i GPS. Frasi tipo:

```
$GNGGA,143045.00,3752.5678,N,01505.4321,E,1,08,0.9,42.5,M,45.8,M,,*47
$GNRMC,143045.00,A,3752.5678,N,01505.4321,E,0.05,180.5,110826,,,A*7B
$GNGSV,3,1,10,03,45,180,42,05,30,090,37,08,60,270,45*70
```

<p align="center">
  <img src="img/nmea_format.png" alt="Struttura di una sentenza NMEA" width="600">
</p>

*Figura 4: struttura di una sentenza NMEA. Ogni riga inizia con `$`, seguita dal talker ID (GP=GPS, GN=multi-GNSS), il tipo di frase (GGA, RMC, GSV, ...), i campi separati da virgola, e termina con un checksum XOR dopo `*`.*

Il parser NMEA usato è **TinyGPSPlus** di Mikal Hart, che consuma i caratteri uno alla volta e mantiene lo stato interno. Il flusso:

```cpp
while (GpsSerial.available()) {
    gps.encode(GpsSerial.read());
}
if (gps.location.isValid() && gps.hdop.isValid() && gps.hdop.hdop() < 50.0) {
    // fix valido!
}
```

Il controllo `hdop < 50.0` è un filtro di qualità: HDOP (Horizontal Dilution of Precision) misura quanto è "buona" la geometria dei satelliti visibili. Valori bassi (<1) sono ottimi, alti (>10) sono scarsi. Il threshold 50 è generoso ma esclude i fix "impossibili" (tipici del bootstrap).

**Timeout adattivo cold/warm start**. Un GPS che si accende per la prima volta senza nessuna informazione (cold start) può richiedere fino a **90 secondi** per il primo fix, perché deve:
- ricevere l'almanacco (informazioni orbitali) via broadcast satellitare
- acquisire il tempo TOW (Time Of Week)
- risolvere l'ambiguità di posizione iniziale

Una volta che ha un fix e viene ricaricato (warm start, con l'almanacco valido in memoria) il fix richiede tipicamente 5-30 secondi. Il firmware distingue i due casi:

```cpp
uint32_t gpsTimeout = hasWarmData ? GPS_FIX_TIMEOUT_WARM_S : GPS_FIX_TIMEOUT_COLD_S;
```

`hasWarmData` è una variabile in RTC memory: sopravvive al deep sleep. Al primo boot dopo un power-cycle è `false` (cold), poi diventa `true` per tutti i wake successivi.

**Fallback a "posizione stale"**. Se il fix non arriva entro il timeout, il payload viene composto comunque usando l'ultima posizione valida nota:

```cpp
if (gpsOk) {
    // ...usa il fix corrente
    lastLat_e7 = payload.lat_e7;
    lastLon_e7 = payload.lon_e7;
    hasWarmData = true;
} else if (hasWarmData) {
    payload.lat_e7 = lastLat_e7;   // ultima posizione buona
    payload.hdop_x100 = 9999;       // marker "posizione stale"
}
```

Il valore `9999` in HDOP è un marker convenzionale: la dashboard può riconoscerlo e mostrare il marker in grigio invece di verde, indicando "posizione probabilmente vecchia".

---

## LoRaWAN con RadioLib 7.x

RadioLib è la libreria che astrae il chip SX1262 sotto uno stack LoRaWAN. La versione 7.x ha un'API leggermente diversa dalla 6.x, e il codice segue esattamente il pattern degli esempi ufficiali `LoRaWAN_ABP.ino`. La sequenza:

<p align="center">
  <img src="img/lorawan_class_a.png" alt="LoRaWAN classe A - RX windows" width="600">
</p>

*Figura 5: timing delle finestre RX in LoRaWAN classe A. Dopo ogni TX, il device apre due brevi finestre di ricezione (RX1 dopo 1 secondo, RX2 dopo 2 secondi) per ricevere eventuali downlink dal server. Al di fuori di queste finestre il device è in deep sleep e non può ricevere nulla.*

```cpp
ConfigLoRa_t config;
config.frequency = 868;              // MHz, poi LoRaWAN lo cambia da solo
radio.begin(config);

node.beginABP(devAddr, NULL, NULL, nwkSKey, appSKey);
node.activateABP();
```

Alcune cose non ovvie:

**`ConfigLoRa_t config` con solo `frequency = 868`**. Non ci sono altri campi da settare. Gli altri parametri (bandwidth, spreading factor, coding rate) sono default. È stato uno dei bug più fastidiosi da debuggare durante lo sviluppo: chiamando `radio.begin()` senza argomenti (come in RadioLib 6.x), il chip veniva inizializzato con bandwidth = 0, e ogni tentativo di TX falliva con errore `-1101 = INVALID_BANDWIDTH`.

**Chiavi ABP per LoRaWAN 1.0**. RadioLib 7.x supporta nativamente LoRaWAN 1.1, che ha **tre chiavi network separate**: `fNwkSIntKey`, `sNwkSIntKey`, `nwkSEncKey`. LoRaWAN 1.0 (che è quello supportato dal Conduit e da ChirpStack di default) ne ha una sola: `NwkSKey`. Per compatibilità 1.0 si passano le prime due chiavi come `NULL`:

```cpp
node.beginABP(devAddr, NULL, NULL, nwkSKey, appSKey);
```

Questo dice a RadioLib "attiva la sessione in modalità 1.0", usando la stessa `nwkSKey` per tutti i ruoli criptografici che 1.1 separerebbe. È il pattern documentato dalla LoRa Alliance per il fallback 1.1→1.0.

**`activateABP()` restituisce codici non-error**. La funzione ritorna `RADIOLIB_LORAWAN_NEW_SESSION` (=2) o `RADIOLIB_LORAWAN_SESSION_RESTORED` (=1), che **non sono errori** ma status. Il codice tratta i valori positivi come "OK" e prosegue.

**`sendReceive` in classe A**. Il metodo `node.sendReceive(buf, len, LORAWAN_FPORT)` esegue la sequenza completa della LoRaWAN classe A:

1. Trasmette il pacchetto su `LORAWAN_FPORT` (= 1 nel nostro caso)
2. Apre finestra RX1 (dopo 1 secondo, sulla stessa frequenza del TX ma con DR diverso)
3. Apre finestra RX2 (dopo 2 secondi, su 869.525 MHz, DR0)
4. Se riceve un downlink, lo decodifica automaticamente
5. Ritorna con codice che indica se il downlink è arrivato in RX1 (=1), RX2 (=2), o nessun downlink (=0)

Non si può fare TX senza aprire le due finestre RX: è specifica del protocollo LoRaWAN classe A. Il consumo delle finestre RX è ~10-15 mA per pochi secondi.

---

## Persistenza del frame counter

Il **frame counter uplink (FCnt)** è un contatore monotono che ogni end-device incrementa ad ogni trasmissione. Il Network Server ne tiene traccia e **rifiuta i pacchetti con FCnt minore di quello atteso** — protezione contro attacchi di replay.

Il problema: al reset del device (batteria staccata, brown-out, reflash del firmware), il FCnt in memoria RAM si perde. Se il device riparte da FCnt=0, i suoi pacchetti vengono rigettati dal NS finché non risale sopra l'ultimo valore visto.

Le soluzioni tipiche sono tre:

**A) Disabilitare la verifica FCnt lato server** ("Skip frame-counter check" su ChirpStack, "FCnt Check Enabled = false" sul Conduit). Semplice ma indebolisce la sicurezza — un attaccante che intercetta un pacchetto può riprodurlo.

**B) Salvare il FCnt in memoria non volatile** ed incrementarlo con margine di sicurezza al boot. Robusto ma comporta scritture su flash, che ha vita limitata (~100.000 cicli per settore).

**C) Passare a OTAA** (Over-The-Air Activation). Ogni join genera un nuovo DevAddr e nuovi FCnt, senza collisioni. Ma richiede procedure di join più complesse e la partecipazione attiva del NS.

Il firmware usa la strategia **B** con ottimizzazioni:

```cpp
#define FCNT_NVS_SAVE_EVERY   200    // salva ogni 200 uplink
#define FCNT_BOOT_MARGIN      200    // al boot salta avanti di 200

// RTC memory: sopravvive a deep sleep, persa a power-off
RTC_DATA_ATTR uint8_t rtcSessionBuffer[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
RTC_DATA_ATTR bool    rtcSessionValid = false;

// NVS (flash): sopravvive a power-off, reset HW, reflash del firmware
Preferences prefs;
```

Al boot si prova prima la RTC memory (veloce, nessuna usura), poi la NVS come fallback:

```cpp
if (rtcSessionValid) {
    node.setBufferSession(rtcSessionBuffer);
} else if (loadSessionFromNVS(sessionBuf)) {
    node.setBufferSession(sessionBuf);
}
```

Dopo ogni TX, si aggiorna sempre la RTC memory; si salva in NVS solo ogni 200 cicli (write batching):

```cpp
cacheSessionInRTC();       // sempre
if (currentFCnt % FCNT_NVS_SAVE_EVERY == 0) {
    saveSessionToNVS();    // ogni 200 cicli
}
```

**Perché non salvare l'FCnt raw (4 byte) ma l'intero session buffer (~300 byte)?** Perché in RadioLib 7.x **non esiste `setFCntUp()`**. L'unico modo per ripristinare il FCnt è tramite `setBufferSession(buf)`, che ricarica l'intero stato LoRaWAN. Il session buffer contiene FCnt + parametri MAC + configurazione ADR + molto altro. Scriverlo tutto è più byte ma è l'API disponibile.

**Vita della flash**. Con TX ogni minuto e save ogni 200 cicli:
- ~7 scritture NVS al giorno
- Con wear leveling ESP-IDF, ogni settore riceve una frazione delle scritture totali
- Vita stimata: **>10 anni** anche in condizioni pessimistiche

**Il flag `ENABLE_NVS_PERSISTENCE`** permette di disattivare completamente NVS per debug:

```cpp
#define ENABLE_NVS_PERSISTENCE  1  // 0 per debug, 1 per produzione
```

Durante lo sviluppo (con "skip FCnt check" attivo sul server), tenerlo a 0 evita di usurare la flash con scritture inutili.

---

## Deep sleep e GPIO hold

L'ESP32-S3 supporta il **deep sleep** con consumo tipico <10 μA. In questa modalità:

<p align="center">
  <img src="img/esp32_sleep_modes.png" alt="ESP32 sleep modes" width="700">
</p>

*Figura 6: le modalità di risparmio energetico dell'ESP32. Il **deep sleep** spegne CPU, RAM principale, WiFi e BT, lasciando acceso solo il dominio RTC (con RTC RAM da 8 KB e i pin RTC GPIO). Il chip può svegliarsi solo tramite le sorgenti configurate (timer, GPIO, ULP).*

- CPU spenta
- RAM principale (SRAM) spenta → contenuto perso
- WiFi, BT, radio spenti
- USB CDC spento (importante: la porta COM sparisce da Windows)
- **Dominio RTC** resta acceso: timer di wake, RTC RAM (8 KB), RTC GPIO

Le variabili marcate `RTC_DATA_ATTR` vengono piazzate in RTC RAM invece che in SRAM, sopravvivendo al sleep. Il wake avviene tramite:

```cpp
esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
esp_deep_sleep_start();
```

Il timer RTC è basato su oscillatore XTAL o RC interno; la precisione è >99% ma non perfetta.

**Il problema dei GPIO che vanno flottanti**. Quando il dominio digital si spegne, i pin GPIO normali perdono il loro stato: tornano flottanti (o al default hardware pull-up/down). Per pin che comandano MOSFET di alimentazione, questo può causare disastri: `VEXT_CTRL` a HIGH (spento) potrebbe diventare flottante, il MOSFET P si accende parzialmente, Vext resta alimentata e il SCD41 continua a consumare.

La soluzione è il **digital hold**:

```cpp
gpio_hold_en((gpio_num_t)PIN_VEXT_CTRL);   // congela il livello attuale (HIGH)
gpio_hold_en((gpio_num_t)PIN_VGNSS_CTRL);
gpio_deep_sleep_hold_en();                  // arma gli hold globalmente
```

`gpio_hold_en` dice al dominio RTC di mantenere il livello logico corrente del pin anche mentre il digital è spento. `gpio_deep_sleep_hold_en` è l'interruttore globale che abilita gli hold ad avere effetto in deep sleep.

Al risveglio, prima di poter di nuovo manovrare i pin, bisogna **rilasciare** l'hold:

```cpp
gpio_hold_dis((gpio_num_t)PIN_VEXT_CTRL);
gpio_hold_dis((gpio_num_t)PIN_VGNSS_CTRL);
gpio_deep_sleep_hold_dis();
```

Se non lo fai, `digitalWrite(PIN_VEXT_CTRL, LOW)` non ha effetto: il pin resta bloccato al valore precedente. Debug fastidioso.

---

## Protezione batteria da under-discharge

Le celle litio-ione (LiPo, 18650, 14500) hanno un limite basso di tensione oltre il quale iniziano a degradarsi in modo **irreversibile**. Per una cella LiPo standard il valore critico è **~2.5 V**; sotto questa soglia:

- La struttura cristallina degli elettrodi collassa parzialmente
- La cella perde capacità residua (spesso >50%)
- Alla ricarica successiva può entrare in "deep discharge lockout" e non rispondere
- Nel peggiore dei casi, si può gonfiare durante la ricarica successiva o entrare in thermal runaway

Il pattern classico per prevenire questi problemi è a due livelli:

1. **Protezione hardware** — una BMS (Battery Management System) o PCM (Protection Circuit Module) esterna che monitora la tensione della cella e **fisicamente scollega il carico** quando la tensione scende sotto la soglia. Reagisce in microsecondi, funziona anche se il firmware è morto.

<p align="center">
  <img src="img/bms_1s_5a.png" alt="BMS 1S 5A per 18650" width="300">
</p>

*Figura 2: scheda BMS 1S 5A per celle 18650/14500/LiPo. I quattro pad principali sono B+ e B− (verso la cella), P+ e P− (verso il carico).*

2. **Protezione software** — il firmware legge periodicamente la tensione batteria, e se scende sotto una soglia di sicurezza mette il device in "emergency mode": deep sleep di lunga durata, niente trasmissioni, minimo consumo assoluto.

### Quando entrambe servono, e quando basta il software

Con il TP4054 integrato nella Heltec V4, la ricarica è già protetta (over-charge). Il gap che rimane è l'over-discharge sul carico.

**Se il device sta sul tuo tavolo** con monitoraggio via seriale o via dashboard MQTT, e usi celle di buona qualità (Vapcell, Samsung, LG), la protezione software da sola è sufficiente:
- Vedi in dashboard che `battery_pct` scende
- Ricarichi prima che diventi critico
- La protezione software è una safety net che ti evita di rovinare la cella se dimentichi il device sotto carica per settimane

**Se il device è sigillato in un case senza accesso visivo** (installazione permanente, box outdoor, deployment su campo lontano), la protezione software da sola non basta e serve una BMS hardware. Ecco perché:

*Il device sigillato non può essere "visto"*. Non hai un LED che ti dice "batteria scarica, ricaricami". Il device si comporta come uno zombie: continua a fare boot ogni tot minuti, la cella si scarica sempre di più, tu non te ne accorgi perché sei lontano.

*L'ultimo TX di un device sigillato spesso non riesce*. Quando la batteria è quasi scarica, il picco di corrente della trasmissione LoRa (~130 mA) fa crollare la tensione sotto la soglia di brown-out del ESP32 (~2.4V). Il chip fa reset, riprova, fa reset di nuovo. Il device non riesce mai a completare un TX, quindi il dashboard non riceve mai il messaggio "sono al 10% di batteria!". Da fuori sembra che il device sia semplicemente offline. Nel frattempo la cella continua a scaricarsi ogni volta che il chip fa un tentativo, fino a distruggersi.

*Il ciclo di boot infinito accelera il danno*. Ogni boot consuma corrente prima ancora di poter fare qualcosa. Il device sigillato entra in un loop di reset ripetuti che scarica la cella molto più velocemente di un firmware pulito che va in sleep. In 24-48 ore la cella può scendere sotto 1.5V, punto senza ritorno.

*Un cortocircuito interno passa inosservato*. Se una vibrazione o uno stress termico causa un contatto anomalo dentro il case (filo che si scolla, isolante che cede), non c'è nessuno a spegnere il device. La cella eroga corrente massima in cortocircuito, si scalda, nel peggiore dei casi prende fuoco. La BMS esterna interviene in <100 μs e taglia il carico.

*Nessuno può intervenire prima che il danno sia fatto*. In un ambiente controllato ti accorgi delle stranezze in tempo reale. In un box sigillato in un capannone, in una serra remota, sopra un traliccio, l'unica difesa è quella integrata nell'hardware.

### Il codice della protezione software

Il firmware espone tre `#define` per configurare le soglie:

```cpp
#define ENABLE_BATTERY_PROTECTION  1
#define VBAT_EMERGENCY_MV       3100   // sotto questa soglia: emergency sleep
#define VBAT_RECOVERY_MV        3300   // sopra: ripresa operativita' normale
#define VBAT_EMERGENCY_SLEEP_S  21600  // 6 ore di sleep in emergenza
```

E nel `setup()`, subito dopo la lettura di `vbat_mv`:

```cpp
#if ENABLE_BATTERY_PROTECTION
if (vbat_mv > 0 && vbat_mv < VBAT_EMERGENCY_MV) {
    Serial.printf("BATTERIA CRITICA: %u mV\n", vbat_mv);
    enterDeepSleep(VBAT_EMERGENCY_SLEEP_S);
}
#endif
```

Con questo, se la tensione scende sotto 3.1 V:
- Il device non trasmette (evita picco di corrente di ~130 mA)
- Non attiva GPS e SCD41 (evita consumo di ~50 mA)
- Non fa I2C né altre operazioni ridondanti
- Va in deep sleep per 6 ore

Sei ore dopo si sveglia, rilegge la batteria. Se è stata ricaricata nel frattempo (magari il pannello solare ha fatto il suo lavoro), riprende operatività normale. Se è ancora sotto soglia, torna in emergency sleep per altre 6 ore.

### La scelta delle soglie

La soglia critica **3.1 V** è conservativa. La cella si danneggia sotto ~2.5 V, ma:
- Il partitore ADC della V4 ha una tolleranza di ~5%
- Il picco di trasmissione TX fa scendere ulteriormente di ~200 mV per una frazione di secondo
- Un margine di sicurezza porta la soglia realistica a 3.0-3.2 V

**3.1 V** lascia un margine per il picco TX (finiresti a ~2.9 V durante il TX) e per l'errore di misura ADC. Corrisponde a circa il **5-10% di capacità residua**.

Se usi celle di qualità inferiore o rigenerate, alza la soglia a **3.2-3.3 V** per un margine ancora più generoso.

**VBAT_RECOVERY_MV = 3300 mV** (isteresi) è la soglia sopra la quale si riprende operatività normale. L'isteresi (differenza tra soglia di ingresso e uscita dall'emergency mode) è importante: se avessi la stessa soglia per entrare e uscire, il device oscillerebbe tra i due stati quando la tensione è vicina alla soglia, sprecando cicli. Con 200 mV di isteresi, l'emergency mode "sente" solo transizioni nette.

**VBAT_EMERGENCY_SLEEP_S = 6 ore** è un compromesso tra risparmio massimo (dormire per giorni) e possibilità di ripresa rapida (svegliarsi ogni ora per controllare). Con 6 ore, se il pannello solare ricarica di giorno, entro un giorno il device è di nuovo operativo.

### Cosa lascia scoperto

La protezione software è ottima ma non è infallibile. Casi in cui fallisce:

*Firmware corrotto*. Se il firmware crasha, entra in loop, o ha un bug che gli impedisce di leggere correttamente la batteria, la protezione software non serve a niente. La BMS hardware invece funziona sempre, indipendentemente dal firmware.

*Cortocircuito improvviso*. Il picco di corrente di un cortocircuito è tale che nessun firmware può intervenire in tempo — servono microsecondi, il codice ha bisogno di millisecondi. La BMS hardware è progettata per questo caso specifico.

*Brown-out prima di poter salvare stato*. Se la batteria è così scarica che il chip fa reset prima di poter completare il codice di emergency sleep, si entra nel loop di boot infinito descritto sopra. La BMS interrompe il carico ben prima che si arrivi a questo punto.

Per un uso didattico da laboratorio, la protezione software è più che sufficiente. Per un deployment reale in scatola sigillata, si aggiunge sempre la BMS hardware come cintura + bretelle.

---

## Watchdog hardware

Un firmware embedded può bloccarsi in modi inaspettati anche quando è stato testato a fondo:

- **Bug latenti in librerie terze** che si manifestano solo con particolari sequenze di dati
- **Deadlock su bus condivisi** (per esempio se il SCD41 non risponde al comando I²C e la libreria aspetta all'infinito)
- **Loop infiniti** in gestori di errore mal scritti
- **Interferenze RF** che corrompono lo stato interno di un chip a partire dal SPI o UART

Se il firmware si blocca, il device non entra in deep sleep, e le periferiche restano tutte accese assorbendo corrente in continuazione. Una batteria da 1500 mAh si scarica in **~15-40 ore** a seconda di quali periferiche sono rimaste attive. Nel caso peggiore, se il device è sigillato in un case in campo aperto, non hai modo di accorgertene se non quando smette di trasmettere.

### Come funziona il watchdog dell'ESP32-S3

L'ESP32-S3 ha un **hardware watchdog timer** integrato: un contatore che decrementa nel tempo. Se il firmware non lo resetta periodicamente, il contatore raggiunge zero e il chip **fa un reset hardware forzato**.

L'analogia da cui viene il nome è quella di un cane da guardia legato in giardino: ogni tanto il padrone deve andare a spazzolarlo (pet the dog in inglese, letteralmente accarezzarlo) per confermare la propria presenza. Se il padrone non torna per troppo tempo, il cane si insospettisce e "abbaia" (nel nostro caso: fa reset del device). Nel gergo tecnico embedded, l'operazione di ripristinare il timer viene chiamata "**pettinare il cane**" (o *feed the dog*, *kick the watchdog*): si traduce concretamente in una singola scrittura in un registro del chip che azzera il contatore.

Il ciclo di vita è:

1. **All'inizio del setup** si arma il watchdog con un timeout (nel nostro caso 120 secondi)
2. **Il task corrente** (in Arduino: `loopTask`) si iscrive al watchdog
3. **In vari punti del ciclo** si chiama `esp_task_wdt_reset()` per confermare al watchdog che il firmware è vivo
4. **Se il firmware si blocca** e non pettina il watchdog entro il timeout, il chip fa reset
5. **Prima del deep sleep** si rimuove il task dal watchdog (altrimenti il sleep verrebbe interpretato come blocco)

Il reset causato dal watchdog è visibile nel log di boot successivo tramite `esp_reset_reason()`, permettendo di rilevare che c'è stato un problema.

### La scelta del timeout

Il timeout va dimensionato sul massimo tempo che un ciclo normale può richiedere, con un margine di sicurezza.

Nel nostro caso, la parte più lenta è il **cold start GPS**, che può arrivare a **90 secondi**. Aggiungendo tempo per SCD41, TX LoRa, finestre RX, il ciclo attivo massimo è circa **100-110 secondi**.

**Scelta**: `WDT_TIMEOUT_S = 120`. Genereo abbastanza da non scattare in condizioni normali, ma abbastanza corto da recuperare rapidamente da un blocco reale.

Timeout troppo lungo (per esempio 600 s) = il device può stare bloccato molto prima del reset, con consumo alto.

Timeout troppo corto (per esempio 30 s) = il watchdog scatta durante un cold start GPS normale, causando reset spuri.

### Dove pettinare il watchdog

Il watchdog viene resettato in tre punti chiave del firmware:

**1. Nel loop di attesa fix GPS**, ogni 5 secondi (nel log periodico):
```cpp
#if ENABLE_WATCHDOG
    esp_task_wdt_reset();
#endif
```
Necessario perché il cold start GPS può superare il timeout senza questa chiamata.

**2. Subito dopo il TX LoRa**:
```cpp
    Serial.println("TX ok, nessun downlink");
#if ENABLE_WATCHDOG
    esp_task_wdt_reset();
#endif
```
Marker "sono arrivato fuori dalla parte lenta del ciclo".

**3. Prima del deep sleep** (in `enterDeepSleep`), rimozione del task:
```cpp
#if ENABLE_WATCHDOG
    esp_task_wdt_delete(NULL);
#endif
```
Il sleep interrompe l'esecuzione, quindi il task non pettinerà più il watchdog. Se non lo togliessimo, il watchdog resetterebbe il device durante il sleep. Al prossimo wake, `setup()` lo riarma da capo.

### Come attivare/disattivare

Il flag `#define ENABLE_WATCHDOG` controlla tutta la logica:

```cpp
#define ENABLE_WATCHDOG    1   // 0 durante debug con breakpoint
#define WDT_TIMEOUT_S    120
```

**Metti a 0 durante il debug attivo** con breakpoint dell'IDE: se fermi l'esecuzione a esaminare lo stato, il watchdog non sa che stai debuggando e resetta il device. Molto fastidioso.

**Metti a 1 in modalità produzione**: la ridondanza è a costo zero e ti salva da situazioni impreviste.

### Il costo in termini di risorse

- **RAM**: alcune decine di byte per il descrittore del watchdog
- **CPU**: le chiamate `esp_task_wdt_reset()` sono in scrittura registro, tempi nell'ordine del microsecondo
- **Energia**: praticamente zero (nessun impatto sul consumo)
- **Complessità**: 5-6 righe di codice sparse, tutte condizionali sotto `#if ENABLE_WATCHDOG`

Un caso in cui il watchdog **non aiuta**: se il blocco avviene in un interrupt di alta priorità che tiene la CPU costantemente occupata, il task `loopTask` non gira e non può nemmeno essere resettato. Ma questi casi sono rari nell'architettura Arduino/ESP32 tipica.

### Diagnostica del reset

Se sospetti che il device stia venendo resettato dal watchdog, puoi verificarlo aggiungendo all'inizio del setup:

```cpp
esp_reset_reason_t reason = esp_reset_reason();
Serial.printf("Reset reason: %d\n", reason);
// ESP_RST_TASK_WDT = 7  → il watchdog ha fatto reset
```

Se vedi `Reset reason: 7`, sai che il ciclo precedente è stato interrotto forzatamente e vale la pena indagare cosa non andava.

---

## Downlink handler e comandi remoti

Il firmware espone la possibilità di ricevere **comandi remoti** dal Network Server tramite downlink LoRaWAN. Il device resta in **classe A** — significa che i comandi vengono ricevuti solo nelle due brevi finestre RX che si aprono dopo ogni uplink. La latenza massima di un comando è quindi l'intervallo tra due TX (~60 secondi con la configurazione di default).

<p align="center">
  <img src="img/lorawan_architecture.png" alt="Architettura LoRaWAN end-to-end" width="700">
</p>

*Figura 7: architettura LoRaWAN end-to-end. I dati partono dal device end-node (a sinistra), attraversano il canale radio verso i gateway, che li inoltrano via IP al Network Server. L'Application Server decifra i payload applicativi e li rende disponibili a dashboard, database, API. I downlink seguono il percorso inverso.*

### La filosofia: comandi ordinari e configurazione

Seguendo la [convenzione dei topic MQTT del progetto](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/approfondimenti/messaggi_mqtt.md), i downlink al device sono divisi concettualmente in due categorie:

- **Comandi ordinari** — azioni one-shot che non modificano lo stato persistente del device (reboot, identify, forza uplink). Corrispondono al topic MQTT `<ambiente>/comandi/<device>/`.
- **Configurazione** — modifiche persistenti a parametri del device (intervallo TX, potenza radio, soglie di batteria). Corrispondono al topic MQTT `<ambiente>/config/<device>/`.

A livello LoRaWAN, la distinzione è realizzata tramite due FPort dedicati:

| Categoria | FPort LoRaWAN | Persistenza |
|-----------|---------------|-------------|
| Comandi ordinari | **10** | No (one-shot) |
| Configurazione | **20** | Sì (salvata in NVS) |

### Il codec valida la coerenza cmd/FPort

Il codec ChirpStack riceve dal client MQTT sia il comando (dentro `object`) che il FPort su cui deve viaggiare. Non sovrascrive il FPort: si limita a:

1. **Verificare che il comando esista**
2. **Verificare che sia coerente con il FPort scelto** (es. `reboot` è valido su FPort 10, non su FPort 20)
3. **Tradurlo in byte binari**

```javascript
function encodeDownlink(input) {
    var cmd = String(input.data.cmd).toLowerCase();
    var port = input.fPort;

    if (port === 10) {
        // Comandi di azione
        if (cmd === "reboot")   return { bytes: [0x01] };
        if (cmd === "identify") return { bytes: [0x02] };
        // ...
        return { errors: ["Comando FPort 10 sconosciuto: " + cmd] };
    }

    if (port === 20) {
        // Configurazione persistente
        if (cmd === "set_tx_interval") return { bytes: [0x11, input.data.value] };
        // ...
    }
}
```

Chi pubblica il downlink deve conoscere la mappatura comando→FPort. Il vantaggio è che il messaggio MQTT è **auto-descrittivo**: guardando il `fPort` capisci subito la natura del comando, e se un domani vuoi inviare un downlink saltando il codec (con `data` pre-encoded invece di `object`), il FPort è già quello corretto.

Se sbagli e mandi `"cmd":"reboot"` con `"fPort":20`, il codec ti risponde con un errore esplicito che elenca i comandi validi per quel FPort.

### Il primo byte come identificatore di comando

All'interno del payload di ciascun FPort, il **primo byte** è il `command_id`, seguito da eventuali argomenti binari. È lo stesso pattern versionato del `schema_id` dell'uplink: puoi aggiungere comandi nuovi in futuro senza rompere quelli esistenti, e il device può respingere comandi sconosciuti in modo pulito.

Esempio dello scambio per cambiare TX interval:

```
Utente pubblica:      MQTT su application/.../command/down
                      {"fPort":20, "object":{"cmd":"set_tx_interval","value":3}}
        │
        ▼
Codec ChirpStack:     encodeDownlink({data:{cmd:"set_tx_interval",value:3}, fPort:20})
                        → verifica cmd valido su FPort 20 ✓
                        → valida value=3 in range 0-5 ✓
                        → ritorna { bytes: [0x11, 0x03] }
        │
        ▼
LoRaWAN downlink:     FPort=20, payload = [0x11, 0x03]
                                            │     └─ preset 3 = 5 minuti
                                            └─────── comando SET_TX_INTERVAL
        │
        ▼
Firmware dispatcha:   handleDownlinkPort20([0x11, 0x03], 2)
                        → riconosce CFG_SET_TX_INTERVAL
                        → valida 0x03 in range 0-5
                        → salva in NVS con chiave "tx_int"
                        → aggiorna cfgTxIntervalPreset in RAM
                        → stampa log "SET_TX_INTERVAL: preset=3 (300s)"

Ciclo successivo:     usa TX_INTERVAL_SECONDS[cfgTxIntervalPreset] = 300s
```

### La lista dei comandi supportati

**FPort 10 — Comandi ordinari**

| Comando | Byte | Argomenti | Descrizione |
|---------|------|-----------|-------------|
| `REBOOT` | 0x01 | (nessuno) | Riavvia il device via `ESP.restart()` |
| `IDENTIFY` | 0x02 | (nessuno) | LED lampeggia 10 volte per identificare visivamente |
| `FORCE_TX_NOW` | 0x03 | (nessuno) | Setta flag `forceTxNow`, prossimo ciclo TX dopo 2 secondi |
| `CLEAR_NVS` | 0x04 | `0xA5` (magic) | Cancella l'intera NVS del namespace `lora`, riavvia |

Il byte magic `0xA5` su `CLEAR_NVS` è una **cintura di sicurezza**: senza di esso il comando viene ignorato. Un errore accidentale (comando corrotto, checksum sbagliato) non deve poter cancellare una sessione LoRaWAN valida.

**FPort 20 — Configurazione persistente**

| Comando | Byte | Argomenti | Descrizione |
|---------|------|-----------|-------------|
| `SET_TX_INTERVAL` | 0x11 | 1 byte (0-5) | Cambia preset TX: 0=10s, 1=20s, 2=1min, 3=5min, 4=10min, 5=30min |
| `SET_LORAWAN_SF` | 0x12 | 1 byte (7-12) | Cambia spreading factor |
| `SET_TX_POWER` | 0x13 | 1 byte (2-14) | Cambia potenza TX in dBm |
| `SET_GPS_TIMEOUT` | 0x14 | 2 byte LE (10-300) | Cambia timeout attesa fix GPS in secondi |
| `SET_BATT_THRESH` | 0x15 | 4 byte (2×uint16 LE) | Cambia soglie batteria emergency + recovery |

Ogni comando include **validazione dei range** sia lato codec (prima di trasmettere) sia lato firmware (prima di salvare in NVS): valori fuori dai limiti vengono ignorati e loggati. Questo evita che una NVS corrotta possa portare il device in configurazioni impossibili (es. SF=15).

### Persistenza in NVS delle configurazioni

I comandi FPort 20 salvano il nuovo valore in NVS con chiavi dedicate:

```cpp
#define NVS_KEY_TX_INTERVAL   "tx_int"
#define NVS_KEY_LORAWAN_SF    "lora_sf"
#define NVS_KEY_TX_POWER      "tx_pow"
#define NVS_KEY_GPS_TIMEOUT   "gps_t"
#define NVS_KEY_VBAT_EMERG    "vbat_em"
#define NVS_KEY_VBAT_RECOV    "vbat_rc"
```

Al boot successivo, `loadRuntimeConfig()` legge tutte le chiavi. Se una manca (primo boot, o valore mai settato via downlink), usa il default hardcoded dal corrispondente `#define`:

```cpp
void loadRuntimeConfig() {
    // Default hardcoded
    cfgTxIntervalPreset = TX_INTERVAL_PRESET;
    cfgLoRaWANSF        = LORAWAN_SF;
    // ...
    if (prefs.begin(NVS_NAMESPACE, true)) {
        cfgTxIntervalPreset = prefs.getUChar(NVS_KEY_TX_INTERVAL, cfgTxIntervalPreset);
        cfgLoRaWANSF        = prefs.getUChar(NVS_KEY_LORAWAN_SF,  cfgLoRaWANSF);
        // ...
        prefs.end();
    }
    // Sanity check contro NVS corrotta
    if (cfgLoRaWANSF < 7 || cfgLoRaWANSF > 12) cfgLoRaWANSF = LORAWAN_SF;
}
```

Il pattern è quindi:
- **Il `#define`** è il valore di fabbrica (compile-time)
- **La NVS** è il valore attuale (runtime, modificabile da remoto)
- **La variabile `cfg*`** è quello effettivamente usato nel codice

Cambiare un `#define` e riflashare **non sovrascrive** il valore in NVS: i device sul campo mantengono la loro config remota. Solo un `CLEAR_NVS` esplicito ripristina i default.

### Esempi mosquitto_pub

Il topic canonico di ChirpStack per i downlink è:

```
application/<app-id>/device/<devEUI>/command/down
```

Le variabili da adattare:
- `<app-id>` — Application ID di ChirpStack (es. `1`, lo trovi nell'URL della UI)
- `<devEUI>` — DevEUI del device in lowercase senza trattini (es. `26a160fffe6e86bc`)

**Il campo `fPort` nel JSON deve corrispondere alla natura del comando**: `10` per azioni ordinarie, `20` per configurazione. Il codec verifica la coerenza e rifiuta i comandi inviati sul FPort sbagliato. Questa scelta rende il messaggio MQTT auto-descrittivo: guardando il JSON capisci subito la categoria del comando senza dover decodificare i byte.

**Reboot del device** (azione, FPort 10):
```bash
mosquitto_pub -h <IP-ChirpStack> \
  -t "application/1/device/26a160fffe6e86bc/command/down" \
  -m '{"fPort":10,"object":{"cmd":"reboot"}}'
```

**Identify** (azione, FPort 10):
```bash
mosquitto_pub -h <IP-ChirpStack> \
  -t "application/1/device/26a160fffe6e86bc/command/down" \
  -m '{"fPort":10,"object":{"cmd":"identify"}}'
```

**Forza uplink immediato** (azione, FPort 10):
```bash
mosquitto_pub -h <IP-ChirpStack> \
  -t "application/1/device/26a160fffe6e86bc/command/down" \
  -m '{"fPort":10,"object":{"cmd":"force_tx_now"}}'
```

**Cambia TX interval a 10 secondi** (configurazione, FPort 20):
```bash
mosquitto_pub -h <IP-ChirpStack> \
  -t "application/1/device/26a160fffe6e86bc/command/down" \
  -m '{"fPort":20,"object":{"cmd":"set_tx_interval","value":0}}'
```

**Riduci potenza TX a 2 dBm** (configurazione, FPort 20):
```bash
mosquitto_pub -h <IP-ChirpStack> \
  -t "application/1/device/26a160fffe6e86bc/command/down" \
  -m '{"fPort":20,"object":{"cmd":"set_tx_power","value":2}}'
```

**Soglie batteria personalizzate** (configurazione, FPort 20):
```bash
mosquitto_pub -h <IP-ChirpStack> \
  -t "application/1/device/26a160fffe6e86bc/command/down" \
  -m '{"fPort":20,"object":{"cmd":"set_batt_thresholds","value":{"emergency_mv":3000,"recovery_mv":3200}}}'
```

Nel repository c'è anche uno **script bash `test_downlinks.sh`** che avvolge tutti questi comandi in un'interfaccia sintetica:

```bash
./test_downlinks.sh reboot
./test_downlinks.sh identify
./test_downlinks.sh set_tx_interval 0
./test_downlinks.sh sub_up
```

Le variabili di configurazione sono in cima allo script e si possono anche passare come environment override:

```bash
BROKER_HOST=192.168.1.50 EDGE_ID=lab-01 ./test_downlinks.sh identify
```

### Il flusso completo di un comando remoto

Ricapitolando il percorso di un comando dal momento in cui viene pubblicato a quando viene eseguito:

1. **Utente pubblica** su MQTT un JSON su `application/<id>/device/<eui>/command/down` con `{"fPort":10 o 20, "object":{"cmd":..., "value":...}}` — deve scegliere il FPort corretto per la categoria di comando
2. **ChirpStack riceve** il messaggio e chiama `encodeDownlink()` del codec
3. **Il codec valida** che il comando sia coerente con il FPort e restituisce `{bytes: [...]}` (o un errore se la coppia cmd/FPort non è valida)
4. **ChirpStack accoda** il downlink sul FPort ricevuto
5. **Il device fa il prossimo uplink** (secondo il suo intervallo TX)
6. **RadioLib apre le finestre RX** (1s e 2s dopo il TX)
7. **ChirpStack invia il downlink** nella finestra RX appropriata
8. **`node.sendReceive()` restituisce state > 0**, indicando che è arrivato un downlink
9. **Il firmware chiama** `handleDownlink(port, buf, len)`, che dispatcha in base al FPort
10. **Il comando viene eseguito**: azione immediata (FPort 10) o salvataggio in NVS (FPort 20)

Latenza tipica end-to-end: da alcuni secondi (se il device sta per trasmettere) a diversi minuti (se ha appena finito un ciclo).

### Ricognizione dei log

Quando arriva un downlink, il firmware stampa nel monitor seriale:

```
TX ok + downlink in RX1
[DOWNLINK] FPort=20 len=2 data=1103
[DOWNLINK] SET_TX_INTERVAL: preset=3 (300s)
```

Il primo log dice **quando** è arrivato (in RX1 o RX2). Il secondo dice **cosa** è arrivato (bytes esadecimali). Il terzo dice **cosa è stato fatto** (comando riconosciuto e applicato). Se un comando fallisce la validazione lato firmware, il terzo log dice il motivo:

```
[DOWNLINK] FPort=20 len=2 data=1207
[DOWNLINK] SET_LORAWAN_SF: SF fuori range 7-12
```

Utile per debug quando il comando arriva ma il device non si comporta come atteso.

---

## Modalità debug e produzione

Il firmware espone alcuni flag di configurazione in cima al file per switchare tra sviluppo e produzione:

```cpp
#define DEBUG_NO_DEEP_SLEEP        1   // 1 = niente deep sleep vero (USB CDC viva)
#define ENABLE_NVS_PERSISTENCE     0   // 0 = niente scritture flash durante il debug
#define ENABLE_BATTERY_PROTECTION  1   // 1 = emergency sleep se vbat sotto soglia
#define ENABLE_DOWNLINK_HANDLER    1   // 1 = interpreta i downlink come comandi
#define ENABLE_WATCHDOG            1   // 1 = wdt hardware, 0 durante debug con breakpoint
#define USE_OTAA                   0   // 1 = OTAA, 0 = ABP (attuale)
```

**In modalità debug** (`DEBUG_NO_DEEP_SLEEP = 1`):
- Al posto di `esp_deep_sleep_start()`, il codice usa `delay(seconds*1000)` + `ESP.restart()`
- USB CDC resta viva → la porta COM non sparisce da Windows
- Il Monitor Seriale continua a mostrare i log senza interruzioni
- Consumo alto (~30 mA) ma tanto sei con USB collegato

**In modalità produzione** (`DEBUG_NO_DEEP_SLEEP = 0`):
- Vero deep sleep
- USB CDC muore al sleep → porta COM sparisce
- Il Monitor Seriale si scollega dopo ogni TX
- Ma il device consuma davvero <10 μA in sleep

Per flashare un device che sta in deep sleep, si usa la sequenza pulsanti **BOOT + RST**:
1. Tieni premuto BOOT
2. Premi e rilascia RST (BOOT ancora giù)
3. Rilascia BOOT

Ora il device è in bootloader mode indipendentemente da cosa stava facendo il firmware. `esptool.py` può connettersi e flashare.

**Il flag `ENABLE_NVS_PERSISTENCE`** disattiva tutte le scritture in NVS quando è a 0. Utile per non usurare la flash durante lo sviluppo. Con NS che ha "skip FCnt check" attivo, non serve la persistenza per far funzionare il device.

**Il flag `USE_OTAA`** è predisposto per futura implementazione di OTAA come alternativa ad ABP. Attualmente vale sempre 0 (ABP). Quando sarà implementato, con `USE_OTAA=1` il firmware farà il join dinamico con AppKey invece di usare chiavi statiche.

---

## Diagnostica e osservabilità

Il firmware stampa un banner esteso ad ogni boot che include informazioni utili per il debug:

```
===================================================
 Heltec V4 - SCD41 + L76K - LoRaWAN ABP
 schema=0x42  boot#3  TX=60s  SF9  TxPow=14dBm
 DevEUI: 26-A1-60-FF-FE-6E-86-BC
 DevAddr (configurato): 260B262C
===================================================
```

**Il DevEUI derivato dal MAC**. L'ESP32 ha un MAC address di fabbrica (48 bit) leggibile con `ESP.getEfuseMac()`. Per costruire un DevEUI standard EUI-64 (64 bit) si applica il pattern IEEE MAC48-to-EUI64: inserisci `FF:FE` in mezzo:

```
MAC 48-bit:  26:A1:60:6E:86:BC
DevEUI 64:   26:A1:60:FF:FE:6E:86:BC
                     └─┬─┘
                    inseriti
```

Non è un identificatore ufficiale IEEE OUI-based, ma è un identificatore stabile e unico per quel chip. Perfetto per registrare il device sul Network Server.

**Log di ogni fase con timestamp**. Ogni step del ciclo stampa il proprio stato:

```
Attendo fix GPS (max 90 s)...
  [GPS] 5s  RX bytes=1541  frasi NMEA=54  sat.in.fix=0  fix=NO
  [GPS] 10s  RX bytes=2931  frasi NMEA=104  sat.in.fix=3  fix=NO
Fix ok in 12500 ms: lat=37.512345 lon=15.089876 sat=7 hdop=1.10
SCD41: CO2=485 ppm  T=20.36 °C  RH=47.5 %
Batteria: 4096 mV (89 %)
TX 32 byte: 42010700595D000000000000E501D8092D12...
TX ok, nessun downlink
FCnt uplink corrente: 42
```

Il timestamp permette di misurare quanto ci vuole ogni fase. Se il GPS fix dura 89 secondi ogni volta, forse il timeout warm-start di 30 secondi è troppo aggressivo — si può alzare.

**Decodifica errori RadioLib**. La funzione `sendPayload` stampa il significato dei codici di errore comuni:

```cpp
switch(state) {
    case -1101: Serial.println("  (bandwidth non valida - check band plan)"); break;
    case -1102: Serial.println("  (spreading factor non valido)"); break;
    case -1116: Serial.println("  (RX timeout - nessun downlink ricevuto, normale)"); break;
    ...
}
```

Questa è forse la modifica che ci è costata più tempo durante lo sviluppo: capire cosa vogliono dire i numeri negativi di RadioLib è la chiave per risolvere il 90% dei problemi di configurazione.

**Sketch di diagnostica dedicati**. Oltre al firmware principale, la repo contiene due sketch di test:

- `diagnostica_i2c_v4.ino`: alimenta Vext e fa uno scan I²C completo per verificare che il SCD41 (indirizzo 0x62) risponda. Utile per debug hardware (cavi, alimentazione, pull-up).
- `diagnostica_gps_v4.ino`: alimenta il GPS e stampa raw NMEA in tempo reale, per verificare che il modulo comunichi e che veda satelliti.

Questi sketch non usano LoRaWAN né altre astrazioni; sono minimali e mirati. Si caricano temporaneamente quando serve isolare un problema.

---

## Considerazioni finali

Il firmware, per essere solo un end-device LoRaWAN "banale", incorpora concetti che coprono uno spettro ampio:

- **Basso consumo con deep sleep** e gestione dell'alimentazione periferica
- **Sensori con protocolli diversi** (I²C sincrono per SCD41, UART asincrono per GPS)
- **Payload binario compatto versionato** con schema forward-compatible
- **Stack LoRaWAN** con classe A, ABP, banda EU868
- **Uso didattico di FPort** come discriminatore di tipo di messaggio
- **Persistenza multilivello** (RTC memory + NVS con write batching)
- **Downlink handler con configurazione runtime modificabile** da remoto
- **Protezione batteria** software da under-discharge
- **Watchdog hardware** per recupero automatico da blocchi imprevisti
- **Modalità debug/produzione** switchabili
- **Diagnostica strutturata** con logging e sketch dedicati

Alcune cose che non si sono fatte ma che varrebbe la pena aggiungere in un progetto reale:

- **OTAA invece di ABP**: elimina il problema del FCnt e semplifica il rollout. Il `#define USE_OTAA` è già predisposto
- **ADR (Adaptive Data Rate)** gestito dal server: attualmente il device usa sempre SF9 fisso; ADR permetterebbe di scendere a SF7 quando il gateway è vicino, risparmiando airtime
- **Persistenza dell'ultima posizione GPS in NVS** invece che solo in RTC memory: sopravvivrebbe anche a power-off (scelta didattica: non implementata perché la posizione stale può essere fuorviante se il device viene spostato mentre spento)
- **Node-RED come traduttore MQTT** per rispettare la convenzione `<ambiente>/comandi/` e `<ambiente>/config/` sul broker centrale, mappandoli al topic canonico ChirpStack

Ognuna di queste sarebbe una lezione a parte.

---

## Fonti immagini

Le immagini sono nella cartella `img/` accanto al file `.md`. Se una dovesse mancare o essere danneggiata, questi sono gli URL originali da cui sono state scaricate:

- **Figura 1** (`img/heltec_pinout.png`) — Pinout Heltec: `https://resource.heltec.cn/download/WiFi_LoRa32_V3/HTIT-WB32LA_V3.png` — Heltec Automation
- **Figura 2** (`img/bms_1s_5a.png`) — BMS 1S 5A per 18650 — immagine commerciale Amazon
- **Figura 3** (`img/scd41.png`) — Sensore SCD41: `https://sensirion.com/media/images/00/f5/94/1743502836/CO2-Sensor-SCD41.png/w-800.png` — Sensirion AG
- **Figura 4** (`img/nmea_format.png`) — Struttura sentenza NMEA: `https://www.hemispheregnss.com/wp-content/uploads/2020/02/NMEA-0183-Message-Format.png` — Hemisphere GNSS
- **Figura 5** (`img/lorawan_class_a.png`) — LoRaWAN classe A RX windows: `https://miro.medium.com/v2/resize:fit:1400/1*fpe80CIVX5PWTvLTBaBBWQ.png` — articolo Medium sulla LoRaWAN
- **Figura 6** (`img/esp32_sleep_modes.png`) — Modalità sleep ESP32: `https://lastminuteengineers.b-cdn.net/wp-content/uploads/iot/ESP32-Sleep-Modes-Comparison-Power-Consumption-Wake-up-Sources.png` — Last Minute Engineers
- **Figura 7** (`img/lorawan_architecture.png`) — Architettura LoRaWAN: `https://www.thethingsindustries.com/docs/img/technologies/lorawan/architecture.png` — The Things Industries

Se un'immagine dovesse smettere di essere disponibile, cerca su Google Immagini con le seguenti keyword:
- "Heltec WiFi LoRa 32 pinout"
- "SCD41 sensor Sensirion"
- "NMEA 0183 sentence format"
- "LoRaWAN class A downlink RX window"
- "ESP32 sleep modes power"
- "LoRaWAN architecture diagram"

---

## Codice sorgente

I file completi descritti in questa dispensa sono disponibili in:

- **Firmware Arduino**: [`FWs/heltec_v4_scd41_gps_lorawan.ino`](FWs/heltec_v4_scd41_gps_lorawan.ino)
- **Codec JavaScript per ChirpStack**: [`codecs/chirpstack_codec.js`](codecs/chirpstack_codec.js)
- **Script bash di test downlink**: [`tests/test_downlinks.sh`](tests/test_downlinks.sh)
