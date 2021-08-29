>[Torna a gateway analogico](ak811hwgatewayanalogico.md)


## **Gateway per sensore resistivo di umidità del suolo**

### **Schema di principio del collegamento a stella**

Notare i **collegamenti punto-punto** tra tutti i dispositivi e il microcontrollore tramite linee esclusive.

<img src="star.png" alt="alt text" width="200">

### **Il sensore di livello resistivo**

<img src="httpsstatics3.seeedstudio.comseeedfile2018-07bazaar881132_3.jpg" alt="alt text" width="600">

### **Schema cablaggio**

<img src="Arduino_MOSFET_Steckplatine.png" alt="alt text" width="600">

### **Condizionamento digitale**

    | Tensione di alimentazione               |        3.3V         |          5V           |
    |-----------------------------------------|---------|-----------|-----------|-----------|
    | Umido/Secco                             |  Umido  |   Secco   |   Umido   |   Secco   |
    | Sensore di umidità del suolo capacitivo |  2...8  |  14...21  | 821...824 | 490...549 |
    | Sensore di umidità del suolo resistivo  |   1024  | 483...505 |   1023    | 344...358 |
    
Teoricamente si può usare una qualsiasi di queste combinazioni, ma è necessario calibrare il  sensore prima di dichiarare che il terreno è bagnato o asciutto.

Per il sensore di umidità del suolo capacitivo si vede anche dalla tabella che la differenza nel valore del sensore per una tensione di esercizio di 3,3V è molto bassa. Pertanto per evitare probabili errori è consigliabile, per il sensore capacitivo, utilizzare una tensione operativa di 5V.

### **Sensore di umidità del suolo resistivo**

Il **sensore resistivo di umidità** del suolo è composto da 2 sonde con inserimento nel terreno. A seconda della direzione della corrente, una sonda funzionerà come catodo e l'altra come anodo. Generalmente quale sonda sia l'anodo o il catodo è irrilevante per la funzionalità del sensore, perché il sensore misura solo la resistenza ed è quindi indipendente dalla direzione del flusso di corrente.

Il circuito elettrico è chiuso sul **terreno** che funge da **resistenza** per il flusso di corrente. Questa resistenza viene misurata e dipende dalla **quantità di acqua** nel terreno perché l'acqua è un conduttore naturale per l'elettricità. Minore è la resistenza misurata, maggiore è la quantità di acqua nel terreno.

Il flusso di corrente attraverso l'anodo del sensore resistivo di umidità del suolo, che ha contatto con l'acqua, è un ambiente perfetto per l'**elettrolisi** e quindi la deposizione galvanica. Questa elettrolisi danneggia il sensore e rende il sensore impreciso. Quanto sarà forte l'elettrolisi dipende da quanto spesso e da quanta corrente viene fatta passare attraverso gli elettrodi.

Per evitare il flusso di corrente al sensore, utilizziamo un circuito MOSFET a canale N per scollegare il sensore dall'alimentazione. Inoltre leggiamo il valore del sensore solo una volta ogni tanto.

### **Librerie del progetto**

Dal **punto di vista SW** non servono librerie particolari tranne quelle per la pubblicazione dei valorri traite MQTT. Una parte del codice va comunque dedicata al condizionamento dei valori misurati dal sensore per tradurli ai valori di interesse di umidità.

### **Gateway MQTT per la lettura di un solo sensore di umidità del suolo**

Utilizziamo il pin digitale per abilitare il flusso di corrente sul gate del MOSFET. Questo chiude il circuito del sensore di umidità del suolo e leggiamo il valore del sensore analogico dopo un breve ritardo di 1 secondo. Dopo che il valore del sensore è stato stampato sul monitor seriale, aspettiamo 1 secondo e disabilitiamo il flusso di corrente attraverso il sensore tirando il Gate del MOSFET LOW.

Poiché ogni pin I/O digitale dell'ESP32, che non viene utilizzato per un'interfaccia di comunicazione, può essere un ingresso analogico, dobbiamo scegliere un pin qualsiasi come ingresso analogico.

