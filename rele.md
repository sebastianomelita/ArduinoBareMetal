

>[Torna a gateway digitale](gateway.md)

## **Gateway per scheda relè digitale**

### **Schema di principio del collegamento a stella**

Notare i **collegamenti punto-punto** tra tutti i dispositivi e il microcontrollore tramite linee esclusive.

<img src="star.png" alt="alt text" width="200">

### **La scheda relè**

Il relè è un dispositivo elettromeccanico utilizzato come interruttore e frapposto tra dispositivi ad alta corrente e dispositivi a bassa corrente. Le sue parti principali sono bobina e contatti. Generalmente applichiamo tensioni CC (5-12V) alla bobina e colleghiamo il carico (230V) attraverso i contatti normalmente aperti o chiusi. Quando forniamo tensione continua attraverso la bobina, inizia a dare energia. Quando la bobina è completamente eccitata, il contatto passa dalla posizione normalmente aperta alla posizione normalmente chiusa.

<img src="rele-passo-passo-funzionamento.webp" alt="alt text" width="500">

Le lampadine o i motori di solito funzionano con alimentazione a 230 volt in caso di alimentazione in CA (corrente alternata). Non possiamo interfacciare questi carichi AC direttamente con la scheda di sviluppo ESP32. Dobbiamo usare un relè tra ESP32 e la lampada. 

Utilizzeremo un relè come interruttore per controllare lampade da 120-230 volt. Controlleremo il relè con ESP32. Potremmo usare qualsiasi modulo relè. Tuttavia, utilizzeremo il modulo relè SRD-05VDC-SL-C 5V. Funziona e può essere utilizzato per controllare carichi in uscita da 120-240 volt e una corrente massima di 10 A.

Bisogna fare attenzione a distinguere tra **carichi resistivi** e **carichi induttivi**. Una lampadina è un tipico carico resistivo, mentre un motore è un tipico carico induttivo. 

I **carichi induttivi** sono costituiti internamente da bobine più o meno grandi. I carichi induttivi soffrono del problema delle **sovratensioni** e delle **sovracorrenti** all'**accensione** e allo **spegnimento**. 

Una **sovracorrente** è una corrente che può essere **parecchie volte superiore** alla **massima corrente nominale** del **carico** (anche 10-15 volte), ha però la caratteristica di essere presente per un periodo **transitorio molto breve**. La sua presenza è un **fattore di stress** per i relè che ne accorcia la vita operativa. Esistono relè **appositamente progettati** per l'azionamento dei **carichi induttivi** ma hanno normalmente dimensioni e costo maggiore di quelli per carichi resistivi.

E' da rimarcare che normalmente non è una buona idea **collegare direttamente** la bobina di un relè ad una porta digitale del microcontrollore perchè questa potrebbe non fornire tutta la **corrente necessaria** per **eccitare competamente** il relè. Normalmente tra, microcontrollore e relè vi sono dei **circuiti di pilotaggio** che forniscono la corrente necessaria realizzati con **transistor BJT o MOS**. Una **scheda relè** normalmente ha a bordo, oltre al relè, pure i circuiti di pilotaggio necessari.

<img src="Relay-module-pinout-ESP32.png" alt="alt text" width="1000">

Una **scheda relè** consiste di **6 pin**:
- Tre pin vengono utilizzati per il collegamento con il **lato ad alta tensione**, ad esempio carichi elettrici e alimentazione a 220 volt 
- altri tre pin vengono utilizzati per il **lato a bassa tensione**, vale a dire: vcc, massa e pin del segnale di controllo.

<img src="rele.png" alt="alt text" width="1000">

Ci sono due **pin di contatto** che sono **normalmente aperti** e **normalmente chiusi**. Si usa sempre un **pin comune** insieme ad un pin normalmente aperto o un pin normalmente chiuso alternativamente per collegare il carico. A seconda di NC o NO, si utilizza un segnale attivo alto o attivo basso per controllare il relè:
- **Normalmente aperto (NA)**. Si utilizza un pin normalmente aperto, il relè si attiverà quando il segnale alto è presente sul **pin di controllo** mentre il segnale basso sullo stesso pin spegnerà il relè come mostrato in figura.
- **Normalmente chiuso (NC)**.  Si utilizza un pin normalmente chiuso, il segnale basso sul **pin di controllo** attiva il relè mentre il segnale alto sul pin di controllo disattiva il relè.

Alcuni moduli relè hanno transistor che funzionano con logica invertita. Logica invertita significa che il segnale basso attivo attiverà il relè mentre il segnale alto attivo spegnerà il relè. È necessario controllare la funzionalità del modulo relè prima di utilizzarlo. SRD-05VDC-SL-C, il modulo relè a 5V utilizzato nel progetto, non funziona su logica invertita.

### **Schema cablaggio**

**Avvertenza:** prima di realizzare questo schema elettrico, assicurarsi di non aver collegato ESP32 e lampada con alimentazione ad alta tensione. Stai utilizzando l'ALTA tensione in questo progetto, prendi tutte le precauzioni prima di creare questo layout. 

**Prima di toccare** il circuito per **inserire o disinserire collegamenti** assicurarsi con **certezza assoluta** di aver **scollegato la tensione di rete a 230V**. **Ripetere** questo controllo **con scrupolo** ogni volta che si **maneggia il circuito**.

<img src="Web-Server-control-a-Relay-module-with-ESP32.jpg" alt="alt text" width="700">

### **Librerie del progetto**

Dal **punto di vista SW** non servono librerie particolari tranne quelle per la pubblicazione dei valori tramite MQTT. 

### **Gateway MQTT per il comando di una scheda relè individuato via JSON**

