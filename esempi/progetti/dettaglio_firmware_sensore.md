# Dettaglio — Firmware di un sensore LoRaWAN smart-road

> Documento di dettaglio collegato allo svolgimento principale (sezione §3.2.2).
> Schema semplificato del firmware di un end-device, a scopo didattico.
> Ispirato al pattern a fasi del materiale di riferimento `sebastianomelita/ArduinoBareMetal` (sensorfw.md), adattato dal caso WiFi/MQTT al caso LoRaWAN.

---

## Indice

- [1. Differenza rispetto al firmware MQTT classico](#1-differenza-rispetto-al-firmware-mqtt-classico)
- [2. Fasi principali del firmware](#2-fasi-principali-del-firmware)
- [3. Macchina a stati del sensore](#3-macchina-a-stati-del-sensore)
- [4. Pseudocodice commentato](#4-pseudocodice-commentato)
- [5. Esempio concreto in C++ (Arduino/LMIC)](#5-esempio-concreto-in-c-arduinolmic)
- [6. Gestione dell'energia](#6-gestione-dellenergia)
- [7. Formato del payload (Cayenne LPP)](#7-formato-del-payload-cayenne-lpp)

---

## 1. Differenza rispetto al firmware MQTT classico

Il materiale di riferimento descrive il firmware di un sensore che pubblica le misure **direttamente su un broker MQTT** via WiFi. Lo schema a fasi è: inizializza parametri → connetti al broker → leggi sensore → invia messaggio a intervalli → ripeti.

Nel nostro progetto smart-road il sensore **non parla MQTT né WiFi**: è un end-device LoRaWAN. La differenza fondamentale è che:

- Il sensore non ha uno stack TCP/IP né conosce l'indirizzo del broker. Trasmette in radio LoRa verso il gateway, che incapsula il messaggio e lo gira al network server. È **il network server** (non il sensore) che poi pubblica su MQTT.
- Il sensore non è "sempre connesso": per risparmiare batteria sta in **deep sleep** quasi tutto il tempo e si sveglia solo per trasmettere (classe A).
- Il payload non è JSON leggibile, ma un **formato binario compatto** (Cayenne LPP), perché la banda radio è limitatissima.

La struttura logica a fasi, però, **rimane la stessa** del materiale di riferimento: è questo il valore didattico del confronto. Cambia il mezzo trasmissivo, non l'impianto del firmware.

---

## 2. Fasi principali del firmware

1. **Inizializzazione dell'hardware**
   - Configurazione dei pin dei sensori (I²C/SPI/analogici per meteo, aria, fondo, ecc.).
   - Inizializzazione del modulo radio LoRa (es. Semtech SX1276).
   - Inizializzazione del gestore di alimentazione (deep sleep, watchdog).

2. **Provisioning / Join alla rete (OTAA)**
   - Caricamento delle credenziali pre-programmate in fabbrica: DevEUI, AppEUI, AppKey.
   - Invio della Join Request al network server tramite il gateway.
   - Ricezione della Join Accept e memorizzazione delle chiavi di sessione (AppSKey, NwkSKey).
   - Questa fase avviene **una sola volta** al primo avvio (o dopo un reset), non a ogni ciclo.

3. **Ciclo principale (a ogni risveglio dal deep sleep)**
   - Risveglio dal deep sleep (timer o evento esterno).
   - Lettura dei valori dai sensori.
   - Codifica del payload in formato compatto (Cayenne LPP).
   - Trasmissione dell'uplink LoRaWAN.
   - Apertura delle due finestre di ricezione (RX1, RX2) per eventuali downlink (comandi, ACK, aggiornamento ADR).
   - Gestione di un eventuale downlink ricevuto.

4. **Ritorno in deep sleep**
   - Spegnimento della radio e dei sensori.
   - Programmazione del timer di risveglio (es. 60 secondi).
   - Ingresso in deep sleep per minimizzare il consumo.

---

## 3. Macchina a stati del sensore

```
        ┌──────────────┐
        │  POWER ON /   │
        │    RESET      │
        └──────┬───────┘
               │
               ▼
        ┌──────────────┐
        │  INIT HW      │  configura sensori + radio + power mgmt
        └──────┬───────┘
               │
               ▼
        ┌──────────────┐      join fallito (retry con backoff)
        │  JOIN OTAA    │◄─────────────────┐
        └──────┬───────┘                   │
               │ join OK                    │
               ▼                            │
        ┌──────────────┐                   │
        │  READ SENSOR  │                   │
        └──────┬───────┘                   │
               │                            │
               ▼                            │
        ┌──────────────┐                   │
        │  ENCODE + TX  │  uplink LoRaWAN   │
        └──────┬───────┘                   │
               │                            │
               ▼                            │
        ┌──────────────┐                   │
        │  RX1 / RX2    │  ascolta downlink │
        └──────┬───────┘                   │
               │ (gestione comando)         │
               ▼                            │
        ┌──────────────┐                   │
        │  DEEP SLEEP   │  timer ~60 s      │
        └──────┬───────┘                   │
               │ risveglio                  │
               └────────────────────────────┘
                  (torna a READ SENSOR;
                   se sessione scaduta, torna a JOIN)
```

---

## 4. Pseudocodice commentato

```
// ===== CREDENZIALI OTAA (programmate in fabbrica) =====
DevEUI  = 00-80-00-FF-FF-00-2A-3F
AppEUI  = 8B-6C-F0-8E-EE-DF-1B-B6
AppKey  = <chiave segreta a 128 bit>

INTERVALLO_SLEEP = 60   // secondi

funzione setup():
    init_hardware()             // pin sensori, radio LoRa, power mgmt
    lora_init(classe = A)       // classe A = minimo consumo
    se non lora_join_OTAA(DevEUI, AppEUI, AppKey):
        riprova con backoff esponenziale   // 5s, 10s, 20s, ...

funzione loop():               // eseguito a ogni risveglio
    // -- lettura sensori --
    temp   = leggi_sensore_temperatura()
    umid   = leggi_sensore_umidita()
    pm10   = leggi_sensore_aria()
    vibr   = leggi_sensore_fondo()

    // -- codifica compatta --
    payload = cayenne_lpp_encode(temp, umid, pm10, vibr)

    // -- trasmissione uplink (cifrata con AppSKey, MIC con NwkSKey) --
    lora_send(payload, porta = 33)

    // -- finestre di ricezione downlink --
    downlink = lora_receive(RX1, RX2)
    se downlink presente:
        gestisci_comando(downlink)   // es. cambio intervallo, ADR

    // -- risparmio energetico --
    radio_sleep()
    sensori_sleep()
    deep_sleep(INTERVALLO_SLEEP)     // il risveglio fa ripartire loop()
```

Nota sulla differenza con il firmware MQTT del materiale di riferimento: lì il ciclo principale usa `client.publish(topic, message)` su una connessione TCP persistente, e il microcontrollore resta sempre acceso. Qui `lora_send()` trasmette un singolo pacchetto radio e poi il dispositivo torna immediatamente in deep sleep: è questa la chiave dell'autonomia pluriennale a batteria.

---

## 5. Esempio concreto in C++ (Arduino/LMIC)

Schema semplificato basato sulla libreria **MCCI LoRaWAN LMIC** (la più diffusa per end-device Arduino/ESP32). Volutamente ridotto all'essenziale didattico; in un firmware reale ci sarebbero più gestioni di errore.

```cpp
#include <lmic.h>
#include <hal/hal.h>
#include <CayenneLPP.h>

// ---- Credenziali OTAA (LSB/MSB secondo le convenzioni LMIC) ----
static const u1_t PROGMEM DEVEUI[8]  = { /* ... */ };
static const u1_t PROGMEM APPEUI[8]  = { /* ... */ };
static const u1_t PROGMEM APPKEY[16] = { /* ... */ };
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

CayenneLPP lpp(51);                 // buffer payload compatto
const unsigned TX_INTERVAL = 60;    // secondi
static osjob_t sendjob;

// Lettura sensori (semplificata)
float readTemp() { /* I2C/SPI ... */ return 18.5; }
float readHum()  { return 67.0; }
float readPM10() { return 12.0; }

void do_send(osjob_t* j) {
  lpp.reset();
  lpp.addTemperature(1, readTemp());   // canale 1
  lpp.addRelativeHumidity(2, readHum()); // canale 2
  lpp.addAnalogInput(3, readPM10());   // canale 3
  // accoda l'uplink: LMIC cifra con AppSKey e calcola il MIC con NwkSKey
  LMIC_setTxData2(33, lpp.getBuffer(), lpp.getSize(), 0);
}

void onEvent(ev_t ev) {
  switch (ev) {
    case EV_JOINED:
      // join OTAA completato: sessione attiva
      LMIC_setLinkCheckMode(0);   // disabilita link check per risparmio
      break;
    case EV_TXCOMPLETE:
      // uplink inviato + finestre RX chiuse -> pianifica il prossimo invio
      os_setTimedCallback(&sendjob,
        os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      // (su hardware a batteria qui si entrerebbe in deep sleep)
      break;
    default: break;
  }
}

void setup() {
  os_init();
  LMIC_reset();
  do_send(&sendjob);   // avvia join + primo invio
}

void loop() {
  os_runloop_once();   // motore a eventi di LMIC
}
```

Punti didattici da evidenziare in sede d'esame:

- `LMIC_setTxData2(...)` è il punto in cui avviene la **cifratura AES con AppSKey** e il **calcolo del MIC con NwkSKey** — tutto gestito dalla libreria, ma è bene sapere che è lì che accade.
- L'evento `EV_JOINED` segna il completamento dell'**OTAA**: prima di quel momento il sensore non può trasmettere dati.
- L'evento `EV_TXCOMPLETE` arriva **dopo** che si sono chiuse le due finestre di ricezione: solo allora è sicuro tornare in sleep.
- Il canale (`1`, `2`, `3`) nel Cayenne LPP identifica quale grandezza si sta trasmettendo, permettendo al decoder lato server di ricostruire il JSON.

---

## 6. Gestione dell'energia

È l'aspetto che distingue un firmware LoRaWAN da uno WiFi/MQTT. Tecniche chiave:

- **Deep sleep tra una trasmissione e l'altra**: il microcontrollore (es. ESP32) scende a pochi µA di consumo. La radio si spegne completamente.
- **Classe A**: le finestre di ricezione si aprono solo dopo un uplink, quindi il sensore non deve mai stare in ascolto continuo.
- **Adaptive Data Rate (ADR)**: il network server, vedendo l'ottimo RSSI/SNR dei sensori vicini al gateway, comanda loro di usare uno spreading factor basso (trasmissioni più brevi → meno energia). Il firmware deve solo rispettare i parametri ricevuti.
- **Trasmissioni rade**: una misura al minuto è già abbondante per dati ambientali; per il fondo stradale si può salire a una ogni 10 s solo se serve.
- **Energy harvesting**: il pannellino fotovoltaico ricarica la batteria tampone; il firmware può leggere il livello di carica e, se basso, **ridurre la frequenza di trasmissione** automaticamente (graceful degradation energetica).

---

## 7. Formato del payload (Cayenne LPP)

Cayenne Low Power Payload è un formato standard compatto per LoRaWAN. Ogni misura è codificata come: `[canale][tipo][valore]`. Esempio per temperatura + umidità + PM10:

| Byte | Significato |
|------|-------------|
| `01` | canale 1 |
| `67` | tipo = temperatura |
| `00 B9` | valore = 18.5 °C (0.1 °C/LSB → 185) |
| `02` | canale 2 |
| `68` | tipo = umidità relativa |
| `86` | valore = 67% (0.5%/LSB → 134) |
| `03` | canale 3 |
| `02` | tipo = analog input |
| `00 78` | valore = 12.0 (0.01/LSB → 1200) |

Il messaggio totale occupa pochi byte (qui 11), contro le decine o centinaia di un JSON equivalente. Lato network server, un **decoder Cayenne LPP** ricostruisce il JSON leggibile che poi viaggia su MQTT verso le dashboard:

```json
{
  "channel_1": { "temperature": 18.5 },
  "channel_2": { "humidity": 67 },
  "channel_3": { "analog_in": 12.0 }
}
```

È il punto in cui il dato "compatto radio" diventa "dato leggibile IP", e coincide con il confine LoRaWAN/IP discusso nello svolgimento principale (§3.2.5).