```C++
/********************************************************
 * This demo is only supported after RUI firmware version 3.0.0.13.X on RAK811
 * Master Board Uart Receive buffer size at least 128 bytes. 
 ********************************************************/

#include "RAK811.h"
#include "SoftwareSerial.h"
#define WORK_MODE LoRaWAN   //  LoRaWAN or LoRaP2P
#define JOIN_MODE OTAA    //  OTAA or ABP
#if JOIN_MODE == OTAA
String DevEui = "8680000000000001";
String AppEui = "70B3D57ED00285A7";
String AppKey = "DDDFB1023885FBFF74D3A55202EDF2B1";
#else JOIN_MODE == ABP
String NwkSKey = "69AF20AEA26C01B243945A28C9172B42";
String AppSKey = "841986913ACD00BBC2BE2479D70F3228";
String DevAddr = "260125D7";
#endif
#define TXpin 11   // Set the virtual serial port pins
#define RXpin 10
//#define SensorPin A0  // used for Arduino and ESP8266
#define SensorPin 4     // used for ESP32
#define DebugSerial Serial
#define TX_INTERVAL ((uint32_t) 300)
#define SENSOR_VCC_PIN  5     // used for ESP32
SoftwareSerial ATSerial(RXpin,TXpin);    // Declare a virtual serial port
char buffer[]= "72616B776972656C657373";

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial,DebugSerial);

int16_t h1;

void setup() {
  DebugSerial.begin(115200);
  while(DebugSerial.available())
  {
    DebugSerial.read(); 
  }
  
  ATSerial.begin(9600); //set ATSerial baudrate:This baud rate has to be consistent with  the baud rate of the WisNode device.
  while(ATSerial.available())
  {
    ATSerial.read(); 
  }

  if(!RAKLoRa.rk_setWorkingMode(0))  //set WisNode work_mode to LoRaWAN.
  {
    DebugSerial.println(F("set work_mode failed, please reset module."));
    while(1);
  }
  
  RAKLoRa.rk_getVersion();  //get RAK811 firmware version
  DebugSerial.println(RAKLoRa.rk_recvData());  //print version number

  DebugSerial.println(F("Start init RAK811 parameters..."));
 
  if (!InitLoRaWAN())  //init LoRaWAN
  {
    DebugSerial.println(F("Init error,please reset module.")); 
    while(1);
  }

  DebugSerial.println(F("Start to join LoRaWAN..."));
  while(!RAKLoRa.rk_joinLoRaNetwork(60))  //Joining LoRaNetwork timeout 60s
  {
    DebugSerial.println();
    DebugSerial.println(F("Rejoin again after 5s..."));
    delay(5000);
  }
  DebugSerial.println(F("Join LoRaWAN success"));

  if(!RAKLoRa.rk_isConfirm(0))  //set LoRa data send package type:0->unconfirm, 1->confirm
  {
    DebugSerial.println(F("LoRa data send package set error,please reset module.")); 
    while(1);    
  }
}

bool InitLoRaWAN(void)
{
  if(RAKLoRa.rk_setJoinMode(JOIN_MODE))  //set join_mode:OTAA
  {
    if(RAKLoRa.rk_setRegion(5))  //set region EU868
    {
      if (RAKLoRa.rk_initOTAA(DevEui, AppEui, AppKey))
      {
        DebugSerial.println(F("RAK811 init OK!"));  
        return true;    
      }
    }
  }
  return false;
}

bool readSensorsAndTx() {
// Split both words (16 bits) into 2 bytes of 8
	char payload[2];

	Serial.print("Requesting data...");
	digitalWrite(SENSOR_VCC_PIN, HIGH);
	h1 = analogRead(SensorPin);
	digitalWrite(SENSOR_VCC_PIN, LOW);
	Serial.println("DONE");

	payload[0] = highByte(h1);
	payload[1] = lowByte(h1);
	
	Serial.println(F("Packet queued"));
  
	return RAKLoRa.rk_sendData(1, payload);
}

void loop() {
  DebugSerial.println(F("Start send data..."));
  if (readSensorsAndTx())
  {    
    for (unsigned long start = millis(); millis() - start < 90000L;)
    {
      String ret = RAKLoRa.rk_recvData();
      if(ret != NULL)
      { 
        DebugSerial.println(ret);
      }
      if((ret.indexOf("OK")>0)||(ret.indexOf("ERROR")>0))
      {
        DebugSerial.println(F("Go to Sleep."));
        RAKLoRa.rk_sleep(1);  //Set RAK811 enter sleep mode
        delay(TX_INTERVAL * 1000);  //delay 10s
        RAKLoRa.rk_sleep(0);  //Wakeup RAK811 from sleep mode
        break;
      }
    }
  }
}
```