La libreria MQTT è asincrona per cui non bloccante. E' adoperabile sia per **ESP8266** che per **ESP32**.

Il codice seguente, alla ricezione dei messaggi JSON, inviati via MQTT sul topic **"torele"**,  **{"rele1":"0"}** oppure **{"rele1":"1"}** scrive il bit basso o alto sulla **porta di controllo** del relè ivi collegato. **Subito dopo** la scrittura del comando viene effettuata una **lettura dello stato** della stessa porta (la cmdport) e viene inviato il suo valore al server MQTT lungo un **canale di feedback** al topic **"fromrele"**. I **comandi** vengono inviati dal server MQTT sul topic **"torele"**.

**Periodicamente**, grazie ad una **schedulazione** all'interno del loop(), il microcontrollore **invia spontaneamente** lo **stato della porta** del relè con una cadenza memorizzata su interval e impostata a **60 secondi**.

```C++
//#include <WiFiClientSecure.h>
#include <WiFi.h>       // per ESP32
//#include <ESP8266WiFi.h> per ESP8266
#include <AsyncMqttClient.h>
#include <Ticker.h>

#define WIFI_SSID "myssid"
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
#define MQTT_PUB "fromrele"
#define MQTT_SUB "torele"

//{"rele1":"0"}, {"rele1":"1"}

AsyncMqttClient mqttClient;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 60*1000;        // Interval at which to publish sensor readings
byte count = 0;
int8_t ax;
int8_t cmdport = 22;

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

void readDataAndPub(){ 
    //crea la stringa JSON
    ax = digitalRead(cmdport);
	String str = "{\"rele1\":\"";
	str += ax;
	str += "\"}";
	
    // Publish an MQTT message on topic esp32/ds18b20/temperature    
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB, 1, true, str.c_str(), str.length());                           
    Serial.print("Pubblicato sul topic %s at QoS 1, packetId: ");
    Serial.println(MQTT_PUB);
    Serial.println(packetIdPub1);
    Serial.print("Messaggio inviato: ");
    Serial.println(str); 
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
  
  //parsing della stringa JSON
  String instr = String(payload);
  if(instr.indexOf("\"rele1\":\"1\"") >= 0){
	  digitalWrite(cmdport, HIGH);
  }
  if(instr.indexOf("\"rele1\":\"0\"") >= 0){
	  digitalWrite(cmdport, LOW);
  }
  
  Serial.print("Requesting data...");
  readDataAndPub();
  Serial.println("DONE");
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
  uint16_t packetIdSub = mqttClient.subscribe(MQTT_SUB, 2);
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
  delay(2000);
  pinMode(cmdport, OUTPUT);

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
        previousMillis = currentMillis;
    
        Serial.print("Requesting data...");
	readDataAndPub();
	Serial.println("DONE");
  }
}


```
### **Gateway MQTT per il comando di una scheda relè individuato via topic MQTT**

E' anche possibile utilizzare direttamente la **struttura gerarchica dei topic** per creare un **path** che individui univocamente il **nome** di un **comando** o di uno **stato**. Ad esempio: **soggiorno/lampadario1/led4/cmd** per il comando e **soggiorno/lampadario1/led4/state** per lo stato.

Variante del codice precdente in cui, alla ricezione dei messaggi JSON, inviati via MQTT sul topic **"torele1/cmd"** **"0"** oppure **"1"** scrive il bit basso o alto sulla **porta di controllo** del relè ivi collegato. **Subito dopo** la scrittura del comando viene effettuata una **lettura dello stato** della stessa porta (la cmdport) e viene inviato il suo valore al server MQTT lungo un **canale di feedback** al topic **"fromrele1/state"**. I **comandi** vengono inviati dal server MQTT sul topic **"torele1/cmd"**. 

**Periodicamente**, grazie ad una **schedulazione** all'interno del loop(), il microcontrollore **invia spontaneamente** lo **stato della porta** del relè con una cadenza memorizzata su interval e impostata a **60 secondi**.

```C++
//#include <WiFiClientSecure.h>
#include <WiFi.h>       // per ESP32
//#include <ESP8266WiFi.h> per ESP8266
#include <AsyncMqttClient.h>
#include <Ticker.h>

#define WIFI_SSID "myssid"
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
#define MQTT_PUB "fromrele1/state"
#define MQTT_SUB "torele1/cmd"

AsyncMqttClient mqttClient;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 60*1000;        // Interval at which to publish sensor readings
byte count = 0;
int8_t ax;
int8_t cmdport = 22;

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

void readDataAndPub(){ 
        //crea la stringa JSON
        ax = digitalRead(cmdport);
	char str[2];
	
	itoa(ax,str,10);
        // Publish an MQTT message on topic esp32/ds18b20/temperature    
	uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB, 1, true, str, 1);                           
        Serial.print("Pubblicato sul topic %s at QoS 1, packetId: ");
	Serial.println(MQTT_PUB);
        Serial.println(packetIdPub1);
	Serial.print("Messaggio inviato: ");
	Serial.println(str); 
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
  
  digitalWrite(cmdport, atoi(payload));
  
  Serial.print("Requesting data...");
  readDataAndPub();
  Serial.println("DONE");
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
  uint16_t packetIdSub = mqttClient.subscribe(MQTT_SUB, 2);
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
  delay(2000);
  pinMode(cmdport, OUTPUT);

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
        previousMillis = currentMillis;
    
        Serial.print("Requesting data...");
	readDataAndPub();
	Serial.println("DONE");
  }
}
```

**Sitografia:**

- https://microcontrollerslab.com/esp32-web-server-control-relay-220-volt-lamp/
- https://www.monicoimpianti.net/rele-passo-passo/

>[Torna a gateway digitale](gateway.md)
