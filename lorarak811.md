>[Torna all'indice generale](index.md)
## **Gateways per reti di sensori con modem LoRaWan HW con il modulo RAK811** !!! NON ANCORA FINITO !!!
BUS cablato:
1. [Dallas](lorawanswdallas.md)
2. [I2C](gatewayi2clorasw.md)
3. [SPI](gatewayspi.md)
4. [Modbus](gatewaymodbus.md)

Stella cablato (punto-punto):
1. [Analogico](ak811hwgatewayanalogico.md)
2. [Digitale](ak811hwgatewaydigitale.md)

Radio punto-punto:
1. [Analogico](gatewayradioa.md)
2. [Digitale](gatewayradiod.md)


Per una discussione sintetica di tutti i tipi di BUS semplici dal punto di vista generale si rimanda a [Cablati semplici](cablatisemplici.md ).

### **La scheda LoRa RAK811**

<img src="hwiotacrchitecturerak811.png" alt="alt text" width="1000">

E' un **modem** che implementa **in HW** tutto lo **stack LoraWan** ed è configurabile via seriale mediante i comandi AT.

Il modulo di breakout LPWAN RAK811 fornisce: scheda con terminali + transceiver Semtech SX1276 + MCU STM32L151.  

E' utilizzabile:
- come **modem HW** accessibile via UART tramite ccomandi AT gestibili anche tramite una libreria per Arduino. Sia i comandi AT che la libreria permettono l'accesso ai GPIO di cui è dotata tramite un'intestazione aggiuntiva ai messaggi. Il vantaggio di questa configurazione è che il codice della MCU risulta non appesatito dall'implementazione dell'intero stack protocollare LoRaWan e quindi con più risorse disponibili da dedicare alla logica dell'applicazione.
- come **MCU** su cui caricare il FW con lo **stack** LoRaWan **completo** (**LMIC**) oppure un FW con uno **stack** di comunicazione **minimale** per la modalità **P2P** tra **coppie** di dispositivi. Il vantaggio di questa configurazione è che un nodo **non ha** bisogno di **MCU aggiuntive** per gestire la **logica** dell'applicazione con conseguente risparmio di dimensioni e consumi.

### **Schema cablaggio**

Collegamento di una scheda modem RAK811 con una scheda MCU **ESP32**.

<img src="EMPaUn3XkAA6bUY.jfif" alt="alt text" width="700">

La lunghezza del filo dipende dalla frequenza:
- 868 MHz: 86,3 mm 
- 915 MHz: 81,9 mm
- 433 MHz: 173,1 mm

### **Classi di servizio**

**Classe A**

La **trasmissione** del nodo in uplink è **asincrona** cioè i messaggi, dal nodo al gateway, possono essere inviati in qualsiasi momento. 

Dopo la trasmissione il nodo apre **due finestre di ricezione**. Il Network server **può rispondere** tramite un gateway con un messaggio in Downlink in una delle due finestre. Solitamente **la prima** finestra è aperta sullo stesso canale utilizzato nella trasmissione in Uplink, mentre **la seconda** finestra viene aperta su un canale differente, accordato in precedenza con il Network Server, per migliorare la resistenza alle interferenze.

**Classe B**

**Periodicamente**, ogni 128 secondi, un **nodo** riceve dal **gateway** un beacon per la **sincronizzazione** (tra nodo e gateway) e per la **schedulazione** delle ricezioni. Un **pacchetto beacon** contiene uno specifico tempo di riferimento in cui far aprire ai nodi della rete una **finestra di ricezione extra**, chiamata **ping slot**. In pratica è una multiplazione TDMA con cui, ad ogni beacon, il network server riserva uno slot temporale ai vari nodi afferenti ad un certo gateway. La posizione dello slot, nella trama tra un beacon e l'altro, viene ricalcolata ad ogni nuovo beacon per evitare errori di ricezione **sistematici**. 

La ricezione è affetta da una **latenza** piuttosto variabile e potenzialmente elevata. In compenso, il **consumo** è ancora puttosto ridotto perchè, ad ogni trama, la finestra di ricezione è una sola e molto breve.

La **trasmissione** in uplink, analogamente alla classe A, è ancora asincrona.

**Classe C**

La **finestra** di ricezione, quando il nodo non trasmette, è **sempre aperta** per cui la trasmissione in downlink da parte del network server può avvenire, in maniera completamente **asincrona**, in qualsiasi momento. La finestra di ricezione è **chiusa** soltanto durante la trasmissione. In questa modalità la **latenza** è minima ma il **consumo** è massimo per cui si tratta di un modo di funzionamento adatto a dispositivi perennemente connessi ad una fonte di **alimentazione persistente** (rete elettrica, panneli solari).

**ALOHA**

In tutte le classi l'**accesso multiplo** al canale in **uplink** è un TDM a contesa random di tipo **ALOHA**.

<img src="classilorawan.png" alt="alt text" width="900">

### **Modi di autenticazione**

**Attivazione via etere (OTAA)**

L'**attivazione over-the-air (OTAA)** è il modo preferito e più sicuro per connettersi con The Things Network. I dispositivi eseguono una procedura di unione con la rete (join), durante la quale viene assegnato un **DevAddr** dinamico e le **chiavi** di sicurezza vengono **negoziate** con il dispositivo.

**Attivazione tramite personalizzazione (ABP)**

In alcuni casi potrebbe essere necessario codificare il **DevAddr** e le **chiavi di sicurezza** hardcoded nel dispositivo. Ciò significa attivare un dispositivo **tramite personalizzazione (ABP)**. Questa strategia potrebbe sembrare più semplice, perché si salta la procedura di adesione, ma presenta alcuni svantaggi legati alla sicurezza.

**ABP vs OTAA**

In generale, non ci sono inconvenienti nell'utilizzo dell'OTAA rispetto all'utilizzo dell'ABP, ma ci sono alcuni requisiti che devono essere soddisfatti quando si utilizza l'OTAA.La specifica LoRaWAN avverte in modo specifico contro il ricongiungimento sistematico in caso di guasto della rete. Un dispositivo dovrebbe conservare il risultato di un'attivazione in una memoria permanente se si prevede che il dispositivo venga spento e riacceso durante la sua vita:
- un dispositivo ABP utilizza una memoria non volatile per mantenere i contatori di frame tra i riavvii. 
- Un approccio migliore sarebbe passare all'utilizzo di OTAA e memorizzare la sessione OTAA anziché i contatori di frame.

L'unica cosa da tenere a mente è che un join OTAA richiede che il dispositivo finale si trovi all'interno della copertura della rete su cui è registrato. La ragione di ciò è che la procedura di join OTAA richiede che il dispositivo finale sia in grado di ricevere il messaggio di downlink Join Accept dal server di rete.

Un approccio migliore consiste nell'eseguire un join OTAA in una fabbrica o in un'officina in cui è possibile garantire la copertura di rete e i downlink funzionanti. Non ci sono svantaggi in questo approccio finché il dispositivo segue le migliori pratiche LoRaWAN (https://www.thethingsindustries.com/docs/devices/best-practices/).

### **Buone pratiche**

**Connessioni confermate**

È possibile che non si riceva subito un ACK per ogni uplink o downlink di tipo confermato. Una buona regola empirica è attendere almeno tre ACK mancati per presumere la perdita del collegamento.

In caso di perdita del collegamento, procedere come segue:
- Imposta la potenza TX al massimo consentito/supportato e riprova
- Diminuisci gradualmente la velocità dei dati e riprova
- Ripristina i canali predefiniti e riprova
- Invia richieste di adesione periodiche con backoff

**Cicli di alimentazione**

I dispositivi dovrebbero salvare i parametri di rete tra i cicli di alimentazione regolari. Ciò include parametri di sessione come DevAddr, chiavi di sessione, FCnt e nonces. Ciò consente al dispositivo di unirsi facilmente, poiché chiavi e contatori rimangono sincronizzati.

### **Architettura di riferimento per IoT**

L'**architettura tipica del SW** a bordo di un dispositivo IoT è riassumibile:

<img src="iotarchitecture.png" alt="alt text" width="600">

Il **middleware** in genere è composto da **librerie di terze parti** da **compilare** all'interno di un **IDE** (ad es. Arduino) o all'interno di un **SDK** cioè un pacchetto di sviluppo per applicazioni che fornisce vari strumenti per compilazione, debug e documentazione (ad es. AWS IoT, Azure IoT, ESP-IDF). Oppure esistono **framework** specifici per IoT Open Source come RIOT che, una volta compilati su una macchina Linux, forniscono veri e propri **SO per IoT** con esattamente ciò che serve per la **comunicazione** di un certo dispositivo.

### **Librerie del progetto**

In questo caso gran parte delle funzioni del middleware, quelle relative alla comunicazione via stack LoRaWan, è implementato all'interno del chip del modem. Il middleware nel sistema a microprocessore (MCU) stavolta svolge ben poche funzioni di comunicazione. 

Dal **punto di vista SW** seve **4 librerie** da scaricare dentro la solita cartella **libraries**:
- **Arduino-RAK811-Library**. Si scarica da https://github.com/RAKWireless/WisNode-Arduino-Library come WisNode-Arduino-Library-master.zip, poi si  scompatta in una cartella sul desktop da cui si copia la cartella **Arduino-RAK811-Librarye** che va messa nela libraries dell'IDE di Arduino.

La libreria dovrebbero supportare solamente le classi di servizio A e B (C esclusa).

**Gestione della ricezione**

<img src="Beacon.png" alt="alt text" width="800">

### **Gateway LoraWan con OTAA join**

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
	h1 = analogRead(SensorPin);
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

### **Gateway LoraWan con OTAA join e deepSleep**

La **memoria RTC** (Real Time Clock) è un'area della SRAM del processore che rimane alimentata e accessibile alle funzioni RTC del microcontrollore ESP32 e del coprocessore ULP anche quando è attivato lo standby. Nell'esempio sottostante la memoria RTC viene utilizzata per memorizzare un flag di avvenuta conessione. I parametri della connessione (chiave di sessione) vengono recuperati dal modem se questo era stato precedentemente associata (join) al gateway LoRaWan con successo.

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
//#define SensorPin A0  // used for Arduino and ESP8266
#define SensorPin 4     // used for ESP32
#define TXpin 11   // Set the virtual serial port pins
#define RXpin 10
#define TX_INTERVAL ((uint32_t) 300)
SoftwareSerial ATSerial(RXpin,TXpin);    // Declare a virtual serial port
char buffer[]= "72616B776972656C657373";

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial,Serial);

// Saves the LMIC structure during DeepSleep
RTC_DATA_ATTR bool joined = false;

int16_t h1;

void inline sensorsInit() {

}

bool readSensorsAndTx() {
// Split both words (16 bits) into 2 bytes of 8
	char payload[2];

	Serial.print("Requesting data...");
	h1 = analogRead(SensorPin);
	Serial.println("DONE");

	payload[0] = highByte(h1);
	payload[1] = lowByte(h1);
	
	Serial.println(F("Packet queued"));
  
	return RAKLoRa.rk_sendData(1, payload);
}

void goDeepSleep()
{
    Serial.println(F("Go DeepSleep"));
    Serial.flush();
    esp_sleep_enable_timer_wakeup(TX_INTERVAL * 1000000);
    esp_deep_sleep_start();
}

void setup() {
  RAKLoRa.rk_sleep(0);  //Wakeup RAK811 from sleep mode
  sensorsInit();
  delay(1000); //wait for the arduino ide's serial console to open
  
  Serial.begin(115200);
  while(Serial.available())
  {
    Serial.read(); 
  }
  
  ATSerial.begin(9600); //set ATSerial baudrate:This baud rate has to be consistent with  the baud rate of the WisNode device.
  while(ATSerial.available())
  {
    ATSerial.read(); 
  }

  if(!RAKLoRa.rk_setWorkingMode(0))  //set WisNode work_mode to LoRaWAN.
  {
    Serial.println(F("set work_mode failed, please reset module."));
    while(1);
  }
  
  RAKLoRa.rk_getVersion();  //get RAK811 firmware version
  Serial.println(RAKLoRa.rk_recvData());  //print version number

  Serial.println(F("Start init RAK811 parameters..."));
 
  if(!joined){
	  if (!InitLoRaWAN())  //init LoRaWAN
	  {
		Serial.println(F("Init error,please reset module.")); 
		while(1);
	  }
  }

  Serial.println(F("Start to join LoRaWAN..."));
  while(!RAKLoRa.rk_joinLoRaNetwork(60))  //Joining LoRaNetwork timeout 60s
  {
    Serial.println();
    Serial.println(F("Rejoin again after 5s..."));
    delay(5000);
  }
  
  if(!joined){
	  joined = true;
  }
  Serial.println(F("Join LoRaWAN success"));

  if(!RAKLoRa.rk_isConfirm(0))  //set LoRa data send package type:0->unconfirm, 1->confirm
  {
    Serial.println(F("LoRa data send package set error,please reset module.")); 
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
        Serial.println(F("RAK811 init OK!"));  
        return true;    
      }
    }
  }
  return false;
}

