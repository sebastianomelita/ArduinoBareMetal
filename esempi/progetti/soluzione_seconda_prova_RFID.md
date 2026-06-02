# SIMULAZIONE SECONDA PROVA — SISTEMI E RETI
**Indirizzo:** Informatica e Telecomunicazioni — Articolazione Informatica  
**Disciplina:** Sistemi e Reti  
**Anno scolastico:** 2025/2026

---

# PRIMA PARTE — Soluzione

## 1. Analisi del sistema e ipotesi aggiuntive

Il sistema da progettare è un'infrastruttura metropolitana per la gestione di card di trasporto pubblico basata su tecnologia **NFC/RFID HF** (ISO 14443, 13,56 MHz), analoga alle soluzioni adottate da Oyster Card (Londra), Navigo (Parigi) e Carta Mia (Torino). Le card sono di tipo **MIFARE DESFire EV3** (o equivalente) per garantire autenticazione crittografica e resistenza alla clonazione.

**Ipotesi aggiuntive:**
- La card fisica e l'APP per smartphone adottano entrambe lo standard **NFC ISO 14443-4**.
- Il **Servizio Unico Metropolitano (SUM)** è un'infrastruttura cloud/datacenter centralizzata gestita dalla società partecipata.
- I controllori utilizzano smartphone o tablet con app dedicata che accede via HTTPS al server della sede.
- Si stimano, a livello metropolitano, circa **500 fermate/stazioni** (luoghi di imbarco/sbarco).
- Le **stazioni ad alta frequenza** (Categoria A) sono circa 80; le **fermate a bassa frequenza** (Categoria B) sono circa 420.
- Il sistema opera in **tempo reale** con latenza massima accettabile di 2 secondi per il feedback al viaggiatore.

---

## 2. Architettura della rete e dei sistemi di elaborazione

### 2.1 Schema generale (tre livelli)

```
┌─────────────────────────────────────────────────────────────────────┐
│                   SERVIZIO UNICO METROPOLITANO (SUM)                │
│   [DB Centrale] ─── [App Server] ─── [Broker MQTT] ─── [API GW]    │
│         ↑ VPN/TLS                           ↑ TLS/MQTT              │
└─────────────────────────────────────────────────────────────────────┘
              ↑                                       ↑
     ┌────────┴────────┐                   ┌──────────┴──────────┐
     │  CAT. A — LAN   │                   │  CAT. B — WAN diretta│
     │  locale + GW    │                   │  (≥2 reader per sito)│
     └────────┬────────┘                   └──────────┬──────────┘
              ↑                                       ↑
        [Reader NFC]                           [Reader NFC x2]
```

### 2.2 Categoria A — Luoghi ad alta frequenza (stazioni treni, metro principali, pontili capolinea)

**Architettura locale:**

- **Switch PoE** (es. Cisco SG350-10P) per alimentare e connettere i reader NFC via cavo Ethernet.
- **Server locale edge** (es. mini-server con CPU multi-core, 16 GB RAM, SSD) che esegue il middleware RFID: gestione delle sessioni di viaggio, cache del saldo card, sincronizzazione asincrona con SUM.
- I reader si collegano al server locale via **LAN Ethernet 1 Gbps**.
- Il server locale si connette al SUM tramite **fibra ottica** (connessione dedicata o MPLS) con **link ridondato** (linea primaria + fallback 4G/5G).
- Il display e il buzzer di ogni reader sono gestiti direttamente dal reader stesso sulla base della risposta del server locale (< 200 ms).

**Motivazione:** la presenza del server locale riduce la latenza del feedback al viaggiatore (la risposta non deve fare il giro fino al SUM centrale), ottimizza la banda verso il SUM (trasmette eventi aggregati) e garantisce continuità di servizio anche in caso di interruzione temporanea del link WAN.

### 2.3 Categoria B — Fermate a bassa frequenza (metro secondarie, fermate tram/bus, pontili intermedi)

**Architettura:**

