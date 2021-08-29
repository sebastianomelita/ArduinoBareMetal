

>[Torna a gateway digitale](ak811hwgatewaydigitale.md)

## **Gateway per scheda relè digitale**

### **Schema di principio del collegamento a stella**

Notare i **collegamenti punto-punto** tra tutti i dispositivi e il microcontrollore tramite linee esclusive.

<img src="star.png" alt="alt text" width="200">

### **Il motore in corrente continua (DC)**

Ogni motore DC è costituito da due componenti chiave importanti per la funzionalità:
- Lo **statore** è la parte stazionaria del motore che fornisce un campo magnetico rotante per l'armatura.
- L'**armatura** è la parte del motore a corrente continua che ruota e in cui sono collegati componenti esterni, come una ruota.

I **magneti** nello statore (poli) e una **bobina** di fili (avvolgimento di campo), attraversati da una corrente, generano un **campo elettromagnetico**. Gli avvolgimenti del filo isolato all'esterno dell'armatura (avvolgimento dell'indotto) sono avvolti attorno all'armatura per concentrare il campo magnetico dello statore.

<img src="DC-Motor-Construction.jpg" alt="alt text" width="500">

Gli avvolgimenti dell'indotto sono collegati a un **commutatore**, fondamentalmente un **interruttore elettrico rotante**, che applica corrente elettrica agli avvolgimenti. La corrente elettrica eccita ciascuna bobina dell'indotto e crea un campo magnetico rotante costante. L'interruttore, a contatti striscianti, serve a fornire corrente alla bobina che è in grado di trainare il rotore, togliendo corrente alle bobine che non sono più nella posizione giusta per spigerlo.

Se le bobine dell'indotto vengono accese e spente in sequenza, il campo magnetico rotante interagisce con il campo magnetico dello statore e crea una coppia, che fa ruotare l'armatura. Pertanto, il motore DC **converte** l'**energia elettrica** in **energia meccanica**.

E' da rimarcare che normalmente non è una buona idea **collegare direttamente** la bobina di un relè ad una porta digitale del microcontrollore perchè questa potrebbe non fornire tutta la **corrente necessaria** per **eccitare competamente** il motore perchè questo non sarebbe in grado di erogare tutta la corrente necessaria. 

Utilizzeremo un transistor MOS come interruttore per controllare un piccolo motore a basso consumo. Controlleremo il MOS con ESP32. Potremmo usare qualsiasi altro microcontrollore. 

Bisogna fare attenzione a distinguere tra **carichi resistivi** e **carichi induttivi**. Una lampadina è un tipico carico resistivo, mentre un motore è un tipico carico induttivo. 

I **carichi induttivi** sono costituiti internamente da bobine più o meno grandi. I carichi induttivi soffrono del problema delle **sovratensioni** e delle **sovracorrenti** all'**accensione** e allo **spegnimento**. 

Una **sovracorrente** è una corrente che può essere **parecchie volte superiore** alla **massima corrente nominale** del **carico** (anche 10-15 volte), ha però la caratteristica di essere presente per un periodo **transitorio molto breve**. La sua presenza è un **fattore di stress** per i tramsistor che ne accorcia la vita operativa. Per cui è opportuno progettare il circuito per renderlo in grado di erogare una intensa corrente di spunto parecchie volte maggiore di quella nominale.

<img src="Different-Terminals-BJT-and-MOSFET.png" alt="alt text" width="500">

### **Come scegliere il MOSFET giusto per controllare il motore DC?**

Per il nostro circuito scegliamo il MOSFET IRLZ44. Ma come facevo a sapere che posso usare questo MOSFET? La decisione si basa sul microcontrollore e sulle specifiche elettriche del motore CC. I seguenti passaggi mostrano il processo, come trovare il transistor giusto per il tuo circuito:
- La tensione di soglia del gate (V_GS) è inferiore alla tensione di funzionamento del microcontrollore. L'IRLZ44 ha una tensione di soglia gate-source compresa tra 1 V e 2 V. Il nostro Arduino con tensione operativa di 5 V e ESP32 ed ESP8266 con una tensione operativa di 3,3 V hanno una tensione superiore alla tensione di soglia gate-source dell'IRLZ44.
- La tensione di rottura drain-source (breakdown voltage) non viene superata. Poiché utilizziamo il microcontrollore come fonte di alimentazione del motore CC, la nostra tensione drain-source non supererà i 5V. La tensione di rottura drain-source dell'IRLZ44 è molto più alta con 55V.
- La corrente di drain continua non viene superata. Il motore CC che usiamo in questo esempio assorbe in media 100 mA e il valore massimo assoluto per la corrente di drain continua dell'IRLZ44 è 35 A. Pertanto, il MOSFET IRLZ44 è una buona scelta per il nostro circuito. 

### **Perché abbiamo bisogno di un diodo in parallelo al motore DC?**

Abbiamo appreso nella prima parte di questo articolo che il motore DC ha bobine all'interno che creano un campo magnetico. Quando un motore DC è spento, la corrente nello statore è 0A e il rotore gira a vuoto a causa del momento d'inerzia. Questa rotazione provoca un'autoinduzione nelle bobine del motore e quindi picchi di tensione. 

Il diodo, chiamato anche **diodo flyback**, che abbiamo collegato in parallelo al motore CC fornisce un percorso sicuro per la corrente induttiva in modo che la tensione drain-source non aumenti e danneggi potenzialmente il circuito.

### **Perché abbiamo bisogno di un resistore tra il gate del transistor e la massa?**

Se il pin I/O digitale del microcontrollore Arduino, ESP8266 o ESP32 è impostato su LOW, ma la tensione di alimentazione del motore CC all'emettitore è ancora attiva, è possibile che il transistor sia passato in modo errato. Questo può accadere anche all'inizio del programma, dove il pin digitale del microcontrollore non è ancora definito come output. Con il resistore R2 impostiamo uno stato predefinito in questo caso portiamo a massa il gate.

La dimensione di questo resistore R2 tra gate e terra crea un partitore di tensione con il resistore R1. L'uscita del partitore di tensione è la tensione di gate del MOSFET. L'obiettivo è che la tensione di gate non sia troppo ridotta dal resistore R2. In caso contrario, potrebbe accadere che la tensione di soglia non venga superata per commutare il MOSFET.

### **Perché abbiamo bisogno di un resistore in serie al gate del transistor?**

Il resistore R1 è lì per limitare la corrente per il microcontrollore perché il gate del MOSFET è un piccolo condensatore che inizia a caricarsi se viene applicata una tensione al gate. L'assorbimento di corrente iniziale potrebbe essere troppo alto per il pin I/O digitale del microcontrollore, come mostrato nella figura seguente.

Quando la corrente supera la corrente massima del microcontrollore, che abbiamo visto nella tabella del capitolo precedente, il microcontrollore potrebbe danneggiarsi. Pertanto, utilizziamo il resistore R1 per proteggere il pin digitale e appiattire la curva di carica del gate MOSFET. Se volessimo un'alta frequenza di commutazione superiore a 100 kHz, possiamo usare un resistore R1 di 220.

### **Schema cablaggio**

**Avvertenza:** prima di realizzare questo schema elettrico, assicurarsi di non aver collegato ESP32 e lampada con alimentazione ad alta tensione. Stai utilizzando l'ALTA tensione in questo progetto, prendi tutte le precauzioni prima di creare questo layout. 

**Prima di toccare** il circuito per **inserire o disinserire collegamenti** assicurarsi con **certezza assoluta** di aver **scollegato la tensione di rete a 230V**. **Ripetere** questo controllo **con scrupolo** ogni volta che si **maneggia il circuito**.

<img src="Connect-DC-Motor-without-IC-ESP8266-NodeMCU_schem.png" alt="alt text" width="700">

<img src="Connect-DC-Motor-without-IC-ESP8266-NodeMCU_bb.png" alt="alt text" width="700">

### **Librerie del progetto**

Dal **punto di vista SW** non servono librerie particolari tranne quelle per la pubblicazione dei valori tramite LoRaWan. 

### **Gateway GPIO-LORaWan con modem HW per il comando di una scheda relè**

Il codice seguente, alla ricezione del messaggio sotto forma di un singolo byte (uint8_t) scrive il bit basso o alto sulla **porta di controllo** del relè ivi collegato. **Subito dopo** la scrittura del comando viene effettuata una **lettura dello stato** della stessa porta (la cmdport) e viene inviato il suo valore al server applicativo lungo un **canale di feedback** in uplink. 

**Periodicamente**, grazie ad una **schedulazione** all'interno del loop(), il microcontrollore **invia spontaneamente** lo **stato della porta** del relè con una cadenza memorizzata su interval e impostata a **60 secondi**.

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

int8_t cmdport = 22;
int8_t ax;

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
  if(!RAKLoRa.rk_setClass(1)){
	DebugSerial.println(F("LoRa class B not setted.")); 
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
	char payload[1];

	Serial.print("Requesting data...");
	ax = digitalRead(cmdport);
	Serial.println("DONE");

	payload[0] = ax;
	
	Serial.println(F("Packet queued"));
  
	return RAKLoRa.rk_sendData(1, payload);
}

void loop() {
	if(ATSerial.available()){
		String ret = RAKLoRa.rk_recvData();
		if(ret != NULL)
		{ 
			DebugSerial.println(ret);
			digitalWrite(cmdport, ret.toInt());
			readSensorsAndTx();
		}
		DebugSerial.println(F("Go to Sleep."));
		RAKLoRa.rk_sleep(1);  //Set RAK811 enter sleep mode
		delay(TX_INTERVAL * 1000);  //delay 10s
		RAKLoRa.rk_sleep(0);  //Wakeup RAK811 from sleep mode
	}
}
```


**Sitografia:**

- https://microcontrollerslab.com/esp32-web-server-control-relay-220-volt-lamp/
- https://www.monicoimpianti.net/rele-passo-passo/
- https://diyi0t.com/control-dc-motor-without-ic-motor-driver/

>[Torna a gateway digitale](ak811hwgatewaydigitale.md)
