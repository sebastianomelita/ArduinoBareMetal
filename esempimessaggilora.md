>[Torna a reti Lorawan](lorawanclasses.md#formato-del-payload)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
  

### **Messaggi lunghi**

#### **Esempio JSON solo sensori**

```C++
{
  "device_id": "1234567890ABCDEF",
  "timestamp": "2024-05-07T12:30:45Z",
  "sensor_data": {
    "temperature": 25.5,
    "humidity": 60.2,
    "battery_voltage": 3.7
  },
  "other_information": {
    "location": {
      "latitude": 40.7128,
      "longitude": -74.0060
    },
    "signal_strength": -110
  }
}
```

#### **Esempio JSON sensori + attuatori**

```C++
{
  "device_id": "1234567890ABCDEF",
  "timestamp": "2024-05-07T12:30:45Z",
  "sensor_data": {
    "temperature": 25.5,
    "humidity": 60.2,
    "battery_voltage": 3.7
  },
  "actuator_data": {
    "led": {
      "status": "on",
      "color": "red"
    },
    "motor": {
      "status": "off",
      "speed": 0
    }
  },
  "other_information": {
    "location": {
      "latitude": 40.7128,
      "longitude": -74.0060
    },
    "signal_strength": -110
  }
}
```
### **Messaggi corti**

#### **Esempio struct sensori + attuatori**

```C++
#include <iostream>

const int MAX_STRING_LENGTH = 100; // Lunghezza massima per le stringhe

// Definizione della struttura per il payload completo
struct LoRaPayload {
    char device_id[MAX_STRING_LENGTH];
    char timestamp[MAX_STRING_LENGTH];
    float temperature;
    float humidity;
    float battery_voltage;
    char led_status[MAX_STRING_LENGTH];
    char led_color[MAX_STRING_LENGTH];
    char motor_status[MAX_STRING_LENGTH];
    int motor_speed;
    char location[MAX_STRING_LENGTH];
    int signal_strength;
};
```
#### **Esempio serializzazione sensori + attuatori con la libreria Cayenne**

Per la serializzazione dei dati utilizzando il formato Cayenne LPP (Low Power Payload), possiamo adattare la struttura LoRaPayload in modo che i dati siano organizzati secondo le specifiche di Cayenne LPP. Cayenne LPP è un formato compatto e standardizzato per la trasmissione di dati attraverso reti LPWAN (Low Power Wide Area Network), progettato per dispositivi a basso consumo energetico come i dispositivi IoT. Questo formato consente di rappresentare diversi tipi di dati (ad esempio, temperatura, umidità, tensione) in un formato ottimizzato per la trasmissione su reti LPWAN.

Ecco un esempio di come potrebbe apparire una struttura LoRaPayload con la serializzazione Cayenne LPP:
```C++
#include <iostream>

const int MAX_PAYLOAD_SIZE = 100; // Dimensione massima del payload Cayenne LPP

// Definizione della struttura per il payload Cayenne LPP
struct CayenneLPP {
    uint8_t buffer[MAX_PAYLOAD_SIZE];
    int size; // Dimensione effettiva del payload
};

// Funzione per aggiungere un valore di tipo float al payload Cayenne LPP
void addFloatToCayenneLPP(CayenneLPP &lpp, uint8_t channel, float value) {
    lpp.buffer[lpp.size++] = channel;
    lpp.buffer[lpp.size++] = 0x02; // Tipo di dati (float)
    lpp.buffer[lpp.size++] = value >> 8; // Byte più significativo del float
    lpp.buffer[lpp.size++] = value & 0xFF; // Byte meno significativo del float
}

// Funzione per aggiungere un valore di tipo int al payload Cayenne LPP
void addIntToCayenneLPP(CayenneLPP &lpp, uint8_t channel, int value) {
    lpp.buffer[lpp.size++] = channel;
    lpp.buffer[lpp.size++] = 0x02; // Tipo di dati (int)
    lpp.buffer[lpp.size++] = value >> 8; // Byte più significativo dell'intero
    lpp.buffer[lpp.size++] = value & 0xFF; // Byte meno significativo dell'intero
}

int main() {
    // Esempio di utilizzo della struttura per creare un payload Cayenne LPP
    CayenneLPP lpp;
    lpp.size = 0; // Inizializza la dimensione del payload a 0
    addFloatToCayenneLPP(lpp, 1, 25.5); // Canale 1: Temperatura
    addFloatToCayenneLPP(lpp, 2, 60.2); // Canale 2: Umidità
    addFloatToCayenneLPP(lpp, 3, 3.7);  // Canale 3: Tensione della batteria
    addIntToCayenneLPP(lpp, 101, -110); // Canale 101: Forza del segnale

    // Output dei dati del payload Cayenne LPP
    std::cout << "Cayenne LPP Payload:" << std::endl;
    for (int i = 0; i < lpp.size; ++i) {
        std::cout << std::hex << static_cast<int>(lpp.buffer[i]) << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

>[Torna a reti Lorawan](lorawanclasses.md#formato-del-payload)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