- **Minimo 2 reader NFC** per fermata (ridondanza fisica richiesta dal capitolato).
- I reader si connettono **direttamente al SUM** tramite la rete WAN (4G/5G LTE o fibra residenziale con backup 4G).
- Non è presente un server locale: la logica applicativa risiede interamente nel SUM.
- Ogni reader dispone di **memoria cache locale** (storage flash integrato) per bufferizzare le transazioni in caso di disconnessione temporanea e inviarle al SUM al ripristino del link.

**Motivazione:** il traffico di queste fermate è inferiore e non giustifica l'investimento in un server locale. La ridondanza dei reader garantisce comunque continuità operativa in caso di guasto di un singolo dispositivo.

---

## 3. Tecnologie e modalità di comunicazione

### 3.1 Interfaccia card ↔ reader

| Parametro | Valore |
|-----------|--------|
| Standard | ISO 14443-4 (NFC Type A/B) |
| Frequenza | 13,56 MHz (RFID HF) |
| Distanza operativa | 0–10 cm |
| Velocità dati | 106–848 kbit/s |
| Crittografia | AES-128 (MIFARE DESFire EV3) |
| Autenticazione | Mutua (card + reader) con chiavi diversificate per UID |

**Motivazione della scelta NFC HF rispetto a UHF:** la portata corta (intenzionale) garantisce che solo la card/smartphone deliberatamente esposto venga letto, evitando false letture in contesti affollati. NFC è lo standard de facto per pagamenti e biglietteria, con ampio supporto in tutti gli smartphone moderni.

### 3.2 Comunicazione reader ↔ server locale (Cat. A)

- Protocollo fisico: **Ethernet 100BASE-TX** (cavo) su LAN locale.
- Protocollo applicativo: **MQTT** (broker sul server locale) con QoS 1 per garantire la consegna almeno una volta.
- I reader pubblicano eventi di esposizione card sul topic `stazione/<id_stazione>/reader/<id_reader>/eventi`.

### 3.3 Comunicazione verso il SUM

- **Cat. A:** il server locale pubblica eventi aggregati via **MQTT over TLS 1.3** su connessione WAN (fibra MPLS o VPN IPsec).
- **Cat. B:** i reader pubblicano direttamente via **MQTT over TLS 1.3** su connessione 4G/5G.
- Il SUM espone un **broker MQTT centrale** (es. EMQX o HiveMQ in cluster) raggiungibile su porta 8883 (TLS).

### 3.4 Comunicazione controllori ↔ server sede società

- I controllori usano app su smartphone/tablet connessi alla rete pubblica (4G/5G o WiFi).
- La comunicazione avviene tramite **HTTPS (TLS 1.3)** verso le **REST API** del server della sede.
- Autenticazione tramite **JWT (JSON Web Token)** con scadenza breve + refresh token.
- Il server espone un endpoint `/api/card/{uid}/stato` che restituisce in tempo reale lo stato della card.

---

## 4. Piano di indirizzamento

### 4.1 Schema generale degli indirizzi

Si adotta lo schema **RFC 1918** con subnetting VLSM. Spazio privato utilizzato: `10.0.0.0/8`.

| Contesto | Rete | Subnet mask | Range host | Gateway | Note |
|----------|------|-------------|------------|---------|------|
| SUM — DMZ pubblica | 10.0.1.0/28 | /28 | .1–.14 | 10.0.1.1 | Broker MQTT, API Gateway |
| SUM — server farm interna | 10.0.2.0/24 | /24 | .1–.254 | 10.0.2.1 | DB, App Server, broker interno |
| SUM — gestione/admin | 10.0.3.0/28 | /28 | .1–.14 | 10.0.3.1 | Accesso amministratori |
| **Cat. A — Stazione 1** | 10.1.1.0/26 | /26 | .1–.62 | 10.1.1.1 | Fino a 61 reader + server locale |
| **Cat. A — Stazione 2** | 10.1.2.0/26 | /26 | .1–.62 | 10.1.2.1 | |
| **Cat. A — Stazione N** | 10.1.N.0/26 | /26 | .1–.62 | 10.1.N.1 | (fino a N=80) |
| **Cat. B — Fermata 1** | 10.2.1.0/29 | /29 | .1–.6 | 10.2.1.1 | 2–6 reader, no server locale |
| **Cat. B — Fermata 2** | 10.2.2.0/29 | /29 | .1–.6 | 10.2.2.1 | |
| **Cat. B — Fermata M** | 10.2.M.0/29 | /29 | .1–.6 | 10.2.M.1 | (fino a M=420) |
| Sede società partecipata | 10.3.0.0/24 | /24 | .1–.254 | 10.3.0.1 | Server accesso controllori |
| Tunnel VPN Cat. A → SUM | 10.255.1.0/24 | /24 | — | — | Interfacce tunnel IPsec |

