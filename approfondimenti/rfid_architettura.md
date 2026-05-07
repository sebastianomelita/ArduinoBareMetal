> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Architettura del sistema RFID**

Lo schema architetturale tipico di un sistema RFID si compone di **quattro strati**:

```
                            ┌──────────────────────────────┐
   Strato applicativo  →    │  ERP / WMS / MES / Web App   │
                            └─────────────┬────────────────┘
                                          │  REST / MQTT / EPCIS
                            ┌─────────────┴────────────────┐
   Strato middleware   →    │  Filtraggio - Deduplica -    │
                            │  Smoothing - Traduzione      │
                            └─────────────┬────────────────┘
                                          │  LLRP / proprietario
                            ┌─────────────┴────────────────┐
   Strato di lettura   →    │  Reader fissi e handheld     │
                            └─────────────┬────────────────┘
                                          │  RF (LF/HF/UHF)
                            ┌─────────────┴────────────────┐
   Strato fisico       →    │  Tag RFID sugli oggetti      │
                            └──────────────────────────────┘
```

## **Middleware RFID**

Le letture grezze prodotte dai reader sono **inutilizzabili direttamente** dalle applicazioni di business. Un tag fermo davanti a un'antenna può generare **migliaia di letture al secondo**, tutte identiche. Un'antenna può "vedere" un tag che è in realtà su un pallet adiacente. Un tag si può perdere e poi riapparire.

Per questo tra il **livello dei reader** e il **livello applicativo** (ERP, WMS, sistemi di tracciabilità) si interpone un **middleware RFID**, che svolge funzioni di:

- **Filtraggio**: scarta letture sotto una soglia di RSSI, o di antenne non interessanti per quell'evento.
- **Deduplica**: aggrega le centinaia di letture dello stesso tag in un **singolo evento** ("tag X visto al varco 1 alle 10:32:15").
- **Smoothing**: gestisce le **oscillazioni** dovute a riflessioni e angoli di lettura sfavorevoli (un tag visto, perso, riacquisito è in realtà sempre presente).
- **Correlazione con eventi esterni**: aggancia la lettura RFID al **trigger** della fotocellula del varco, allo **stato del PLC**, all'**ordine di lavoro** corrente.
- **Traduzione semantica**: converte l'**EPC binario** in un **identificativo di business** (codice articolo, lotto, scadenza) consultando il **database aziendale**.
- **Pubblicazione**: invia gli eventi raffinati al **broker MQTT**, al **server EPCIS** o direttamente all'**ERP** via API REST.

Il middleware può essere:

- **Embedded** nel reader stesso (i reader moderni hanno SoC Linux e permettono di eseguire script Python, Node.js o moduli Java direttamente a bordo).
- **On-edge** su un **gateway** dedicato collocato vicino ai reader (riduce traffico di rete e latenza).
- **Centralizzato** su un server di middleware (es. **Impinj ItemSense**, **Zebra MotionWorks**, **Mojix ViZix**, soluzioni open-source come RFID-Edge o LLRP-toolkit).

## **Semantica applicativa**

A differenza di **BLE**, dove la **semantica** dei dispositivi (lampade, termostati, pulsanti) è **standardizzata** dal protocollo fino al livello di **presentazione** della pila ISO/OSI, l'**RFID base** scambia esclusivamente **identificativi binari** (EPC, UID): è quindi un protocollo che si **ferma al livello 2** della pila ISO/OSI per quanto riguarda l'**accesso al tag**, e **non possiede** una semantica applicativa standard incorporata.

Si può quindi dire che:

- una **rete BLE** scambia **oggetti domotici strutturati** (lampada con stato on/off, termostato con setpoint, ecc.) il cui significato è noto allo stack del protocollo. È una rete **L7-aware**.
- una **rete RFID** scambia **codici identificativi** privi di significato applicativo intrinseco. Il "significato" dell'EPC `3035 0000 0000 1234 5678 ABCD` è dato da una **lookup table esterna** (database, ERP) che lo associa a un articolo specifico.
- la **traduzione semantica** è quindi un compito del **middleware**, non del protocollo radio.

### **Esempio di evento RFID di business**

