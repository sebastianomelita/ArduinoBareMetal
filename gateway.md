>[Torna all'indice generale](index.md)
## **Gateways per BUS di sensori**

1. [BUS Dallas](gatewaydallas.md)
2. [BUS I2C](gatewayi2c.md)
3. [BUS spi](gatewayspi.md)
4. [BUS modbus](gatewaymodbus.md)

### **Client MQTT per ESP32 con metodo di connessione bloccante**
Utilizza la libreria **arduino-mqtt** scaricabille da https://github.com/256dpi/arduino-mqtt e adatta sia per esp8266 che per esp32. Ha la particolarità (comune a quasi tutte le librerie MQTT) di avere la **funzione di connessione bloccante**, per cui, se il server è momentaneamente indisponibile, il client rimane fermo sulla funzione di connessione senza ritornare. Scaduto un **timeout** la connect() **ritorna** permettendo al task che la contiene di continuare l'esecuzione. Nell'esempio seguente la connect() è dentro la callback di un timer HW ed è attivarta da una ISR associata ad un interrupt. Le ISR, per definizione, dovrebbero essere non bloccanti perchè spesso non sono interrompibili. **Bloccare una ISR** collegata ad **eventi core** della macchina può causare **instabilità** che determinano il **reset** del sistema.

In realtà, il task principale che contiene il loop() su **ESP32** non si blocca perchè sta su un thread diverso da quello in cui sta la connect MQTT (probabilmente anche un'altro core della CPU). Nel caso di una CPU single core come ESP8266 il codice potrebbe essere modificato inserendo nella callback del timer un **flag** al posto della connect e nel loop un **check del flag** che chiama la connect se questo è vero. In questo modo verrebbe bloccato solo il loop principale per qualche secondo, tempo in cui il sistema è non responsivo ma comunque funzionante. Oppure si potrebbe sostituire il timer HW che comanda la riconnessione periodica del client MQTT con un timer SW basato sul polling della millis() nel loop.

Le **librerie** vanno scaricate e scompattate dentro la cartella **libraries** all'interno della **cartella di lavoro** di Arduino. Le librerie da scaricare devono avere il nome (eventualmente rinominandole): 
- arduino-mqtt

Si tratta di una libreria **molto leggera** che non occupa spazio eccessivo in memoria istruzioni.

```C++
//#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <MQTT.h>
#include <Ticker.h>

#define WIFIRECONNECTIME  2000
#define MQTTRECONNECTIME  2000

Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

const char ssid[] = "myssd";
const char pass[] = "mypsw";
const char mqttserver[] = "test.mosquitto.org";

//WiFiClientSecure net;
WiFiClient net;
MQTTClient mqttClient(1024);

unsigned long lastMillis = 0;
byte count = 0;

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      mqttReconnectTimer.attach_ms(MQTTRECONNECTIME, mqttConnTest);
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      wifiReconnectTimer.once_ms(WIFIRECONNECTIME, connectToWifi);
      break;
  }
}

void mqttConnTest() {
    if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
	Serial.print("MQTT lastError: ");
	Serial.println(mqttClient.lastError());
	connectToMqtt();
    }
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  WiFi.begin(ssid, pass);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect("bside2botham2", "try", "try");
  if(mqttClient.connected()){
	mqttClient.subscribe("/hello");
  }
  Serial.println("...end");
  // client.unsubscribe("/hello");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  WiFi.onEvent(WiFiEvent);
  // MQTT brokers usually use port 8883 for secure connections.
  mqttClient.begin(mqttserver, 1883, net);
  mqttClient.onMessage(messageReceived); 
  connectToWifi();
  count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10) {
    delay(500);
    count++;
    Serial.print(".");
  }
}

void loop() {
  mqttClient.loop();
  //delay(10);  // <- fixes some issues with WiFi stability

  // publish a message roughly every 2 second.
  if (millis() - lastMillis > 2000) {
        lastMillis = millis();
	
	Serial.println("Tick");
	// pubblicazione periodica di test
        mqttClient.publish("/hello", "world");
  }
}

/*
WL_CONNECTED: assigned when connected to a WiFi network;
WL_NO_SHIELD: assigned when no WiFi shield is present;
WL_IDLE_STATUS: it is a temporary status assigned when WiFi.begin() is called and remains active until the number of attempts expires (resulting in WL_CONNECT_FAILED) or a connection is established (resulting in WL_CONNECTED);
WL_NO_SSID_AVAIL: assigned when no SSID are available;
WL_SCAN_COMPLETED: assigned when the scan networks is completed;
WL_CONNECT_FAILED: assigned when the connection fails for all the attempts;
WL_CONNECTION_LOST: assigned when the connection is lost;
WL_DISCONNECTED: assigned when disconnected from a network;
\*/
```

Versione per **ESP8266** con un **flag** che attiva la riconnessione all'interno del loop(). Il **loop()** rimane **bloccato**per svariati secondi (circa 18) se il server MQTT è **non disponibile** o non raggingibile. Il timer di ritrasmissione MQTT non esegue azioni bloccanti ma si limita ad **impostare il flag**. Il flag viene **controllato** con un **polling** continuo all'interno del loop().

```C++
//#include <WiFiClientSecure.h>
//#include <WiFi.h>       // per ESP32
#include <ESP8266WiFi.h>  // per ESP8266
#include <MQTT.h>
#include <Ticker.h>

#define WIFIRECONNECTIME  2000
#define MQTTRECONNECTIME  2000

Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

const char ssid[] = "myssid";
const char pass[] = "mypsw";
const char mqttserver[] = "test.mosquitto.orge";

//WiFiClientSecure net;
WiFiClient net;
MQTTClient mqttClient(1024);

unsigned long lastMillis = 0;
byte count = 0;
bool mqttreconnflag = false; // 8 bit, non è necessaria una corsa critica!

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      mqttReconnectTimer.attach_ms(MQTTRECONNECTIME, mqttConnTest2);
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      wifiReconnectTimer.once_ms(WIFIRECONNECTIME, connectToWifi);
      break;
  }
}

void mqttConnTest() {
	if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
		Serial.print("MQTT lastError: ");
		Serial.println(mqttClient.lastError());
		connectToMqtt();
    }
}

void mqttConnTest2() {
	mqttreconnflag = true;
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  WiFi.begin(ssid, pass);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect("bside2botham2", "try", "try");
  if(mqttClient.connected()){
	mqttClient.subscribe("/hello");
  }
  Serial.println("...end");
  // client.unsubscribe("/hello");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  WiFi.onEvent(WiFiEvent);
  // MQTT brokers usually use port 8883 for secure connections.
  mqttClient.begin(mqttserver, 1883, net);
  mqttClient.onMessage(messageReceived); 
  connectToWifi();
  count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10) {
    delay(500);
    count++;
    Serial.print(".");
  }
}

void loop() {
  mqttClient.loop();
  //delay(10);  // <- fixes some issues with WiFi stability
  // polling del flag di riconnessione MQTT
  if(mqttreconnflag){
	  mqttreconnflag = false;
	  mqttConnTest();
  }
  
  // publish a message roughly every 2 second.
  if (millis() - lastMillis > 2000) {
        lastMillis = millis();
	
	Serial.println("Tick");
	// pubblicazione periodica di test
        mqttClient.publish("/hello", "world");
  }
}

/*
WL_CONNECTED: assigned when connected to a WiFi network;
WL_NO_SHIELD: assigned when no WiFi shield is present;
WL_IDLE_STATUS: it is a temporary status assigned when WiFi.begin() is called and remains active until the number of attempts expires (resulting in WL_CONNECT_FAILED) or a connection is established (resulting in WL_CONNECTED);
WL_NO_SSID_AVAIL: assigned when no SSID are available;
WL_SCAN_COMPLETED: assigned when the scan networks is completed;
WL_CONNECT_FAILED: assigned when the connection fails for all the attempts;
WL_CONNECTION_LOST: assigned when the connection is lost;
WL_DISCONNECTED: assigned when disconnected from a network;
\*/
```

### **Client MQTT per ESP32 e ESP8266 con metodo di connessione non bloccante**
Utilizza la libreria **sync-mqtt-client** scaricabile da https://github.com/marvinroger/async-mqtt-client e adatta sia per esp8266 che per esp32. In questo caso la funzione di riconnessione del client MQTT è non bloccante e non crea problemi anche se viene chiamata all'interno della ISR di un timer HW. Il prezzo da pagare è la sostituzione della sottostante libreria TCP bloccante con una versione analoga asincrona che ritorna sempre. L'installazione di librerie aggiuntiva comporta una **occupazione** di memoria in area istruzioni **maggiore della soluzione** precedente.

Le **librerie** vanno scaricate e scompattate dentro la cartella **libraries** all'interno della **cartella di lavoro** di Arduino. Le librerie da scaricare devono avere il nome (eventualmente rinominandole): 
- async-mqtt-client
- AsyncTCP


```C++
//#include <WiFiClientSecure.h>
#include <WiFi.h>       // per ESP32
//#include <ESP8266WiFi.h> per ESP8266
#include <AsyncMqttClient.h>
#include <Ticker.h>

#define WIFI_SSID "myssd"
#define WIFI_PASSWORD "mypsw"
#define WIFIRECONNECTIME  2000
#define MQTTRECONNECTIME  2000

Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
// Raspberry Pi Mosquitto MQTT Broker
//#define MQTT_HOST IPAddress(192, 168, 1, 254)
#define MQTT_HOST "test.mosquitto.org"
// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
#define MQTT_PORT 1883
// Temperature MQTT Topic
#define MQTT_PUB_TEMP "hello"

AsyncMqttClient mqttClient;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 4000;        // Interval at which to publish sensor readings
byte count = 0;

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

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  Serial.print("  payload: ");
  Serial.println(payload);
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
      wifiReconnectTimer.once_ms(WIFIRECONNECTIME, connectToWifi);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("hello", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once_ms(MQTTRECONNECTIME, connectToMqtt);
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
  mqttClient.onMessage(onMqttMessage);
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

void loop() {
  unsigned long currentMillis = millis();
  // Every X number of seconds it publishes a new MQTT message
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    
    Serial.println("Tick");
	
    // Publish an MQTT message on topic 
    //(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0);
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, "Ciao mondo", strlen("Ciao mondo"));                            
    Serial.printf("Pubblicato sul topic %s at QoS 1, packetId: ", MQTT_PUB_TEMP);
    Serial.println(packetIdPub1);
    Serial.print("Messaggio inviato: ");
    Serial.println("Ciao mondo");
  }
}


```

>[Torna all'indice generale](index.md)