### 4.2 Esempio dettagliato — Stazione A "Centrale FS" (Cat. A)

| Dispositivo | Indirizzo IP | Note |
|-------------|-------------|------|
| Server locale edge | 10.1.1.1 | Gateway della subnet, broker MQTT locale |
| Switch PoE | 10.1.1.2 | Gestione via SNMP |
| Router/firewall | 10.1.1.3 | Interfaccia LAN; WAN: IP pubblico assegnato da ISP |
| Reader NFC #01 | 10.1.1.10 | Ingresso banchina 1 |
| Reader NFC #02 | 10.1.1.11 | Ingresso banchina 2 |
| Reader NFC #03 | 10.1.1.12 | Uscita banchina 1 |
| … | 10.1.1.13–.40 | Altri reader |
| Access Point WiFi | 10.1.1.50 | Per manutenzione tecnici |

### 4.3 Esempio dettagliato — Fermata B "Via Roma" (Cat. B)

| Dispositivo | Indirizzo IP | Note |
|-------------|-------------|------|
| Reader NFC #01 | 10.2.5.2 | Connessione 4G integrata, DHCP da SIM |
| Reader NFC #02 | 10.2.5.3 | Reader di ridondanza, DHCP da SIM |
| Gateway SIM | 10.2.5.1 | Assegnato dal provider mobile (via NAT) |

> **Nota:** per le fermata Cat. B con connessione 4G/5G, l'indirizzamento pubblico è gestito dall'operatore mobile con NAT; i reader stabiliscono connessioni MQTT outbound verso il SUM (non serve IP pubblico statico in ingresso).

---

## 5. Configurazione dei dispositivi di rete

### 5.1 Router/Firewall stazione Cat. A (es. MikroTik o Cisco ISR)

```
# Interfaccia LAN verso switch
interface eth0
  ip address 10.1.1.3/26
  description "LAN stazione"

# Interfaccia WAN (fibra MPLS / connessione ISP)
interface eth1
  ip address <IP_PUBBLICO>/30   # assegnato da ISP
  description "WAN primaria fibra"

# Interfaccia WAN backup (4G)
interface lte0
  description "WAN backup 4G"

# Route di default su WAN primaria con failover
ip route 0.0.0.0/0 <GW_FIBRA> metric 10
ip route 0.0.0.0/0 <GW_4G>    metric 100   # failover

# Tunnel VPN IPsec verso SUM
ipsec tunnel to-SUM
  local-address  <IP_PUBBLICO>
  remote-address <IP_SUM>
  authentication pre-shared-key <PSK>
  encryption     AES-256-GCM
  network local  10.1.1.0/26
  network remote 10.0.0.0/16

# ACL — firewall
ip firewall filter
  chain input
    accept protocol tcp dst-port 22 src-address 10.0.3.0/28   # SSH solo da admin SUM
    accept protocol tcp dst-port 1883 src-address 10.1.1.0/26  # MQTT interno
    drop all  # blocco implicito

  chain forward
    accept in-interface eth0 out-interface ipsec-tunnel-to-SUM
    accept in-interface ipsec-tunnel-to-SUM out-interface eth0
    drop all
```

### 5.2 Server locale edge — Cat. A (configurazione broker MQTT locale)

```yaml
# mosquitto.conf (broker MQTT locale)
listener 1883 10.1.1.1
allow_anonymous false
password_file /etc/mosquitto/passwd

listener 8883 10.1.1.1
cafile  /etc/ssl/ca.crt
certfile /etc/ssl/server.crt
keyfile  /etc/ssl/server.key
tls_version tlsv1.3
require_certificate true

# Bridge verso SUM centrale
connection bridge-to-SUM
  address broker.sum-metropolitano.it:8883
  topic stazione/# out 1
  topic stazione/# in 1
  bridge_cafile   /etc/ssl/ca.crt
  bridge_certfile /etc/ssl/bridge.crt
  bridge_keyfile  /etc/ssl/bridge.key
```