Una lettura grezza di reader produce qualcosa come:

```
Reader: 192.168.10.5
Antenna: 2
EPC (hex): 3035 0000 4D2 0000 CE54 D2A0
RSSI: -52
Timestamp: 2025-09-12T10:32:15.234Z
```

Dopo il passaggio nel **middleware**, l'evento viene tradotto in un **oggetto JSON** che include la **semantica di business**:

```json
{
  "eventTime": "2025-09-12T10:32:15.234Z",
  "eventType": "ObjectEvent",
  "epc": "urn:epc:id:sgtin:0614141.012345.62852",
  "businessStep": "urn:epcglobal:cbv:bizstep:receiving",
  "disposition": "urn:epcglobal:cbv:disp:in_progress",
  "readPoint": "urn:epc:id:sgln:0614141.00012.1",
  "bizLocation": "urn:epc:id:sgln:0614141.00012.0",
  "product": {
    "gtin": "00614141123459",
    "name": "T-shirt mod. Aurora taglia M colore blu",
    "lot": "L20250903"
  },
  "reader": {
    "id": "reader-magazzino-varco1",
    "antenna": 2,
    "rssi": -52
  }
}
```

Questo è il messaggio che viene pubblicato sul **broker MQTT** o inoltrato all'**ERP**.

## **Gateway**

### **Ruolo del gateway**

In una architettura RFID il **gateway** ha lo stesso ruolo concettuale che ha in una rete BLE: è il **dispositivo di confine** tra la **rete di accesso RFID** (che usa protocolli specializzati come **LLRP**, oppure interfacce **seriali RS232/RS485** dei reader low-end) e la **rete di distribuzione IP** dove vivono il broker MQTT, l'ERP, il middleware centrale.

Le funzioni tipiche del gateway sono:

- **Inoltro** (routing/bridging) dei messaggi tra reader e middleware:
  * **diretto** verso un middleware in **LAN aziendale**, con il gateway che possiede un **IP privato**.
  * **indiretto** tramite **VPN** o tunnel **GRE** verso un middleware **cloud** o **HQ aziendale**, con il gateway che possiede un **IP pubblico** sull'interfaccia Internet e un **IP privato** sul tunnel cifrato.
- **Traduzione di formato**: da messaggi LLRP (binari, su TCP) a messaggi MQTT (testuali, JSON, su TCP). Questa è la funzione principale che differenzia il gateway RFID dal gateway BLE: invece di tradurre **GATT → JSON**, qui si traduce **LLRP → JSON**.
- **Interrogazione periodica** (polling) dei reader: il gateway può configurare i reader per eseguire inventory continui o triggerati da GPI (General Purpose Input).
- **Raccolta e bufferizzazione** delle letture: in caso di **caduta del link** verso il middleware, il gateway può accumulare le letture in una **coda locale persistente** e ritrasmetterle quando la connessione è ripristinata. Funzionalità **fondamentale** in scenari logistici dove la perdita di una lettura significa la perdita di tracciabilità di un oggetto.
- **Esecuzione di logica di business "edge"**: filtraggio, deduplica, smoothing — cioè parte delle funzioni del middleware vengono eseguite a bordo del gateway per ridurre il traffico verso il backend.
- **Protezione della rete RFID**: firewall, segmentazione VLAN. Importante perché i reader RFID, essendo dispositivi embedded poco aggiornati, sono spesso un **vettore di attacco** verso la rete aziendale.

Il **gateway** è uno **snodo nevralgico** dei messaggi RFID, per cui la sua posizione dovrebbe essere **ben nota** e accuratamente **riportata in planimetria** per permettere una sua rapida manutenzione/sostituzione.

### **Gateway come Client MQTT**

In **generale**, su reti **non IP** (RFID, BLE, LoRaWAN), i **client MQTT** (con il ruolo di **publisher** o di **subscriber**) sono sempre i **gateway di confine** della **rete di sensori** o di Auto-ID. Nel caso RFID, il **client MQTT** è quasi sempre **a bordo del reader** (i reader Linux moderni includono client MQTT) oppure **a bordo del gateway** dedicato.

