>[Torna a reti LoRaWAN](lorawanclasses.md)

- [Dettaglio architettura Ethernet](archeth.md)
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
import random
import struct
from lorawan.lorawan import LoRaWAN
from lorawan.crypto import loramac

# Configurazione LoRaWAN
DEVICE_ADDRESS = '26011BEE'  # esempio di indirizzo dispositivo
NWK_SESSION_KEY = '2B7E151628AED2A6ABF7158809CF4F3C'
APP_SESSION_KEY = '2B7E151628AED2A6ABF7158809CF4F3C'
DEV_EUI = '00E4D1607A383409'
APP_EUI = '70B3D57ED00201A6'
APP_KEY = '2B7E151628AED2A6ABF7158809CF4F3C'

# Intervallo di lettura in secondi
INTERVAL = 60

# Funzione per leggere la temperatura simulata
def read_temperature():
    return round(random.uniform(20.0, 30.0), 2)

# Inizializzazione LoRaWAN
lora = LoRaWAN.new()
lora.setDevAddr(DEVICE_ADDRESS)
lora.setNwkSKey(NWK_SESSION_KEY)
lora.setAppSKey(APP_SESSION_KEY)

last_sent_time = 0

while True:
    current_time = time.time()
    
    if current_time - last_sent_time >= INTERVAL:
        temperature = read_temperature()
        # Compattare il messaggio in binario
        payload = struct.pack('>f', temperature)  # Usa 'f' per un float a 4 byte
        lora.create(MHDR='Unconfirmed Data Up', port=1, data=payload)
        lorawan_message = lora.to_raw()
        
        # Inviare il messaggio (pseudo-codice, dipende dalla libreria/hardware usato)
        result = send_lorawan_message(lorawan_message)
        
        # Assicurarsi che il messaggio sia stato inviato
        if result:
            print(f"Messaggio inviato: {temperature}°C")
        else:
            print(f"Errore nell'invio del messaggio")
        
        last_sent_time = current_time

```

## Fasi in C++

``` Python
#include <LoRa.h>
#include <SPI.h>

// Configurazione LoRaWAN
const char* deviceAddress = "26011BEE";  // esempio di indirizzo dispositivo
const char* nwkSessionKey = "2B7E151628AED2A6ABF7158809CF4F3C";
const char* appSessionKey = "2B7E151628AED2A6ABF7158809CF4F3C";

// Intervallo di lettura in secondi
const int INTERVAL = 60 * 1000;  // in millisecondi
unsigned long lastSentTime = 0;

// Funzione per leggere la temperatura simulata
float readTemperature() {
    return random(2000, 3000) / 100.0;
}

void setup() {
    Serial.begin(9600);
    while (!Serial);

    // Inizializzazione LoRa
    if (!LoRa.begin(868E6)) {  // Frequenza europea
        Serial.println("Avvio LoRa fallito!");
        while (1);
    }
    
    LoRa.setDeviceAddress(deviceAddress);
    LoRa.setNwkSessionKey(nwkSessionKey);
    LoRa.setAppSessionKey(appSessionKey);
    
    Serial.println("LoRa avviato con successo");
}

void loop() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastSentTime >= INTERVAL) {
        float temperature = readTemperature();
        
        // Compattare il messaggio in binario
        byte payload[4];
        memcpy(payload, &temperature, sizeof(float));
        
        LoRa.beginPacket();
        LoRa.write(payload, sizeof(payload));
        LoRa.endPacket();
        
        Serial.print("Messaggio inviato: ");
        Serial.print(temperature);
        Serial.println("°C");
        
        lastSentTime = currentTime;
    }
}
```

>[Torna a reti LoRaWAN](lorawanclasses.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