### 5.3 Reader NFC Cat. B — configurazione (pseudocodice)

```
# Parametri di connessione reader fermata B
MQTT_BROKER   = "broker.sum-metropolitano.it"
MQTT_PORT     = 8883
MQTT_TLS      = true
MQTT_CERT     = "/certs/reader-b.crt"
MQTT_KEY      = "/certs/reader-b.key"
MQTT_CA       = "/certs/ca.crt"
MQTT_QOS      = 1
MQTT_TOPIC    = "fermata/<id_fermata>/reader/<id_reader>/eventi"
BUFFER_LOCAL  = "/data/buffer.json"   # flash storage per offline buffering
```

---

## 6. Continuità di servizio e sicurezza

### 6.1 Continuità di servizio

**Ridondanza dei link WAN (Cat. A):**
- Connessione primaria in fibra + connessione backup 4G/5G attivata automaticamente in caso di guasto (failover con metric routing).
- Il server locale mantiene una **cache del saldo** degli ultimi N viaggiatori (sliding window), garantendo il funzionamento anche senza link WAN per un breve periodo.

**Ridondanza fisica (Cat. B):**
- Almeno 2 reader per fermata: se uno si guasta, l'altro continua ad operare.
- Ogni reader dispone di buffer flash locale: le transazioni vengono memorizzate offline e sincronizzate al ripristino del link.

**Alta disponibilità del SUM:**
- Broker MQTT in cluster (almeno 2 nodi in active-active).
- Database centralizzato in configurazione **master-slave** con replicazione sincrona.
- Deploy su datacenter con certificazione **Tier III** (uptime ≥ 99,982%).

**Gestione dei duplicati:**
- Ogni evento di esposizione contiene un **UUID** univoco generato dal reader; il SUM deduplica le transazioni con lo stesso UUID entro una finestra temporale di 60 secondi.

### 6.2 Sicurezza dell'infrastruttura

**Sicurezza fisica dei reader:**
- Involucro antimanomissione (tamper-evident) con sensore di apertura.
- Monitoraggio in tempo reale dello stato dei reader via SNMP/MQTT.

**Sicurezza della comunicazione:**
- Tutto il traffico verso il SUM transita su **TLS 1.3** (diretto) o **VPN IPsec AES-256** (per Cat. A).
- I reader si autenticano al broker tramite **certificati X.509** rilasciati dall'autorità di certificazione interna della società partecipata.
- I certificati hanno validità di 1 anno e vengono rinnovati automaticamente via SCEP o ACME interno.

**Sicurezza della card:**
- La card adotta **autenticazione mutua AES-128** (standard MIFARE DESFire EV3): sia la card autentica il reader, sia il reader autentica la card, prevenendo la clonazione e l'uso con reader non autorizzati.
- Le chiavi crittografiche sono **diversificate per UID**: ogni card ha chiavi uniche derivate dalla chiave master della società tramite KDF (Key Diversification Function), così la compromissione di una card non mette a rischio le altre.
- I dati sulla card sono **cifrati**: saldo, storico recente e identificativo utente non sono leggibili in chiaro.

**Protezione del SUM:**
- **Firewall perimetrale** con regole whitelistiche (allowlist-only) sulle porte 8883 (MQTT/TLS) e 443 (HTTPS API).
- **IDS/IPS** (es. Suricata) per il rilevamento di attacchi DDoS e tentativi di accesso anomali.
- **WAF (Web Application Firewall)** davanti alle REST API dei controllori.
- Separazione in zone DMZ (broker MQTT, API pubblica) e zona interna (database, app server).
- Log centralizzato (SIEM) con retention di 12 mesi per audit e conformità GDPR.