Le **uniche reti RFID** che non hanno bisogno di un gateway esterno sono quelle composte da **reader IP nativi** (Ethernet/WiFi) con client MQTT integrato, che possono pubblicare direttamente sul broker.

### **Gateway come router L7**

Avendo più interfacce su reti di tipo diverso sia in L1 che in L2, il gateway ha anche le funzioni di **router** e, se la rete di distribuzione è pubblica come **Internet**, anche di **firewall**.

Ma la funzione **più importante** che possiede nel contesto di una rete RFID è la **traduzione semantica** dei messaggi: l'**EPC binario** letto dal tag viene tradotto in un **oggetto JSON** che descrive l'**evento di business**, comprensivo di metadati (timestamp, ID reader, antenna, RSSI) e, opzionalmente, della **denormalizzazione** dell'EPC nei dati di prodotto consultando un **database locale**.

A differenza del caso BLE — dove la semantica applicativa standard delle entità (lampade, sensori) è **definita dal protocollo** — nell'RFID la semantica deve essere **costruita esplicitamente** dall'integratore, perché lo standard radio si limita a trasportare un identificativo.

### **Formato dei messaggi**

I **messaggi** sono attualmente definiti sotto forma di **oggetti JSON** in formato ASCII. Questo dovrebbe garantire da un lato l'interoperabilità tra reader RFID di vendor diversi, dall'altro l'interoperabilità con sistemi terzi che si occupano della pubblicazione dei dati o della loro elaborazione statistica.

Gli oggetti JSON scambiati nella rete di distribuzione vanno **progettati** in modo tale da includere tutti i campi necessari ai sistemi consumer:

```json
{
  "eventTime": "2025-09-12T10:32:15.234Z",
  "epc": "urn:epc:id:sgtin:0614141.012345.62852",
  "reader": "reader-magazzino-varco1",
  "antenna": 2,
  "rssi": -52,
  "direction": "IN",
  "businessStep": "receiving"
}
```

Il campo `direction` è un esempio di **arricchimento semantico** prodotto dal middleware: confrontando le **letture sequenziali** sulle antenne 1 e 2 di un varco, il middleware deduce se il pallet è entrato o uscito dal magazzino. Questa informazione **non è presente** nelle letture grezze: è una **costruzione di alto livello** del middleware.

### **Esempio di gerarchia di topic MQTT**

Sulla scorta dell'esempio BLE, una gerarchia tipica per un sistema RFID multi-sito potrebbe essere:

```
azienda/
  └── sede_milano/
        ├── magazzino/
        │     ├── varco_ricezione/
        │     │     ├── letture
        │     │     ├── stato
        │     │     └── config
        │     └── varco_spedizione/
        │           ├── letture
        │           ├── stato
        │           └── config
        └── produzione/
              └── linea_1/
                    ├── ingresso_materiale/letture
                    └── uscita_prodotto/letture
```

I **topic** seguono il principio **gerarchico-spaziale**: il prefisso identifica la **localizzazione** dell'evento, il suffisso specifica il **tipo di informazione** (letture, stato del reader, configurazione).

#### **Sottoscrizione di un'applicazione di tracciabilità**

Un'applicazione che vuole ricevere **tutte le letture di tutti i varchi del magazzino** si sottoscrive con il wildcard `+`:

```
mosquitto_sub -h broker.azienda.it -t 'azienda/sede_milano/magazzino/+/letture'
```

Un'applicazione di **monitoring** che vuole sapere **lo stato di salute** di tutti i reader dell'azienda si sottoscrive con il wildcard `#`:

```
mosquitto_sub -h broker.azienda.it -t 'azienda/+/+/+/stato'
```

#### **Configurazione remota di un reader**

Un comando di configurazione (es. cambio della potenza di trasmissione) viene pubblicato dal sistema di gestione:

```bash
mosquitto_pub -h broker.azienda.it \
  -t 'azienda/sede_milano/magazzino/varco_ricezione/config' \
  -m '{"txPower": 27.0, "antennas": [1, 2], "session": 1, "target": "A"}'
```

Il reader (o il gateway che lo controlla) è sottoscritto a questo topic e applica i parametri al volo.

> [Torna alla dispensa principale RFID](../archrfid.md)
