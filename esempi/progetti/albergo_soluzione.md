# 11 – GESTIONE ALBERGO (IoT indoor + NAT)
## Soluzione completa d'esame

---

## 1. Ipotesi aggiuntive e dimensionamento

L'albergo è un edificio di **4 piani** (piano terra + 3 piani residenziali), con le seguenti caratteristiche quantitative stimate:

| Parametro | Valore |
|-----------|--------|
| Stanze residenziali | 36 (12 per piano × 3 piani) |
| Ospiti massimi simultanei | 80 persone (2 per stanza, tasso occupazione 110%) |
| Addetti alle pulizie con Wi-Fi | 8 terminali |
| Addetti ricevimento | 5 postazioni cablate |
| Indirizzi IP pubblici | 4 (NAT dinamico per ospiti + servizi esposti) |
| Gateway Zigbee | 4 (1 per piano + 1 piano terra/reception) |
| Access Point Wi-Fi 802.11ac | 10 (2 per piano + 2 piano terra) |
| Dispositivi Zigbee per stanza | 6 (lampada FFD, persiana RFD, condizionatore FFD, serratura RFD, comando comodino RFD, sensore T/H RFD) |
| Dispositivi Zigbee aree comuni | ~30 (luci corridoi FFD, portoni, giardino, garage, palestra, ristorante) |

**Stima totale nodi Zigbee:** 36 × 6 + 30 = **246 dispositivi** distribuiti su 4 reti Zigbee (una per piano/zona).

**Dimensionamento router Zigbee:** Le **lampade alimentate a rete** (FFD) fungono contemporaneamente da attuatori e da router della mesh Zigbee. Con mediamente 3 lampade per stanza e 6 per corridoio, si stimano ~150 router FFD e ~100 end device RFD a batteria.

---

## 2. Scelta Architetturale: Gateway L7 Zigbee (Scenario A)

Il confronto tra i due scenari richiesti è illustrato nella figura seguente.

![Confronto architetture Gateway L7 Zigbee vs Gateway L3 Thread/Matter](../img/arch_l7_vs_l3.svg)

### 2.1 Perché si sceglie il Gateway L7 con Zigbee

**Non si sceglie** il Gateway L3 con Thread/Matter per i seguenti motivi:

1. **Ecosistema maturo**: Zigbee ha un catalogo vastissimo di dispositivi interoperabili (lampade, serrature, termostati, sensori), già ampiamente testati in ambito alberghiero.
2. **Tool chain open-source affermata**: `zigbee2mqtt` è software maturo, ben documentato, che realizza la traduzione semantica ZCL→JSON in modo automatico per centinaia di dispositivi.
3. **Rete di sensori indipendente dall'IP**: la mesh Zigbee funziona anche se la rete LAN ha problemi. I comandi locali (interruttore→lampada) continuano a operare tramite binding diretto.
4. **Batterie longeve**: i telecomandi/pulsanti RFD a batteria durano anni grazie alla modalità sleep mode e al multi-hop verso il primo router FFD vicino.
5. **Compatibilità con MQTT on-premise**: il gateway Zigbee pubblica direttamente sul broker Mosquitto locale, senza dipendere da cloud proprietari. Questo garantisce pieno controllo dei dati, bassa latenza e continuità operativa in assenza di Internet.

**Thread/Matter sarebbe preferibile** solo in scenari con forte standardizzazione futura (nuovo edificio, budget elevato, vendor unico), poiché il livello L3 comune IPv6 elimina la traduzione semantica. Tuttavia la maturità commerciale attuale (2025-2026) rende ancora rischioso affidarsi a Thread per un'infrastruttura alberghiera con 250 dispositivi di produttori diversi. In particolare, **dashboard e comandi web** richiederebbero comunque un connettore Matter→MQTT/REST aggiuntivo (Node-RED o Home Assistant), annullando il vantaggio del gateway "solo L3".

### 2.2 Broker MQTT on-premise: motivazione

Si sceglie un **broker MQTT on-premise** (Mosquitto su server dedicato in sala tecnica) invece di un broker cloud per:

- **Latenza minima**: i comandi domotica (es. accensione luci) raggiungono i gateway in < 10 ms sulla LAN locale.
- **Continuità operativa**: un'interruzione di Internet non compromette la domotica interna.
- **Privacy dei dati**: le prenotazioni e i log di accesso alle stanze non lasciano l'edificio.
- **Costo**: nessun abbonamento mensile a piattaforme cloud IoT.
- **Sicurezza**: il broker è raggiungibile dall'esterno solo tramite VPN o tunnel autenticato, non esposto direttamente su Internet.

