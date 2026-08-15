# LoRaWAN Device Config — Webapp

Webapp responsive per configurare i device LoRaWAN Heltec V4 SCD41+L76K tramite MQTT. Un singolo file HTML autoconsistente + un modulo JavaScript per l'encoding dei downlink. Nessun backend richiesto.

## Cosa contiene

```
webapp/
├── index.html                    # Webapp principale (CSS + JS inline)
├── mqtt_downlink_encoder.js      # Modulo JS per encoding downlink
└── README.md                     # Questo file
```

## Cosa fa la webapp

Dashboard responsive per operare sui device:

- **Connessione MQTT** via WebSocket al broker centrale
- **Selezione del device** attivo tra quelli hardcoded in `index.html`
- **Visualizzazione stato** (schema payload 0x43): boot count, uptime, batteria, feature flags, config runtime, reset reason
- **Configurazione runtime**: TX interval, SF, potenza TX, timeout GPS, soglie batteria, ADR (con salvataggio persistente in NVS del device)
- **Azioni one-shot**: identify (blink), identify persistente ON/OFF, force TX, reboot, clear NVS (con conferma esplicita per le azioni distruttive)
- **Log MQTT in tempo reale** con distinzione tra uplink misure, uplink state, downlink inviati, errori
- **Toast di notifica** per esiti operazioni
- **Modal di conferma** per azioni pericolose (reboot, clear NVS)

Layout mobile-first: 1 colonna su smartphone, 2 su tablet, 3 su desktop. Tema dark/light automatico basato sulle preferenze del sistema operativo.

## Prerequisiti

**1. Broker MQTT Mosquitto con listener WebSocket abilitato**

La webapp gira in browser, quindi non può usare MQTT su TCP puro. Serve il protocollo `ws://` (o meglio `wss://` per HTTPS). Verifica che il file `/etc/mosquitto/mosquitto.conf` sul broker centrale contenga anche una sezione WebSocket:

```conf
# Listener TCP standard (già presente per il bridge)
listener 1883

# Listener WebSocket per client browser
listener 9001
protocol websockets
allow_anonymous true
```

Dopo la modifica, riavvia Mosquitto:

```bash
sudo /etc/init.d/mosquitto restart
```

Verifica dai log che il listener sia attivo:

```bash
sudo tail -20 /var/log/syslog | grep mosquitto
```

Dovresti vedere:
```
Opening ipv4 listen socket on port 9001.
Opening websockets listen socket on port 9001.
```

**2. Codec ChirpStack aggiornato**

Il device profile su ChirpStack deve avere il codec `chirpstack_codec.js` (versione aggiornata con supporto schema 0x43 e range comandi 0x01-0x07). Verifica sulla UI: **Device Profiles → il tuo profilo → Codec**.

**3. Bridge MQTT che forwarda gli uplink**

Il bridge Mosquitto sul RPi con ChirpStack deve inoltrare al broker centrale sia gli uplink `application/#` sia importare i downlink `application/+/device/+/command/down`. Vedi la dispensa `dispensa-chirpstack-configurazione.md` sezione "Bridge MQTT".

## Come servire la webapp

La webapp è composta da file statici, servibile da qualsiasi web server.

### Opzione veloce con Python (senza installazioni)

Se hai Python installato (viene di serie su Linux e macOS):

```bash
cd webapp/
python3 -m http.server 8000
```

Poi apri il browser su `http://localhost:8000`.

Per raggiungere la webapp da uno smartphone sulla stessa rete WiFi, usa l'IP della macchina:

```bash
# Trova il tuo IP locale
ip addr show   # Linux
ipconfig       # Windows
ifconfig       # macOS
```

Sullo smartphone apri `http://192.168.x.x:8000` (sostituisci con il tuo IP).

### Opzione con Node.js

Se hai Node.js:

```bash
cd webapp/
npx http-server -p 8000
```

### Opzione produzione con nginx

Per un deployment stabile, copia i file in una directory servita da nginx:

```bash
sudo cp -r webapp/ /var/www/html/
```

Poi accedi via `http://<tuo-server>/webapp/`.

**Attenzione HTTPS**: se servi la webapp via HTTPS, il browser richiederà il broker MQTT via `wss://` (WebSocket sicuro), non `ws://` puro. Vedi la sezione "HTTPS" più avanti.

## Come configurare i device

I device disponibili sono definiti direttamente nel file `index.html`, in cima allo script JavaScript. Cerca il blocco:

```javascript
const DEVICES = [
    {
        id: 'heltec-serra-01',
        label: 'Heltec Serra 01 (SCD41 + L76K)',
        applicationUuid: '1c2774a7-fe34-46ef-a7bf-18dbd11061fb',
        devEui: 'f85b1bfffebed444'
    }
];
```

