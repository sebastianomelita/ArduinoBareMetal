> [Torna a reti ethernet](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archeth.md) > [Torna a Sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/approfondimenti/sensore.md) > [Messaggi MQTT](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/approfondimenti/messaggi_mqtt.md)

- [Dettaglio architettura Zigbee](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archzigbee.md)
- [Dettaglio architettura BLE](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archble.md)
- [Dettaglio architettura WiFi infrastruttura](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archwifi.md)
- [Dettaglio architettura WiFi mesh](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archmesh.md)
- [Dettaglio architettura LoraWAN](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/lorawanclasses.md)


# Fasi principali del firmware di un attuatore che utilizza MQTT

A differenza del firmware di un **sensore**, che è essenzialmente un *publisher* periodico, il firmware di un **attuatore** è guidato da eventi **asincroni** in arrivo dalla rete: l'attuatore deve mettersi in **ascolto** del topic di comando e, ad ogni messaggio ricevuto, agire sul mondo fisico (un relè, un motore, una lampada). Poiché la consegna del comando deve essere **affidabile** ma il protocollo MQTT non fornisce conferme applicative, è l'attuatore stesso che deve confermare l'avvenuta attuazione **ripubblicando il proprio stato** su un dedicato *topic di feedback*. Questo meccanismo di "ACK applicativo" è la trasposizione, sul livello applicativo, della conferma che il protocollo non garantisce a livello di trasporto.

<p align="center">
  <img src="attuatore_mqtt.svg" alt="Flusso MQTT di un attuatore" width="780"/>
</p>

I tre topic coinvolti sono:

- **topic di comando** `luci/soggiorno/comandi` — l'attuatore è *subscriber* (riceve `{"toggle":"true"}`, `{"state":"on"}`, ecc.)
- **topic di stato / feedback** `luci/soggiorno/stato` — l'attuatore è *publisher* (notifica il proprio stato dopo ogni cambio e/o periodicamente in PUSH)
- **topic di configurazione** `luci/soggiorno/config` — l'attuatore è *subscriber* (riceve impostazioni runtime come `stateperiod`, frequenze, OTA, ecc.)

## Fasi principali

1. **Inizializzazione dei parametri di connessione**

   - Configurare i parametri del broker MQTT (indirizzo, porta, username, password).
   - Configurare il pin di uscita dell'attuatore (es. il pin che pilota il relè della lampada) e lo stato iniziale (tipicamente `off`).
   - Definire l'identificatore univoco del dispositivo (`device_id`) e i tre topic: **comandi**, **stato**, **configurazione**.

2. **Connessione al broker MQTT**

   - Stabilire la connessione con il broker utilizzando i parametri configurati.
   - Registrare le **callback**: `on_connect`, `on_message` (gestione dei messaggi in arrivo), `on_disconnect`.

3. **Inizializzazione dell'attuatore**

   - Configurare il pin di uscita (`pinMode(OUTPUT)` o equivalente).
   - Applicare lo stato iniziale all'uscita fisica.

4. **Sottoscrizione ai topic**

   - Sottoscriversi al **topic di comando** per ricevere i comandi di attuazione.
   - Sottoscriversi al **topic di configurazione** per ricevere impostazioni runtime.

5. **Gestione del comando in arrivo (callback `on_message`)**

   - Estrarre e validare il payload JSON.
   - Verificare se il messaggio è destinato a *questo* dispositivo (matching del `device_id` nel JSON, oppure del prefisso nel path MQTT).
   - Eseguire l'azione richiesta sul pin (ON/OFF, toggle, livello PWM, ...).
   - Aggiornare la variabile interna che rappresenta lo stato corrente.
   - **Pubblicare immediatamente il nuovo stato** sul topic di feedback come *conferma applicativa* dell'avvenuta attuazione.

6. **Ciclo principale**

   - Mantenere viva la connessione MQTT (`client.loop()`).
   - Gestire la riconnessione automatica in caso di disconnessione.
   - **Pubblicazione PUSH periodica dello stato**: se è scaduto l'intervallo `state_period`, ripubblicare comunque lo stato corrente sul topic di feedback (utile a sincronizzare i pannelli di controllo che si fossero persi un aggiornamento).

7. **Attesa prima della prossima iterazione**

   - Breve pausa (ad esempio 1 secondo) per non saturare la CPU.

### Diagramma di flusso

Il diagramma seguente sintetizza le fasi appena descritte. La parte sinistra rappresenta il **flusso sincrono** del firmware (setup lineare + loop principale), mentre la parte destra rappresenta la **callback asincrona** `on_message`, eseguita ogni volta che arriva un messaggio dal broker su uno dei topic sottoscritti.