### **Gateway MQTT per la lettura periodica di un sensore di umidità del suolo alimentato a batteria**

Il codice seguente utilizza la modalità di sleep profondo del microcontrollore ESP32 che consiste nello spegnimento dei due core della CPU e di tutte le periferiche fatta eccezione per un timer HW che viene impostato ad un timeout allo scadere del quale avviene il risveglio della CPU. Lo sleep profondo consente un drastico risparmio di energia nei periodi di inattività che allunga la durata di una eventuale alimentazione a batterie.

L'istruzione ``` esp_deep_sleep_start(); ``` avvia la fase di sleep e viene eseguita una volta sola all'interno del setup(). Il loop() è praticamente inutile e viene lasciato vuoto dato che non è possibile arrivare alla sua esecuzione.

Il risveglio fa ripartire ogni volta il sistema dal setup() per cui tutte le elaborazioni e le eventuali comunicazioni devono avvenire la dentro.

L'istruzione ``` esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR) ``` imposta il timeout del timer.

La funzione ``` print_wakeup_reason() ``` stampa sulla seriale il motivo del wakeup.

I motivi possono essere:
- l'evento di timer HW, che sveglia il sistema in periodi di tempo prestabiliti;
- l'evento di tocco tattile di un certo pin;
- un evento di riattivazione esterna: è possibile utilizzare una riattivazione esterna o più risvegli esterni diversi;
- un evento generato dal coprocessore ULP ma questo non sarà trattato nella presente guida.

Nel codice seguente vengono effettuati alcuni tentativi di riconnessione in caso di mancato collegamento del WiFi o di mancata connessione MQTT. Fallito il numero massimo di tentativi si va in sleep profondo e si riprova al prossimo risveglio.