void loop() {
  Serial.println(F("Start send data..."));
  if (readSensorsAndTx())
  {    
    for (unsigned long start = millis(); millis() - start < 90000L;)
    {
      String ret = RAKLoRa.rk_recvData();
      if(ret != NULL)
      { 
        Serial.println(ret);
      }
      if((ret.indexOf("OK")>0)||(ret.indexOf("ERROR")>0))
      {
        Serial.println(F("Go to Sleep."));
        RAKLoRa.rk_sleep(1);  //Set RAK811 enter sleep mode
	goDeepSleep();
	RAKLoRa.rk_sleep(0);  //Wakeup RAK811 from sleep mode
        break;
      }
    }
  }
}
```

## **Appendice**

### **Comandi AT**

```
AT Command						Description

at+version						Get the current firmware version number.
at+get_config=device:status				Get all information about the device’s hardware components and their current status.
at+set_config=device:restart				After set, the device will restart.
at+set_config=device:boot				Let the device work in boot mode
at+run							Stop boot mode and run as normal. It is valid when the device works in boot mode.
at+set_config=device:sleep:X				After setting, the device will go to sleep mode or wake up immediately.{::nomarkdown}
							- X - 0: sleep, 1: wake-up
							{:/}
at+join							Start to join LoRa® network
at+send=lora:X:YYY					Send a customized data.{::nomarkdown}
							- X - LoRa® port
							{:/}{::nomarkdown}
							YYY - the data which you want to send. The limited length is 50 Bytes, and the data must be in HEX format.
							{:/}