**Privacy e GDPR:**
- I dati di spostamento degli utenti sono **pseudonimizzati** nel log: l'associazione tra UID della card e identità anagrafica è disponibile solo nel DB della società, non nei log operativi dei reader.
- Conformità al Regolamento EU 2016/679 (GDPR): diritto alla cancellazione dei dati di mobilità su richiesta dell'utente.

---

# SECONDA PARTE — Quesiti scelti

## Quesito 1 — Progettazione logica del database dei reader

### Entità e attributi

```
READER(_idReader_, macAddress, modello, firmware, dataInstallazione, stato, ipAddress)
SITO(_idSito_, nome, tipo, indirizzo, lat, lon, categoria)
    tipo ∈ {stazione_treno, stazione_metro, fermata_tram, fermata_bus, pontile}
    categoria ∈ {A, B}
STAZIONE_LOCALE(_idStazione_, idSito, ipServerLocale, nomeServer)
    idSito → SITO.idSito   [1:1, solo per siti Cat. A]
LINEA(_idLinea_, nome, mezzo, gestore)
    mezzo ∈ {treno, metro, tram, bus, imbarcazione}
SERVIZIO_MANUTENZIONE(_idServizio_, idReader, dataOra, tipo, descrizione, tecnico, esito)
    tipo ∈ {preventiva, correttiva, sostituzione}
```

### Associazioni

```
READER ────── SITO           [N:1]  ogni reader è installato in un sito
READER ────── STAZIONE_LOCALE [N:1] ogni reader Cat. A è connesso a un server locale
                                    (NULL per Cat. B)
SITO   ────── LINEA           [N:M] un sito serve più linee; una linea passa per più siti
              └─ SITO_LINEA(_idSito_, _idLinea_, direzione)
READER ────── SERVIZIO_MANUTENZIONE [1:N]
```

### Schema relazionale (modello logico)

```sql
READER(idReader PK, macAddress UNIQUE, modello, firmware, dataInstallazione,
       stato CHECK(stato IN ('attivo','guasto','manutenzione')),
       ipAddress, idSito FK→SITO, idStazioneLocale FK→STAZIONE_LOCALE NULLABLE)

SITO(idSito PK, nome, tipo, indirizzo, lat DECIMAL(9,6), lon DECIMAL(9,6),
     categoria CHECK(categoria IN ('A','B')))

STAZIONE_LOCALE(idStazione PK, idSito FK→SITO UNIQUE, ipServerLocale, nomeServer)

LINEA(idLinea PK, nome, mezzo, gestore)

SITO_LINEA(idSito FK→SITO, idLinea FK→LINEA, direzione, PRIMARY KEY(idSito, idLinea))

SERVIZIO_MANUTENZIONE(idServizio PK, idReader FK→READER, dataOra DATETIME,
                      tipo, descrizione TEXT, tecnico, esito)
```

---

## Quesito 2 — Protocollo applicativo per l'interazione con il SUM

### 2.1 Scelta del protocollo di trasporto

Si adotta **MQTT over TLS 1.3** come protocollo di comunicazione tra reader/server locale e SUM, per le seguenti ragioni:
- è **leggero** (overhead minimo, adatto a dispositivi embedded IoT);
- supporta **QoS 1** (almeno una consegna), garantendo che nessuna transazione venga persa;
- il modello **publish/subscribe** è adatto a un'architettura con molti produttori (reader) e un consumatore centrale (SUM);
- gestisce nativamente la **persistenza dei messaggi** e la riconnessione automatica.

### 2.2 Gerarchia dei topic MQTT

```
sum/sito/<idSito>/reader/<idReader>/tap        ← evento di esposizione card
sum/sito/<idSito>/reader/<idReader>/stato      ← heartbeat/stato del reader
sum/sito/<idSito>/reader/<idReader>/ack        ← risposta SUM → reader (feedback)
sum/sito/<idSito>/config                       ← configurazione inviata dal SUM
sum/controllori/<idControllore>/verifica        ← richiesta verifica card controllore
sum/controllori/<idControllore>/risposta        ← risposta a controllore
```

### 2.3 Formato dei messaggi JSON