```C++
#include <arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//#include <ttn_credentials.h>
#define TTN_APPEUI {0}
#define TTN_DEVEUI {0}
#define TTN_APPKEY {0}
#define MAX_BANDS 4
//#define CFG_LMIC_EU_like 0

bool GOTO_DEEPSLEEP = false;

// rename ttn_credentials.h.example to ttn_credentials.h and add you keys
static const u1_t PROGMEM APPEUI[8] = TTN_APPEUI;
static const u1_t PROGMEM DEVEUI[8] = TTN_DEVEUI;
static const u1_t PROGMEM APPKEY[16] = TTN_APPKEY;
void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }

//static uint8_t payload[5];
static osjob_t sendjob;
bool flag_TXCOMPLETE = false;

// Schedule TX every this many seconds
// Respect Fair Access Policy and Maximum Duty Cycle!
// https://www.thethingsnetwork.org/docs/lorawan/duty-cycle.html
// https://www.loratools.nl/#/airtime
const unsigned TX_INTERVAL = 30;

// payload to send to TTN gateway


// Saves the LMIC structure during DeepSleep
RTC_DATA_ATTR lmic_t RTC_LMIC;

#define PIN_LMIC_NSS 18
#define PIN_LMIC_RST 14
#define PIN_LMIC_DIO0 26
#define PIN_LMIC_DIO1 33
#define PIN_LMIC_DIO2 32
//sensors defines
//#define SensorPin A0  // used for Arduino and ESP8266
#define SensorPin 4     // used for ESP32
#define SENSOR_VCC_PIN  5     // used for ESP32

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = PIN_LMIC_NSS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = PIN_LMIC_RST,
    .dio = {PIN_LMIC_DIO0, PIN_LMIC_DIO1, PIN_LMIC_DIO2},
};

int16_t h1;

// https://github.com/mcci-catena/arduino-lmic/blob/89c28c5888338f8fc851851bb64968f2a493462f/src/lmic/lmic.h#L233

void PrintRuntime()
{
    long seconds = millis() / 1000;
    Serial.print("Runtime: ");
    Serial.print(seconds);
    Serial.println(" seconds");
}

void PrintLMICVersion()
{
    Serial.print(F("LMIC: "));
    Serial.print(ARDUINO_LMIC_VERSION_GET_MAJOR(ARDUINO_LMIC_VERSION));
    Serial.print(F("."));
    Serial.print(ARDUINO_LMIC_VERSION_GET_MINOR(ARDUINO_LMIC_VERSION));
    Serial.print(F("."));
    Serial.print(ARDUINO_LMIC_VERSION_GET_PATCH(ARDUINO_LMIC_VERSION));
    Serial.print(F("."));
    Serial.println(ARDUINO_LMIC_VERSION_GET_LOCAL(ARDUINO_LMIC_VERSION));
}

void onEvent(ev_t ev)
{
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev)
    {
    case EV_SCAN_TIMEOUT:
        Serial.println(F("EV_SCAN_TIMEOUT"));
        break;
    case EV_BEACON_FOUND:
        Serial.println(F("EV_BEACON_FOUND"));
        break;
    case EV_BEACON_MISSED:
        Serial.println(F("EV_BEACON_MISSED"));
        break;
    case EV_BEACON_TRACKED:
        Serial.println(F("EV_BEACON_TRACKED"));
        break;
    case EV_JOINING:
        Serial.println(F("EV_JOINING"));
        break;
    case EV_JOINED:
        Serial.println(F("EV_JOINED"));
        {
            u4_t netid = 0;
            devaddr_t devaddr = 0;
            u1_t nwkKey[16];
            u1_t artKey[16];
            LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
            Serial.print("netid: ");
            Serial.println(netid, DEC);
            Serial.print("devaddr: ");
            Serial.println(devaddr, HEX);
            Serial.print("artKey: ");
            for (size_t i = 0; i < sizeof(artKey); ++i)
            {
                Serial.print(artKey[i], HEX);
            }
            Serial.println("");
            Serial.print("nwkKey: ");
            for (size_t i = 0; i < sizeof(nwkKey); ++i)
            {
                Serial.print(nwkKey[i], HEX);
            }
            Serial.println("");
        }
        // Disable link check validation (automatically enabled
        // during join, but because slow data rates change max TX
        // size, we don't use it in this example.
        LMIC_setLinkCheckMode(0);
        break;
    /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
    case EV_JOIN_FAILED:
        Serial.println(F("EV_JOIN_FAILED"));
        break;
    case EV_REJOIN_FAILED:
        Serial.println(F("EV_REJOIN_FAILED"));
        break;
    case EV_TXCOMPLETE:
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        if (LMIC.txrxFlags & TXRX_ACK)
            Serial.println(F("Received ack"));
        if (LMIC.dataLen)
        {
            Serial.print(F("Received "));
            Serial.print(LMIC.dataLen);
            Serial.println(F(" bytes of payload"));
        }
        GOTO_DEEPSLEEP = true;
        break;
    case EV_LOST_TSYNC:
        Serial.println(F("EV_LOST_TSYNC"));
        break;
    case EV_RESET:
        Serial.println(F("EV_RESET"));
        break;
    case EV_RXCOMPLETE:
        // data received in ping slot
        Serial.println(F("EV_RXCOMPLETE"));
        break;
    case EV_LINK_DEAD:
        Serial.println(F("EV_LINK_DEAD"));
        break;
    case EV_LINK_ALIVE:
        Serial.println(F("EV_LINK_ALIVE"));
        break;
    /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
    case EV_TXSTART:
        Serial.println(F("EV_TXSTART"));
        break;
    case EV_TXCANCELED:
        Serial.println(F("EV_TXCANCELED"));
        break;
    case EV_RXSTART:
        /* do not print anything -- it wrecks timing */
        break;
    case EV_JOIN_TXCOMPLETE:
        Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
        break;
    default:
        Serial.print(F("Unknown event: "));
        Serial.println((unsigned)ev);
        break;
    }
}

void do_send(osjob_t *j)
{
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND)
    {
        Serial.println(F("OP_TXRXPEND, not sending"));
    }
    else
    {
        // Prepare upstream data transmission at the next possible time.
        byte payload[2];

		Serial.print("Requesting data...");
		digitalWrite(SENSOR_VCC_PIN, HIGH);
		h1 = analogRead(SensorPin);
		digitalWrite(SENSOR_VCC_PIN, LOW);
		Serial.println("DONE");

		payload[0] = highByte(h1);
		payload[1] = lowByte(h1);
	
		Serial.println(F("Packet queued"));

        // prepare upstream data transmission at the next possible time.
        // transmit on port 1 (the first parameter); you can use any value from 1 to 223 (others are reserved).
        // don't request an ack (the last parameter, if not zero, requests an ack from the network).
        // Remember, acks consume a lot of network resources; don't ask for an ack unless you really need it.
        LMIC_setTxData2(1, payload, sizeof(payload)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void SaveLMICToRTC(int deepsleep_sec)
{
    Serial.println(F("Save LMIC to RTC"));
    RTC_LMIC = LMIC;

    // ESP32 can't track millis during DeepSleep and no option to advanced millis after DeepSleep.
    // Therefore reset DutyCyles

    unsigned long now = millis();

    // EU Like Bands
#if defined(CFG_LMIC_EU_like2)//era CFG_LMIC_EU_like ma non funziona
    Serial.println(F("Reset CFG_LMIC_EU_like band avail"));
    for (int i = 0; i < MAX_BANDS; i++)
    {
        ostime_t correctedAvail = RTC_LMIC.bands[i].avail - ((now / 1000.0 + deepsleep_sec) * OSTICKS_PER_SEC);
        if (correctedAvail < 0)
        {
            correctedAvail = 0;
        }
        RTC_LMIC.bands[i].avail = correctedAvail;
    }

    RTC_LMIC.globalDutyAvail = RTC_LMIC.globalDutyAvail - ((now / 1000.0 + deepsleep_sec) * OSTICKS_PER_SEC);
    if (RTC_LMIC.globalDutyAvail < 0)
    {
        RTC_LMIC.globalDutyAvail = 0;
    }
#else
    Serial.println(F("No DutyCycle recalculation function!"));
#endif
}

void LoadLMICFromRTC()
{
    Serial.println(F("Load LMIC from RTC"));
    LMIC = RTC_LMIC;
}

void GoDeepSleep()
{
    Serial.println(F("Go DeepSleep"));
    PrintRuntime();
    Serial.flush();
    esp_sleep_enable_timer_wakeup(TX_INTERVAL * 1000000);
    esp_deep_sleep_start();
}

void setup()
{
    Serial.begin(115200);
    pinMode(SENSOR_VCC_PIN, OUTPUT);
	
    Serial.println(F("Starting DeepSleep test"));
    PrintLMICVersion();

    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    if (RTC_LMIC.seqnoUp != 0)
    {
        LoadLMICFromRTC();
    }

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop()
{
    static unsigned long lastPrintTime = 0;

    os_runloop_once();

    const bool timeCriticalJobs = os_queryTimeCriticalJobs(ms2osticksRound((TX_INTERVAL * 1000)));
    if (!timeCriticalJobs && GOTO_DEEPSLEEP == true && !(LMIC.opmode & OP_TXRXPEND))
    {
        Serial.print(F("Can go sleep "));
        SaveLMICToRTC(TX_INTERVAL);
        GoDeepSleep();
    }
    else if (millis() - lastPrintTime > 2000)
    {
        Serial.print(F("Cannot sleep "));
        Serial.print(F("TimeCriticalJobs: "));
        Serial.print(timeCriticalJobs);
        Serial.print(" ");

        PrintRuntime();
        lastPrintTime = millis();
    }
}
```


**Sitografia:**

- https://diyi0t.com/soil-moisture-sensor-tutorial-for-arduino-and-esp8266
- https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
- https://randomnerdtutorials.com/esp32-timer-wake-up-deep-sleep/

>[Torna a gateway analogico](ak811hwgatewayanalogico.md)