at+set_config=lora:work_mode:X				Set the work mode for LoRa®.{::nomarkdown}
							- X - 0: LoRaWAN™, 1: LoRaP2P, 2: Test Mode.
							{:/}
at+set_config=lora:join_mode:X				Set the join mode for LoRaWAN™.{::nomarkdown}
							X - 0: OTAA, 1: ABP
							{:/}
at+set_config=lora:class:X				Set the class for LoRa®.{::nomarkdown}
							- X - 0: Class A,1: Class B,2: Class C
							{:/}
at+set_config=lora:region:XXX				Set the region for LoRa®.{::nomarkdown}
							XXX - one of the following items: EU868 EU433, CN470, IN865, EU868, AU915, US915, KR920, AS923.
							{:/}
at+set_config=lora:confirm:X				Set the type of messages which will be sent out through LoRa®.{::nomarkdown}
							- X - 0: unconfirm, 1: confirm
							{:/}
at+set_config=lora:dev_eui:XXXX				Set the device EUI for OTAA.{::nomarkdown}
							XXXX - the device EUI.
							{:/}
at+set_config=lora:app_eui:XXXX				Set the application EUI for OTAA.{::nomarkdown}
							- XXXX - the application EUI.
							{:/}
at+set_config=lora:app_key:XXXX				Set the application key for OTAA.{::nomarkdown}
							XXXX - the application key.
							{:/}
