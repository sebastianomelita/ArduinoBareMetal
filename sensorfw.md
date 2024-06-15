>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# Fasi principali del firmware di un sensore che Utilizza MQTT

1. **Inizializzazione dei Parametri di Connessione**
   - Configurare i parametri del broker MQTT (indirizzo, porta, username, password).
   - Configurare il pin del sensore di temperatura e l'intervallo di lettura.

2. **Connessione al Broker MQTT**
   - Stabilire la connessione con il broker MQTT utilizzando i parametri configurati.

3. **Inizializzazione del Sensore di Temperatura**
   - Configurare il pin del sensore per la lettura della temperatura.

4. **Ciclo Principale**
   - Ottenere il tempo corrente.
   - Leggere il valore della temperatura dal sensore.
   - Creare un messaggio con il valore della temperatura.
   - Inviare il messaggio al broker MQTT se è trascorso l'intervallo prefissato.
   - Aggiornare il timestamp dell'ultima lettura inviata.

5. **Attesa Prima della Prossima Iterazione**
   - Attendere un breve periodo (ad esempio, 1 secondo) prima di ripetere il ciclo.


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



>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