Per aggiungere un secondo device, estendi l'array:

```javascript
const DEVICES = [
    {
        id: 'heltec-serra-01',
        label: 'Heltec Serra 01 (SCD41 + L76K)',
        applicationUuid: '1c2774a7-fe34-46ef-a7bf-18dbd11061fb',
        devEui: 'f85b1bfffebed444'
    },
    {
        id: 'heltec-serra-02',
        label: 'Heltec Serra 02 (SCD41 + L76K)',
        applicationUuid: '1c2774a7-fe34-46ef-a7bf-18dbd11061fb',
        devEui: 'f85b1bfffebed445'
    }
];
```

Il selettore in alto della webapp mostrerà automaticamente tutti i device definiti.

**Dove trovare i valori**:
- **applicationUuid**: sulla UI ChirpStack apri l'application, l'UUID è nell'URL: `https://<ip>:8080/#/tenants/.../applications/<UUID>`
- **devEui**: nel log seriale del device al boot (`DevEUI: F8-5B-1B-...`), in **lowercase** senza trattini

## Come si usa

**Flusso tipico dopo l'apertura della pagina**:

1. **Verifica** che l'URL del broker sia corretto (default `ws://proxy.marconicloud.it:9001`, modificabile)
2. **Clicca "Connetti"** — il pallino diventa verde se la connessione riesce
3. **Seleziona il device** dal dropdown (se ne hai più di uno)
4. **Clicca "Richiedi stato"** — la webapp invia un `GET_STATE` al device
5. **Attendi 1-2 minuti** — al prossimo TX il device manderà il payload state, la webapp lo riceve e popola i campi di configurazione
6. **Modifica i valori** — ogni cambio pubblica automaticamente un downlink di configurazione FPort 20
7. **Aspetta la conferma** — il device applica la config, salva in NVS, e nel ciclo successivo invia un nuovo state con i valori aggiornati

Le **azioni one-shot** (identify, force TX, reboot) sono immediate: cliccando il bottone si invia il downlink senza modificare configurazioni persistenti.

## Come funziona internamente

**Encoding downlink**

Il modulo `mqtt_downlink_encoder.js` costruisce i bytes del comando e li converte in base64. Esempio:

```javascript
MqttDownlink.encodeSetTxInterval(3)
// → "EQM="  (bytes [0x11, 0x03])
```

Il payload viene pubblicato su MQTT nel formato:
```
Topic:   application/<UUID>/device/<devEui>/command/down
Payload: {"data": "EQM="}
```

ChirpStack riceve, decodifica base64, e il codec deduce il fPort dal primo byte (0x11 → FPort 20).

**Decodifica uplink**

La webapp è sottoscritta al topic `application/<UUID>/device/<devEui>/event/up`. Riceve JSON tipo:

```json
{
  "object": {
    "payload_type": "state",
    "boot_count": 42,
    "uptime_s": 86400,
    ...
  }
}
```

Se `payload_type == "state"`, popola il form. Altrimenti considera l'uplink come misura ambientale e lo registra solo nel log.

## HTTPS (opzionale, per produzione)