at+set_config=lora:dev_addr:XXXX			Set the device address for ABP.{::nomarkdown}
							- XXXX - the device address.
							{:/}
at+set_config=lora:apps_key:XXXX			Set the application session key for ABP.{::nomarkdown}
							- XXXX - the application session key.
							{:/}
at+set_config=lora:nwks_key:XXXX			Set the network session key for ABP.{::nomarkdown}
							- XXXX - the network session key.
							{:/}
at+set_config=lora:ch_mask:X:Y				Set a certain channel on or off.{::nomarkdown}
							- X - the channel number, and you can check which channel can be set before you set it.
							{:/}{::nomarkdown}
							- Y - 0: off, 1: on
							{:/}
at+set_config=lora:adr:X				Open or close the ADR function of LoRa® Node.{::nomarkdown}
							- X - 0: Close ADR, 1: Open ADR.
							{:/}
at+set_config=lora:dr:X					Set the DR of LoRa® Node.{::nomarkdown}
							- X - the number of DR. Generally, the value of X can be 0~5. More details, please check the LoRaWAN™ 1.0.2 specification.
							{:/}
at+get_config=lora:channel				It will return the state of all LoRa® channels, then you can see which channel is closed and which channel is open very clearly
at+set_config=lorap2p:XXX:Y:Z:A:B:C			Set the parameters for LoRa® P2P mode. This AT command is valid when the work mode is ·LoRaP2P.{::nomarkdown}
							- XXX - Frequency in Hz.
							{:/}{::nomarkdown}
							- Y - Spreading factor, [6, 7, 8, 9, 10, 11, 12].
							{:/}{::nomarkdown}
							- Z - Bandwidth, 0: 125 kHz, 1: 250 kHz, 2: 500kHz.
							{:/}{::nomarkdown}
							- A - Coding Rate, 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8.
							{:/}{::nomarkdown}
							- B - Preamble Length, 5~65535.
							{:/}{::nomarkdown}
							- C - Power in dbm, 5~20.
							{:/}
at+send=lorap2p:XXX					Send data through LoRaP2P. This AT command is valid when it works in LoRaP2P mode.{::nomarkdown}
							- XXX - the data in HEX.
```

### **Sitografia:**

- https://www.semiconductorstore.com/pdf/Migrating-Sensor-Design-LoRaWAN-WhitePaper_FINAL.pdf
- https://www.ictpower.it/tecnologia/lora-nozioni-di-base-e-approfondimenti.htm
- https://github.com/RAKWireless/WisNode-Arduino-Library
- https://github.com/RAKWireless/WisNode-Arduino-Library
- https://github.com/stm32duino
- https://www.arduinolibraries.info/architectures/stm32
- https://docs.rakwireless.com/Product-Categories/WisDuo/RAK811-Module/Datasheet/#certification
- https://githubmemory.com/repo/stm32duino/Arduino_Core_STM32/issues/1395
- https://forum.rakwireless.com/t/rak4200-and-rak811-breakout-board/2676/11
- https://downloads.rakwireless.com/LoRa/RAK811-BreakoutBoard/
- https://github.com/RAKWireless/RAK811_LoRaWAN_Arduino


>[Torna all'indice generale](index.md)