<p align="center">
  <img src="attuatore_flowchart.svg" alt="Diagramma di flusso del firmware dell'attuatore" width="820"/>
</p>

I due punti chiave del diagramma sono i due **rombi gialli** del loop principale e i due **rombi gialli** della callback:

- nel loop, il primo decide se eseguire `reconnect()`, il secondo se è il momento della pubblicazione **PUSH periodica** dello stato (alternativa PUSH alla sincronizzazione PULL);
- nella callback, il primo filtra i messaggi non destinati a questo `device_id`, il secondo discrimina tra topic di **comando** (con conferma applicativa tramite publish del nuovo stato) e topic di **configurazione**.

## Fasi in Python

```python
import time
import json
import paho.mqtt.client as mqtt
# import RPi.GPIO as GPIO  # da abilitare su Raspberry Pi reale

# Parametri di connessione MQTT
MQTT_BROKER   = "broker_address"
MQTT_PORT     = 1883
MQTT_USERNAME = "username"
MQTT_PASSWORD = "password"

# Topic
TOPIC_CMD    = "luci/soggiorno/comandi"
TOPIC_STATE  = "luci/soggiorno/stato"
TOPIC_CONFIG = "luci/soggiorno/config"

# Identificatore univoco dell'attuatore
DEVICE_ID = "01"

# Pin dell'attuatore (es. relè della lampada)
ACTUATOR_PIN = 17

# Stato corrente dell'attuatore
current_state = "off"

# Periodo di pubblicazione PUSH dello stato (in secondi)
state_period   = 30
last_state_pub = 0

# --- Astrazione hardware (sostituire con GPIO reale) ---
def apply_output(state):
    # GPIO.output(ACTUATOR_PIN, GPIO.HIGH if state == "on" else GPIO.LOW)
    print(f"[HW] Uscita impostata a: {state}")

# --- Pubblicazione stato (feedback applicativo) ---
def publish_state(client):
    global last_state_pub
    payload = json.dumps({"deviceid": DEVICE_ID, "state": current_state})
    client.publish(TOPIC_STATE, payload)
    last_state_pub = time.time()
    print(f"Stato pubblicato su {TOPIC_STATE}: {payload}")

# --- Callback connessione ---
def on_connect(client, userdata, flags, rc):
    print(f"Connesso al broker MQTT con codice {rc}")
    # Sottoscrizione ai topic di comando e configurazione
    client.subscribe(TOPIC_CMD)
    client.subscribe(TOPIC_CONFIG)
    # Annuncio dello stato iniziale all'avvio
    publish_state(client)

# --- Callback messaggio in arrivo ---
def on_message(client, userdata, msg):
    global current_state, state_period
    try:
        payload = json.loads(msg.payload.decode())
    except json.JSONDecodeError:
        print("Payload non valido (JSON malformato)")
        return

    # Filtraggio per device_id
    if payload.get("deviceid") != DEVICE_ID:
        return

    # Topic di comando
    if msg.topic == TOPIC_CMD:
        if payload.get("toggle") == "true":
            current_state = "off" if current_state == "on" else "on"
        elif "state" in payload:
            current_state = payload["state"]
        apply_output(current_state)
        # Conferma applicativa: pubblicazione immediata del nuovo stato
        publish_state(client)

    # Topic di configurazione
    elif msg.topic == TOPIC_CONFIG:
        if "stateperiod" in payload:
            state_period = int(payload["stateperiod"]) / 1000.0
            print(f"Nuovo periodo di pubblicazione stato: {state_period}s")

# --- Inizializzazione client MQTT ---
client = mqtt.Client()
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Stato iniziale dell'uscita
apply_output(current_state)

# Loop non bloccante: la callback on_message viene servita in background
client.loop_start()

try:
    while True:
        now = time.time()
        # Pubblicazione PUSH periodica dello stato
        if now - last_state_pub >= state_period:
            publish_state(client)
        time.sleep(1)
except KeyboardInterrupt:
    client.loop_stop()
    client.disconnect()
```

## Fasi in C++

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Parametri Wi-Fi
const char* ssid     = "your_SSID";
const char* password = "your_PASSWORD";

// Parametri MQTT
const char* mqtt_server   = "broker_address";
const int   mqtt_port     = 1883;
const char* mqtt_user     = "username";
const char* mqtt_password = "password";

// Topic
const char* topic_cmd    = "luci/soggiorno/comandi";
const char* topic_state  = "luci/soggiorno/stato";
const char* topic_config = "luci/soggiorno/config";

// Identificatore univoco dell'attuatore
const char* device_id = "01";

// Pin dell'attuatore (es. relè della lampada)
const int actuator_pin = 5;

// Stato corrente
String current_state = "off";

