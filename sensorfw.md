>[Torna a reti ethernet](archeth.md)>[Torna a Sensori](/approfondimenti/sensore.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

## Fasi principali del firmware di un sensore MQTT

Il firmware di un nodo sensore che pubblica dati su un broker MQTT si articola in cinque fasi, schematizzate qui sotto.

<img src="/img/01_fasi_firmware.svg" alt="Fasi del firmware" width="700">

### 1. Inizializzazione dei parametri di connessione

Si configurano i parametri necessari al funzionamento: indirizzo e porta del broker MQTT, username e password, pin del sensore di temperatura, intervallo di lettura. In C++ sono tipicamente costanti `const char*` o `#define`; in Python sono variabili a livello modulo.

```cpp
const char* mqtt_server = "broker_address";
const int mqtt_port = 1883;
const long interval = 60000;  // millisecondi
```

### 2. Connessione al broker MQTT

Si stabilisce prima la connessione di rete (su ESP32 tipicamente WiFi) e poi la connessione MQTT vera e propria, che include l'handshake del protocollo con il broker.

```cpp
WiFi.begin(ssid, password);
// ... attesa connessione WiFi ...
client.setServer(mqtt_server, mqtt_port);
client.connect("ArduinoClient", mqtt_user, mqtt_password);
```

### 3. Inizializzazione del sensore di temperatura

Si configura il pin o l'interfaccia di comunicazione con il sensore. Nel caso più semplice è una `pinMode()`; con sensori digitali come BME280 o DS18B20 servono librerie dedicate che gestiscono il protocollo specifico (I2C, SPI, 1-Wire).

### 4. Ciclo principale

È il cuore del firmware, eseguito ripetutamente nel `loop()`:

- ottenere il tempo corrente con `millis()`
- leggere il valore della temperatura
- comporre un messaggio JSON con sensor_id e valore
- pubblicare il messaggio al broker se è trascorso l'intervallo
- aggiornare il timestamp dell'ultimo invio

```cpp
unsigned long currentTime = millis();
if (currentTime - lastSentTime >= interval) {
    float temperature = read_temperature();
    snprintf(message, sizeof(message),
        "{\"sensor_id\": \"%s\", \"temperature\": %.2f}",
        sensor_id, temperature);
    client.publish(mqtt_topic, message);
    lastSentTime = currentTime;
}
```

Il pattern `currentTime - lastSentTime >= interval` è preferibile a un `delay(interval)` perché lascia la CPU libera di gestire altri compiti (come il `client.loop()` di MQTT che mantiene viva la connessione).

### 5. Attesa prima della prossima iterazione

Un breve `delay()` (tipicamente 1 secondo) o, nelle implementazioni a basso consumo, una modalità sleep più o meno profonda.


## Fasi in C++

``` Python
#include <WiFi.h>
#include <PubSubClient.h>

// Parametri di connessione Wi-Fi
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Parametri di connessione MQTT
const char* mqtt_server = "broker_address";
const int mqtt_port = 1883;
const char* mqtt_user = "username";
const char* mqtt_password = "password";
const char* mqtt_topic = "sensors/temperature";

// Identificatore unico del sensore
const char* sensor_id = "sensor_001";

// Intervallo di lettura in millisecondi
const long interval = 60000;
unsigned long lastSentTime = 0;

// Client Wi-Fi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connessione a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connesso");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connessione al broker MQTT...");
    if (client.connect("ArduinoClient", mqtt_user, mqtt_password)) {
      Serial.println("connesso");
    } else {
      Serial.print("fallito, rc=");
      Serial.print(client.state());
      Serial.println(" riprovo tra 5 secondi");
      delay(5000);
    }
  }
}

float read_temperature() {
  // Simulazione della lettura della temperatura
  return random(200, 300) / 10.0;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentTime = millis();

  if (currentTime - lastSentTime >= interval) {
    float temperature = read_temperature();

    char message[100];
    snprintf(message, sizeof(message), "{\"sensor_id\": \"%s\", \"temperature\": %.2f}", sensor_id, temperature);

    if (client.publish(mqtt_topic, message)) {
      Serial.print("Messaggio inviato: ");
      Serial.println(message);
    } else {
      Serial.println("Errore nell'invio del messaggio");
    }

    lastSentTime = currentTime;
  }
}
```

## Fasi in Python

``` Python
import time
import paho.mqtt.client as mqtt
import random  # Utilizzato per simulare la lettura del sensore

# Parametri di connessione MQTT
MQTT_BROKER = "broker_address"
MQTT_PORT = 1883
MQTT_USERNAME = "username"
MQTT_PASSWORD = "password"
MQTT_TOPIC = "sensors/temperature"

# Identificatore unico del sensore
SENSOR_ID = "sensor_001"

# Intervallo di lettura in secondi
INTERVAL = 60

# Inizializzazione del client MQTT
client = mqtt.Client()
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

def on_connect(client, userdata, flags, rc):
    print(f"Connesso al broker MQTT con codice di risultato {rc}")

def on_publish(client, userdata, mid):
    print(f"Messaggio pubblicato con ID {mid}")

client.on_connect = on_connect
client.on_publish = on_publish

client.connect(MQTT_BROKER, MQTT_PORT, 60)

def read_temperature():
    # Simulazione della lettura della temperatura
    return round(random.uniform(20.0, 30.0), 2)

last_sent_time = 0

while True:
    current_time = time.time()
    
    if current_time - last_sent_time >= INTERVAL:
        temperature = read_temperature()
        message = f'{{"sensor_id": "{SENSOR_ID}", "temperature": {temperature}}}'
        result = client.publish(MQTT_TOPIC, message)
        
        # Assicurarsi che il messaggio sia stato inviato
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"Messaggio inviato: {message}")
        else:
            print(f"Errore nell'invio del messaggio: {result.rc}")
        
        last_sent_time = current_time
```



>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