---

## 3. Planimetria e posizionamento dispositivi

![Planimetria albergo con posizionamento dispositivi IoT](../img/planimetria.svg)

**Note progettuali:**

- I **gateway Zigbee** (GW-RT, GW-P1, GW-P2, GW-P3) sono posizionati in rack tecnici vicino al vano scale, nel punto più denso di router FFD (luci corridoio), così da avere molteplici percorsi fisici di ridondanza verso la mesh.
- Gli **Access Point Wi-Fi** sono posizionati baricentricamente con copertura sovrapposta (roaming 802.11r).
- La **sala tecnica** al piano terra ospita: Core Switch L3, Firewall/NAT, Broker MQTT, Web Server, DB Server.
- La **rete domotica** è fisicamente separata dalla rete ospiti tramite VLAN dedicata (VLAN 20).

---

## 4. Schema Logico degli Apparati Attivi

![Schema logico apparati attivi e flusso dati MQTT](../img/schema_logico.svg)

La topologia logica comprende i seguenti livelli:

**Livello L1/L2 — Accesso Zigbee:**  
Ogni piano ha una rete Zigbee indipendente con coordinatore integrato nel gateway. I router FFD (lampade) formano la mesh. I dispositivi RFD (comandi, serrature, sensori) comunicano con il router FFD più vicino con un solo hop.

**Livello L2 — LAN Ethernet:**  
Access switch per piano (802.1Q trunk) + 2 prese Ethernet per stanza. Le VLAN separano: server farm, piani, domotica, Wi-Fi ospiti, Wi-Fi staff.

**Livello L3 — Core Switch (routing statico inter-VLAN):**  
Il Core Switch L3 smista il traffico tra le VLAN con routing statico. La VLAN 20 (domotica) è segregata: i gateway Zigbee possono raggiungere solo il broker MQTT (10.0.1.10), non la rete ospiti.

**Livello L3/L4 — Firewall/NAT:**  
4 IP pubblici mappati con PAT (Port Address Translation):
- IP1 → Web server (porta 443, HTTPS prenotazioni)
- IP2 → VPN gateway (OpenVPN, accesso remoto staff direzione)
- IP3/IP4 → NAT dinamico per accesso Internet ospiti Wi-Fi

**Livello L7 — Gateway Zigbee (zigbee2mqtt):**  
Il bridge `zigbee2mqtt` traduce la semantica ZCL Zigbee in JSON MQTT, pubblicando e sottoscrivendo topic verso il broker Mosquitto on-premise.

**Livello L7 — Broker MQTT (Mosquitto):**  
Riceve i messaggi dai gateway Zigbee (publisher) e li distribuisce alle dashboard web e all'app di gestione (subscriber).

**Livello L7 — Web Server (Node.js + NGINX):**  
Eroga il sito di prenotazione (HTTPS), la dashboard domotica per la reception e i comandi web per aree comuni. Si connette al broker MQTT come client per inviare comandi e ricevere stati.

---

## 5. Piano di indirizzamento e routing

![Subnetting e tabella di routing](../img/subnetting.svg)

Il subnetting utilizza `10.0.0.0/8` come spazio privato con VLAN dedicate per ogni zona funzionale. Il **routing è statico** dato il numero contenuto di subnet e la topologia stabile dell'edificio.

**Regole firewall significative:**

```
VLAN20 (Domotica) → PERMIT TCP 10.0.20.0/24 → 10.0.1.10:1883   # MQTT verso broker
VLAN20 (Domotica) → DENY any any                                 # blocca tutto il resto
VLAN30 (Ospiti)   → PERMIT any → Internet (via NAT)
VLAN30 (Ospiti)   → DENY any → 10.0.0.0/8                       # isola ospiti dalla LAN
VLAN10 (Server)   → PERMIT TCP 0.0.0.0/0 → 10.0.1.20:443        # HTTPS prenotazioni
```

---

## 6. Funzionalità tecnologiche dei dispositivi

![Dispositivi IoT per stanza e messaggi MQTT JSON](../img/dispositivi_iot.svg)

### 6.1 Dispositivi per ogni stanza

Ogni stanza (36 in totale) è dotata dei seguenti dispositivi Zigbee:

| Dispositivo | Tipo Zigbee | Alimentazione | Cluster ZCL | Note |
|-------------|------------|---------------|-------------|------|
| Lampada da soffitto | FFD (Router) | Rete 220V | `0x0006` On/Off, `0x0008` Level, `0x0300` Color | Routing mesh |
| Condizionatore | FFD (Router) | Rete 220V | `0x0201` Thermostat, `0x0202` Fan Control | Routing mesh |
| Persiana motorizzata | RFD | Rete 220V | `0x0102` Window Covering | — |
| Serratura elettronica | RFD | Batteria 3.6V | `0x0101` Door Lock | Vita batteria ~2 anni |
| Comando comodino | RFD | Batteria CR2032 | `0x0005` Scenes, `0x0006` On/Off | Vita batteria ~3 anni |
| Sensore T/H | RFD | Batteria AA | `0x0402` Temperature, `0x0405` Humidity | Vita batteria ~2 anni |

**TV:** non Zigbee, riceve stream IP (IPTV via multicast su VLAN piano). Si connette alle prese Ethernet della stanza.

### 6.2 Comportamento default (rimozione smartcard)

Dopo 1 minuto dalla rimozione della smartcard del cliente:
- Luci → OFF
- Condizionatore → 26°C, modalità economy
- Persiane → posizione chiusa (0%)
- Serratura → bloccata

Questo viene gestito da una regola nel Web Server (Node.js) che:
1. Riceve l'evento di logout smartcard dal topic `albergo/p1/s101/smartcard/stato`
2. Attende 60 secondi
3. Pubblica i comandi di default sui topic `/cmd` di ogni attuatore della stanza

### 6.3 Dispositivi aree comuni

Le **aree comuni** (corridoi, giardino, garage, palestra, ristorante, sala convegni) usano comandi Zigbee RF anche alle postazioni di lavoro dei dipendenti, con la possibilità di **sezionare per gruppo di utenze** (luci, AC, portoni) tramite i gruppi Zigbee e i topic MQTT gerarchici.

---

## 7. Protocolli di comunicazione e sicurezza

### 7.1 Stack protocollare per livello

| Livello | Rete ospiti Wi-Fi | Rete domotica Zigbee | Rete staff |
|---------|-------------------|---------------------|------------|
| L7 | HTTPS, IPTV | MQTT (TLS), zigbee2mqtt | HTTPS, SSH |
| L4 | TCP/UDP | TCP (MQTT) | TCP |
| L3 | IPv4 (10.0.30/31.x) | IPv4 (10.0.20.x) | IPv4 |
| L2 | 802.11ac (WPA3) | 802.3 Ethernet | 802.11ac (WPA2-Enterprise) |
| L1 | 5 GHz / 2.4 GHz | 2.4 GHz (Zigbee IEEE 802.15.4) | 5 GHz |

### 7.2 Sicurezza rete Zigbee

La rete Zigbee utilizza **crittografia AES-128** a livello NWK (rete) e a livello APS (applicazione). Il coordinator (gateway) è il **Trust Center** che distribuisce le chiavi di rete. Il parametro `permit_join` è mantenuto a `false` in produzione; viene abilitato temporaneamente solo durante la fase di commissioning di nuovi dispositivi.

La comunicazione **gateway → broker MQTT** avviene su **TLS 1.3** (porta 8883) con autenticazione reciproca tramite certificati X.509.

### 7.3 Autenticazione utenti

| Utente | Metodo | Tecnologia |
|--------|--------|------------|
| Ospite Wi-Fi | Captive portal L7 + sessione web | NGINX + cookie JWT |
| Staff (addetti pulizie) | WPA2-Enterprise per AP | 802.1X EAP-PEAP con RADIUS |
| Direttore/Amministratore | VPN OpenVPN + HTTPS | Certificato client + password |
| Reception (dashboard) | Login web HTTPS | JWT + sessione server-side |

---

## 8. Topic MQTT — gerarchia e casi d'uso