// Periodo PUSH di pubblicazione stato (ms)
unsigned long state_period   = 30000;
unsigned long last_state_pub = 0;

WiFiClient   espClient;
PubSubClient client(espClient);

// Applica lo stato sul pin fisico
void applyOutput(const String& state) {
  digitalWrite(actuator_pin, state == "on" ? HIGH : LOW);
  Serial.print("[HW] Uscita: ");
  Serial.println(state);
}

// Pubblica lo stato corrente sul topic di feedback
void publishState() {
  StaticJsonDocument<128> doc;
  doc["deviceid"] = device_id;
  doc["state"]    = current_state;
  char buf[128];
  size_t n = serializeJson(doc, buf);
  client.publish(topic_state, buf, n);
  last_state_pub = millis();
  Serial.print("Stato pubblicato: ");
  Serial.println(buf);
}

// Callback per i messaggi in arrivo
void onMessage(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) { Serial.println("JSON malformato"); return; }

  // Filtraggio per device_id
  const char* id = doc["deviceid"];
  if (id == nullptr || String(id) != device_id) return;

  // Topic di comando
  if (String(topic) == topic_cmd) {
    if (doc.containsKey("toggle") && String((const char*)doc["toggle"]) == "true") {
      current_state = (current_state == "on") ? "off" : "on";
    } else if (doc.containsKey("state")) {
      current_state = String((const char*)doc["state"]);
    }
    applyOutput(current_state);
    // Conferma applicativa: pubblicazione immediata dello stato
    publishState();
  }
  // Topic di configurazione
  else if (String(topic) == topic_config) {
    if (doc.containsKey("stateperiod")) {
      state_period = atol(doc["stateperiod"]);
      Serial.print("Nuovo periodo stato (ms): ");
      Serial.println(state_period);
    }
  }
}

void setup_wifi() {
  delay(10);
  Serial.print("Connessione a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connesso");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connessione al broker MQTT...");
    if (client.connect("ActuatorClient", mqtt_user, mqtt_password)) {
      Serial.println("connesso");
      // Sottoscrizione ai topic di interesse
      client.subscribe(topic_cmd);
      client.subscribe(topic_config);
      // Annuncio stato iniziale
      publishState();
    } else {
      Serial.print("fallito, rc=");
      Serial.print(client.state());
      Serial.println(" riprovo tra 5 secondi");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(actuator_pin, OUTPUT);
  applyOutput(current_state);       // stato iniziale
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(onMessage);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();                    // gestisce i messaggi in arrivo

  // Pubblicazione PUSH periodica dello stato
  unsigned long now = millis();
  if (now - last_state_pub >= state_period) {
    publishState();
  }
}
```

## Differenze chiave rispetto al firmware del sensore

| Aspetto | Sensore | Attuatore |
|---|---|---|
| Ruolo MQTT principale | *publisher* | *subscriber* (su comandi e config) + *publisher* (su stato) |
| Modalità di funzionamento | loop periodico di publish | reattiva, guidata da callback `on_message` |
| Conferma applicativa | non necessaria (il dato arriva di nuovo a breve) | **necessaria**: si ripubblica lo stato sul topic di feedback |
| Topic usati | `…/misure` (publish) | `…/comandi` (sub), `…/stato` (pub), `…/config` (sub) |
| Pubblicazione spontanea | misura ogni `INTERVAL` | stato ogni `state_period` (alternativa PUSH alla sincronizzazione PULL) |

Tre principi guidano dunque la scrittura del firmware:

- **Inversione del ruolo**: il sensore è solo *publisher*, l'attuatore è prevalentemente *subscriber* e diventa *publisher* solo per notificare il proprio stato.
- **Conferma applicativa, non protocollare**: ogni comando ricevuto innesca immediatamente la pubblicazione del nuovo stato sul topic di feedback; questo è il meccanismo che permette al display associato al pulsante (o al server applicativo) di verificare l'efficacia del comando e, eventualmente, di ritrasmetterlo.
- **Pubblicazione PUSH periodica dello stato**: anche in assenza di comandi, l'attuatore pubblica periodicamente il proprio stato, costituendo l'alternativa PUSH alla sincronizzazione PULL dei pannelli di controllo che potrebbero aver perso aggiornamenti per disconnessioni di rete.

> [Torna a reti ethernet](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archeth.md)

- [Dettaglio architettura Zigbee](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archzigbee.md)
- [Dettaglio architettura BLE](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archble.md)
- [Dettaglio architettura WiFi infrastruttura](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archwifi.md)
- [Dettaglio architettura WiFi mesh](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/archmesh.md)
- [Dettaglio architettura LoraWAN](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/lorawanclasses.md)
