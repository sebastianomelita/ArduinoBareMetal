>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# Fasi Principali del Firmware di un Sensore che Utilizza MQTT

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
    
    time.sleep(1)  # Attendere un secondo prima della prossima iterazione
```





>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