```
albergo/
├── pT/                          # Piano Terra
│   ├── reception/luce/stato
│   ├── reception/luce/cmd
│   ├── hall/portone/stato
│   ├── hall/portone/cmd
│   ├── congressi/luce/cmd
│   └── ...
├── p1/                          # Piano 1
│   ├── s101/
│   │   ├── luce/stato           # {"state":"ON","brightness":80}
│   │   ├── luce/cmd             # {"state":"OFF"}
│   │   ├── ac/stato             # {"power":"ON","setpoint":22,"mode":"cool"}
│   │   ├── ac/cmd               # {"state":"OFF"}
│   │   ├── persiana/stato       # {"position":75,"tilt":30}
│   │   ├── persiana/cmd         # {"position":0}
│   │   ├── serratura/stato      # {"locked":true,"card_uid":"A3F2C1"}
│   │   ├── serratura/cmd        # {"action":"unlock"}
│   │   ├── sensor/stato         # {"temperature":23.4,"humidity":58}
│   │   └── smartcard/stato      # {"present":true,"uid":"A3F2C1"}
│   └── corridoio/luce/cmd       # {"state":"ON"} → gruppo intero piano
├── p2/...
├── p3/...
└── sistema/
    ├── allarme/stato
    └── energia/consumo          # monitoraggio kWh
```

**Wildcard subscription** usate dalla dashboard reception:
- `albergo/+/+/luce/stato` → stato di tutte le luci di tutti i piani
- `albergo/p1/#` → tutti gli eventi del piano 1

---

## 9. Database e codice

### 9.1 Schema database (PostgreSQL)

```sql
-- Gestione prenotazioni
CREATE TABLE prenotazioni (
    id          SERIAL PRIMARY KEY,
    nome        VARCHAR(100) NOT NULL,
    cognome     VARCHAR(100) NOT NULL,
    email       VARCHAR(150) UNIQUE NOT NULL,
    stanza_id   INTEGER REFERENCES stanze(id),
    checkin     DATE NOT NULL,
    checkout    DATE NOT NULL,
    n_letti     INTEGER NOT NULL DEFAULT 1,
    caparra     NUMERIC(8,2),
    caparra_pagata BOOLEAN DEFAULT FALSE,
    card_uid    VARCHAR(20),
    creata_il   TIMESTAMP DEFAULT NOW()
);

CREATE TABLE stanze (
    id          SERIAL PRIMARY KEY,
    numero      VARCHAR(5) NOT NULL UNIQUE,   -- es. "S101"
    piano       INTEGER NOT NULL,
    n_letti     INTEGER NOT NULL,
    prezzo_notte NUMERIC(8,2) NOT NULL
);

-- Log stati IoT (time-series semplificata)
CREATE TABLE iot_log (
    id          BIGSERIAL PRIMARY KEY,
    topic       VARCHAR(200) NOT NULL,
    payload     JSONB NOT NULL,
    ricevuto_il TIMESTAMP DEFAULT NOW()
);
CREATE INDEX ON iot_log (topic, ricevuto_il DESC);

-- Sessioni utenti staff
CREATE TABLE sessioni (
    token       VARCHAR(64) PRIMARY KEY,
    user_id     INTEGER NOT NULL,
    creata_il   TIMESTAMP DEFAULT NOW(),
    scade_il    TIMESTAMP NOT NULL
);
```

### 9.2 Funzione Node.js — gestione default stanza alla partenza