**Evento di esposizione card (tap) — reader → SUM:**
```json
{
  "eventId":   "3f8a2c1d-4b5e-4a7f-8c9d-0e1f2a3b4c5d",
  "eventType": "tap",
  "timestamp": "2026-06-02T08:34:12.045Z",
  "idReader":  "READER-CAT-B-0042",
  "idSito":    "SITO-METRO-022",
  "uid":       "04:A3:B2:C1:D0:E9:F8",
  "rssi":      -35
}
```

**Risposta SUM al reader (ack) — SUM → reader:**
```json
{
  "eventId":    "3f8a2c1d-4b5e-4a7f-8c9d-0e1f2a3b4c5d",
  "timestamp":  "2026-06-02T08:34:12.182Z",
  "esito":      "OK",
  "tipoEvento": "inizio_viaggio",
  "saldoResiduo": 18.40,
  "messaggio":  "Buon viaggio! Saldo: €18,40"
}
```
> `esito` può essere: `OK`, `ERRORE_CARD_DISABILITATA`, `ERRORE_SALDO_INSUFFICIENTE`, `ERRORE_CARD_SCONOSCIUTA`

**Heartbeat reader → SUM (ogni 60 secondi):**
```json
{
  "eventType":  "heartbeat",
  "timestamp":  "2026-06-02T08:35:00.000Z",
  "idReader":   "READER-CAT-B-0042",
  "stato":      "operativo",
  "tapCount24h": 342,
  "bufferPending": 0
}
```

**Richiesta verifica card da controllore:**
```json
{
  "requestId":  "ctrl-abc-001",
  "timestamp":  "2026-06-02T09:10:00.000Z",
  "uid":        "04:A3:B2:C1:D0:E9:F8",
  "idControllore": "CTR-042",
  "mezzo":      "autobus",
  "linea":      "27"
}
```

**Risposta SUM al controllore:**
```json
{
  "requestId":   "ctrl-abc-001",
  "timestamp":   "2026-06-02T09:10:00.312Z",
  "statoCard":   "valida",
  "ultimoTap":   "2026-06-02T08:34:12.045Z",
  "sito":        "Fermata Via Roma",
  "saldoResiduo": 18.40
}
```

### 2.4 Pseudocodice del firmware del reader (Cat. B)

```
# Inizializzazione
connetti_MQTT(broker=SUM, porta=8883, tls=true, cert=reader.crt)
sottoscrivi(topic="sum/sito/{idSito}/reader/{idReader}/ack")

# Loop principale
loop forever:
    uid, rssi = attendi_tap_NFC(timeout=100ms)
    if uid != null:
        evento = crea_json_tap(uid, rssi, timestamp_now())
        if connessione_MQTT_ok():
            pubblica(topic="sum/sito/{idSito}/reader/{idReader}/tap",
                     payload=evento, qos=1)
            ack = attendi_risposta(timeout=2s)
            if ack != null:
                mostra_feedback(ack.esito, ack.saldoResiduo, ack.messaggio)
            else:
                mostra_errore("Timeout server — riprovare")
        else:
            buffer_locale.append(evento)
            mostra_errore("Offline — transazione memorizzata")

    # Flush buffer offline
    if connessione_MQTT_ok() and buffer_locale.non_vuoto():
        for ev in buffer_locale:
            pubblica(topic_tap, ev, qos=1)
        buffer_locale.svuota()

    # Heartbeat ogni 60s
    if elapsed(ultimo_heartbeat) > 60s:
        pubblica(topic_heartbeat, crea_heartbeat(), qos=0)
        ultimo_heartbeat = now()
```

---

> **Nota sulla dispensa di riferimento:** la soluzione è stata sviluppata seguendo le indicazioni metodologiche della dispensa [Tecnologie RFID](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archrfid.md), in particolare:
> - [Architettura del sistema, middleware, gateway, MQTT](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/approfondimenti/rfid_architettura.md)
> - [Sicurezza e privacy RFID](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/approfondimenti/rfid_sicurezza.md)
> - [Frequenze di lavoro e NFC](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/approfondimenti/rfid_frequenze.md)
> - [Standard RFID (ISO 14443, NFC)](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/approfondimenti/rfid_standard.md)
> - [Esempio di traccia svolta](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/approfondimenti/rfid_seconda_prova.md)
