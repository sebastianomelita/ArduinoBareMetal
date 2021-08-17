

>[Torna a gateway digitale](gateway.md)

## **Gateway per scheda relè digitale**

### **Schema di principio del collegamento a stella**

Notare i **collegamenti punto-punto** tra tutti i dispositivi e il microcontrollore tramite linee esclusive.

<img src="star.png" alt="alt text" width="200">

### **La scheda relè**

Il relè è un dispositivo elettromeccanico utilizzato come interruttore e frapposto tra dispositivi ad alta corrente e dispositivi a bassa corrente. Le sue parti principali sono bobina e contatti. Generalmente applichiamo tensioni cc (5-12V) alla bobina e colleghiamo il carico (240V) attraverso i contatti normalmente aperti o chiusi. Quando forniamo tensione continua attraverso la bobina, inizia a dare energia. Quando la bobina è completamente eccitata, il contatto passa dalla posizione normalmente aperta alla posizione normalmente chiusa.

<img src="rele-passo-passo-funzionamento.webp" alt="alt text" width="600">

E' da rimarcare che normalmente non è una buona idea collegare direttamente la bobina di un relè ad una porta digitale del microcontrollore perchè questa potrebbe non fornire tutta la corrente necessaria per eccitare competamente il relè. Normalmente tra, microcontrollore e relè vi sono ei circuiti di pilotaggio che forniscono la corrente necessaria realizzati con transistor BJT o MOS. Una scheda relè normalmente ha a bordo, oltre al relè, pure i circuiti di pilotaggio necessari.

<img src="Relay-module-pinout-ESP32.png" alt="alt text" width="1000">

Una scheda relè consiste di 6 pin. Tre pin vengono utilizzati per il collegamento con il lato ad alta tensione, ad esempio carichi elettrici e alimentazione a 220 volt e altri tre pin vengono utilizzati per il lato a bassa tensione, vale a dire: vcc, massa e pin del segnale di controllo.

<img src="rele.png" alt="alt text" width="1000">

### **Schema cablaggio**

<img src="Soil-Moisture-Sensor-ESP32-NodeMCU_Steckplatine.png" alt="alt text" width="400">

### **Condizionamento digitale**

    | Tensione di alimentazione               |        3.3V         |          5V           |
    |-----------------------------------------|---------|-----------|-----------|-----------|
    | Umido/Secco                             |  Umido  |   Secco   |   Umido   |   Secco   |
    | Sensore di umidità del suolo capacitivo |  2...8  |  14...21  | 821...824 | 490...549 |
    | Sensore di umidità del suolo resistivo  |   1024  | 483...505 |   1023    | 344...358 |
    
Teoricamente si può usare una qualsiasi di queste combinazioni, ma è necessario calibrare il  sensore prima di dichiarare che il terreno è bagnato o asciutto.

Per il sensore di umidità del suolo capacitivo si vede anche dalla tabella che la differenza nel valore del sensore per una tensione di esercizio di 3,3V è molto bassa. Pertanto per evitare probabili errori è consigliabile, per il sensore capacitivo, utilizzare una tensione operativa di 5V.

### **Librerie del progetto**

Dal **punto di vista SW** non servono librerie particolari tranne quelle per la pubblicazione dei valorri traite MQTT. Una parte del codice va comunque dedicata al condizionamento dei valori misurati dal sensore per tradurli ai valori di interesse di umidità.

### **Gateway MQTT per la lettura di un solo sensore di umidità del suolo**

La libreria MQTT è asincrona per cui non bloccante. E' adoperabile sia per **ESP8266** che per **ESP32**.

```C++
//#include <WiFiClientSecure.h>

//#include <ESP8266WiFi.h> per ESP8266
#include <AsyncMqttClient.h>
#include <Ticker.h>
#include <WiFi.h>       // per ESP32

// Raspberry Pi Mosquitto MQTT Broker
//#define MQTT_HOST IPAddress(192, 168, 1, 254)
#define MQTT_HOST "test.mosquitto.org"
// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
#define MQTT_PORT 1883

#define WIFI_SSID "myssid"
#define WIFI_PASSWORD "mypsw"

//Temperature MQTT Topic
#define MQTT_PUB "esp/umiditasuolo/"
//#define SensorPin A0  // used for Arduino and ESP8266
#define SensorPin 4     // used for ESP32

Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

String datastr = "";

AsyncMqttClient mqttClient;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 2000;        // Interval at which to publish sensor readings
byte count = 0;

unsigned long previusMillis = 0;
bool sensor1 = false;
float t1, h1;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      wifiReconnectTimer.once_ms(2000, connectToWifi);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once_ms(2000, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  //mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  connectToWifi();
  count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10) {
    delay(500);
    count++;
    Serial.print(".");
  }
}

void packData(String &str){    
	str = "{\"humidity1\":\"";
	str += h1;
	str += "\"}";
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
	  
	  Serial.print("Requesting data...");
	  h1 = analogRead(SensorPin);
	  Serial.println("DONE");
	  
	  packData(datastr);
	    
          // Publish an MQTT message on topic esp32/ds18b20/temperature    
	  uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB, 1, true, datastr.c_str(), datastr.length());                        
          Serial.print("Pubblicato sul topic %s at QoS 1, packetId: ");
	  Serial.println(MQTT_PUB);
          Serial.println(packetIdPub1);
	  Serial.print("Messaggio inviato: ");
	  Serial.println(datastr); 
  }
}

```

**Sitografia:**

- https://microcontrollerslab.com/esp32-web-server-control-relay-220-volt-lamp/
- https://www.monicoimpianti.net/rele-passo-passo/

>[Torna a gateway digitale](gateway.md)