```javascript
/**
 * Ascolta gli eventi di rimozione smartcard e ripristina
 * i default energetici della stanza dopo 60 secondi.
 * 
 * @param {MqttClient} mqttClient  - client MQTT già connesso al broker
 * @param {Pool}       db          - pool PostgreSQL
 */
function attivaDefaultStanza(mqttClient, db) {

  // Sottoscriviamo tutti gli eventi smartcard di tutte le stanze
  mqttClient.subscribe('albergo/+/+/smartcard/stato', { qos: 1 });

  const timer_stanze = new Map(); // stanza_id → setTimeout handle

  mqttClient.on('message', async (topic, message) => {
    const payload = JSON.parse(message.toString());

    // Estrai piano e stanza dal topic (es. albergo/p1/s101/smartcard/stato)
    const parts = topic.split('/');
    const piano  = parts[1];   // es. "p1"
    const stanza = parts[2];   // es. "s101"
    const stanzaKey = `${piano}/${stanza}`;

    if (payload.present === true) {
      // Ospite rientrato → annulla timer di default se attivo
      if (timer_stanze.has(stanzaKey)) {
        clearTimeout(timer_stanze.get(stanzaKey));
        timer_stanze.delete(stanzaKey);
        console.log(`[${stanzaKey}] Ospite rientrato, default annullato`);
      }
      return;
    }

    // Smartcard rimossa → avvia timer 60 secondi
    if (timer_stanze.has(stanzaKey)) return; // già in attesa

    console.log(`[${stanzaKey}] Smartcard rimossa, default tra 60s`);
    const handle = setTimeout(() => {
      ripristinaDefault(mqttClient, db, piano, stanza);
      timer_stanze.delete(stanzaKey);
    }, 60_000);

    timer_stanze.set(stanzaKey, handle);
  });
}

/**
 * Pubblica i comandi di ripristino default per tutti gli attuatori
 * della stanza specificata.
 */
async function ripristinaDefault(mqttClient, db, piano, stanza) {
  const base = `albergo/${piano}/${stanza}`;

  const comandi = [
    { topic: `${base}/luce/cmd`,     payload: { state: 'OFF' } },
    { topic: `${base}/ac/cmd`,       payload: { power: 'ON', setpoint: 26, mode: 'eco' } },
    { topic: `${base}/persiana/cmd`, payload: { position: 0, tilt: 0 } },
  ];

  for (const { topic, payload } of comandi) {
    mqttClient.publish(topic, JSON.stringify(payload), { qos: 1, retain: false });
    console.log(`[DEFAULT] Pubblicato su ${topic}:`, payload);
  }

  // Log su DB
  await db.query(
    `INSERT INTO iot_log (topic, payload) VALUES ($1, $2)`,
    [`${base}/sistema/default`, JSON.stringify({ event: 'checkout_default', ts: Date.now() })]
  );
}
```

### 9.3 Interfaccia grafica — Dashboard Reception (bozza)

La dashboard web è una **SPA (Single Page Application)** servita da NGINX, con le seguenti sezioni:

1. **Mappa piani**: visualizzazione in tempo reale dello stato di ogni stanza (luci, AC, occupazione).
2. **Controllo stanza**: selezionando una stanza, la reception può inviare comandi MQTT direttamente (es. accendere la luce per un sopralluogo).
3. **Prenotazioni**: calendario mensile, registrazione arrivi/partenze, gestione caparre.
4. **Aree comuni**: pannello per i comandi delle zone comuni (corridoi, sala congressi, garage).
5. **Alert**: notifiche push (via WebSocket) per allarmi (es. temperatura stanza fuori range, batteria dispositivo bassa).

Il frontend si connette al Web Server tramite **WebSocket** (Socket.io), il quale funge da proxy MQTT→WebSocket per il push degli stati in tempo reale verso il browser.

---

## 10. Protocolli di sicurezza delle informazioni — riepilogo

| Minaccia | Contromisura |
|----------|-------------|
| Accesso non autorizzato rete ospiti | VLAN segregata, Captive Portal, timeout sessione 24h |
| Intercettazione comunicazioni IoT | TLS 1.3 su MQTT (porta 8883), AES-128 Zigbee NWK |
| Accesso non autorizzato rete domotica | ACL VLAN20: solo TCP→10.0.1.10:8883 verso broker MQTT |
| Attacchi da Internet | Firewall stateful, nessuna porta domotica esposta, VPN per accesso remoto |
| Dispositivi Zigbee non autorizzati | `permit_join: false` in produzione, commissioning controllato |
| Furto credenziali staff | WPA2-Enterprise 802.1X (EAP-PEAP), RADIUS server |
| Sniffing MQTT | TLS obbligatorio, autenticazione client broker con username/password SHA-256 |

---

## 11. Conclusioni

L'architettura scelta — **Gateway L7 Zigbee con broker MQTT on-premise** — soddisfa tutti i requisiti del compito:

- **Connettività ubiqua**: ogni stanza ha Ethernet + Wi-Fi per gli ospiti e Zigbee per la domotica.
- **Domotica completa**: luci, AC, persiane, serrature comandabili da reception e da app web, con logica default automatica.
- **Prenotazioni online**: sito HTTPS con calendario, caparre, download brochure PDF.
- **Sicurezza**: segmentazione VLAN, Firewall/NAT, TLS, 802.1X, AES-128 Zigbee.
- **Dashboard e comandi web**: la combinazione Zigbee → zigbee2mqtt → MQTT broker → WebSocket → browser realizza il flusso completo in tempo reale senza cloud di terze parti.
- **Robustezza**: la mesh Zigbee è indipendente dalla LAN; il broker on-premise garantisce continuità anche senza Internet; percorsi di ridondanza fisici sui gateway di piano.