Se servi la webapp via HTTPS (es. GitHub Pages, Netlify, o dietro nginx con certificato Let's Encrypt), il browser **blocca** connessioni WebSocket non sicure. Serve `wss://` invece di `ws://`.

**Configurazione Mosquitto per WebSocket TLS**:

```conf
listener 9001
protocol websockets
certfile /etc/letsencrypt/live/tuo-dominio.it/fullchain.pem
keyfile  /etc/letsencrypt/live/tuo-dominio.it/privkey.pem
```

Nella webapp cambia l'URL default in:
```
wss://proxy.marconicloud.it:9001
```

## Autenticazione MQTT (opzionale)

Se il broker richiede username/password, MQTT.js li accetta come opzioni:

```javascript
app.client = mqtt.connect(url, {
    username: 'utente',
    password: 'password',
    ...
});
```

Attenzione: le credenziali vengono esposte nel JavaScript client-side (chiunque legga il source ha le credenziali). Per casi seri considera un backend che faccia da proxy.

## Troubleshooting

### "Errore connessione" al click su Connetti

- Verifica che Mosquitto abbia il listener WebSocket attivo (vedi Prerequisiti)
- Verifica che la porta 9001 non sia bloccata da firewall
- Prova con `mosquitto_sub` da terminale per confermare che il broker TCP funzioni:
  ```bash
  mosquitto_sub -h proxy.marconicloud.it -p 1883 -v -t "application/#"
  ```
- Apri gli sviluppatori del browser (F12) e guarda la console per errori più dettagliati

### La connessione riesce ma non vedo mai uplink

- Verifica sul lato ChirpStack UI che il device stia effettivamente trasmettendo (**LoRaWAN frames**)
- Verifica che il bridge Mosquitto sul RPi stia forwardando (`sudo tail /var/log/syslog | grep mosquitto`)
- Verifica che l'applicationUuid e il devEui nella webapp corrispondano esattamente al device registrato

### GET_STATE inviato ma nessun state torna indietro

- Il device deve avere il firmware aggiornato con supporto `CMD_GET_STATE = 0x07` e schema payload 0x43
- Il ciclo di risposta ha latenza tipica di 1-2 TX interval (max 2 minuti con TX_INTERVAL=60s)
- Guarda i log del device: dovresti vedere `[DOWNLINK] GET_STATE: verra' inviato payload state nel prossimo TX`

### La modifica di config non ha effetto sul device

- Verifica che il downlink sia effettivamente in coda su ChirpStack (**Queue** del device)
- Se resta in stato `Pending: no` cronico, c'è un problema di sessione LoRaWAN — vedi dispensa firmware sezione "Persistenza del frame counter"
- Il device processa i downlink SOLO nella finestra RX dopo un suo uplink (Class A), quindi c'è latenza minima 1 ciclo TX

### Test manuale via mosquitto_pub

Per verificare che il downlink funzioni senza usare la webapp, usa il formato completo richiesto da ChirpStack (`devEui`, `fPort`, `confirmed`, `data`):

```bash
# Chiedi state al device (0x07 = GET_STATE, FPort 10)
mosquitto_pub -h proxy.marconicloud.it -p 1883 \
  -t "application/1c2774a7-fe34-46ef-a7bf-18dbd11061fb/device/f85b1bfffebed444/command/down" \
  -m '{"devEui":"f85b1bfffebed444","fPort":10,"confirmed":false,"data":"Bw=="}'

# Cambia TX interval a 5 minuti (0x11 0x03 = SET_TX_INTERVAL preset=3, FPort 20)
mosquitto_pub -h proxy.marconicloud.it -p 1883 \
  -t "application/1c2774a7-fe34-46ef-a7bf-18dbd11061fb/device/f85b1bfffebed444/command/down" \
  -m '{"devEui":"f85b1bfffebed444","fPort":20,"confirmed":false,"data":"EQM="}'
```

**Nota importante**: il payload MQTT deve contenere **tutti** e quattro i campi (`devEui`, `fPort`, `confirmed`, `data`), altrimenti ChirpStack scarta silenziosamente il messaggio senza processarlo. Un payload `{"data":"..."}` da solo non funziona.

## Personalizzazione

**Cambiare colori**: modifica le CSS variables in cima a `index.html`:

```css
:root {
    --accent: #0284c7;      /* blu principale */
    --success: #16a34a;     /* verde OK */
    --warning: #ea580c;     /* arancione avviso */
    --danger: #dc2626;      /* rosso pericolo */
}
```

Le stesse variabili in `@media (prefers-color-scheme: dark)` per il tema scuro.

**Aggiungere una nuova sezione**: aggiungi una `<div class="card">` dentro `.grid`. Se vuoi che occupi 2 o 3 colonne su desktop, aggiungi la classe `grid-span-2` o `grid-span-3`.

**Aggiungere un nuovo comando**: aggiungi la funzione al modulo `mqtt_downlink_encoder.js`, poi crea il bottone/controllo nell'HTML e connetti l'handler in `initButtons()`.

## Sicurezza

Questa webapp è pensata per uso **didattico e prototipale**. In produzione:

- Usa HTTPS + `wss://` (mai `ws://` esposto in rete pubblica)
- Autentica MQTT con username/password (o meglio: mTLS con certificati client)
- Considera un backend che faccia da proxy con autenticazione utente vera (JWT, OAuth)
- Non esporre il broker MQTT direttamente su Internet — usa VPN o reverse proxy con ACL
- Applica ACL Mosquitto per limitare quali topic ogni client può pubblicare/sottoscrivere

## Riferimenti

- Codec ChirpStack: [`../chirpstack_codec.js`](../chirpstack_codec.js)
- Firmware Heltec V4: [`../heltec_v4_scd41_gps_lorawan.ino`](../heltec_v4_scd41_gps_lorawan.ino)
- Dispensa ChirpStack: [`../dispensa-chirpstack-configurazione.md`](../dispensa-chirpstack-configurazione.md)
- Dispensa firmware: [`../dispensa-firmware-lorawan-lowpower.md`](../dispensa-firmware-lorawan-lowpower.md)
- MQTT.js documentation: https://github.com/mqttjs/MQTT.js
