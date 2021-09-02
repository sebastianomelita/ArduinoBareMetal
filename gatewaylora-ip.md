>[Torna all'indice generale](index.md)
## **Gateways LoRa-IP**

<img src="lpwan.png" alt="alt text" width="800">

### **Gateway/concentratore nodi**

**“Ponte trasparente”** che  trasporta i dati bidirezionali tra i **nodi finali** e il **network server** a monte:
- inserisce nei messaggi informazioni sulla qualità di ricezione dei dati (potenza, bit rate, ecc):
- A livello logico è **trasparente** in quanto ha solo il ruolo di **instradamento** senza fare alcuna elaborazione. E' il **centro stella** che raccoglie un "grappolo" di nodi LoRaWan.
- I sensori possono inviare messaggi a **più gateway** e quindi a fronte di uno stesso messaggio potrebbe arrivare **messaggi duplicati** al server
- Per poter supportare una **rete a stella** di ampie dimensioni, il gateway deve disporre di buone **capacità di ricezione**, ed essere capace di gestire un **elevato numero di messaggi** provenienti da svariati end device (anche sull’ordine delle **migliaia**). 
- Il gateway è in grado di ascoltare le trasmissioni anche su **più canali** e di decodificare pacchetti inviati con un **differenti spreading factor** contemporaneamente.

### **Server di rete (Network Server)**

Il **network server** è una componente di **back-end** della rete responsabile della ricezione dei dati provenienti dai vari gateway e dello svolgimento dei **processi di gestione** della rete: 
- si connette a più gateway tramite una connessione TCP/IP protetta 
- decide quale gateway dovrebbe rispondere a un messaggio di un nodo terminale
- Funzione fondamentale è il filtraggio ed eliminazione di eventuali pacchetti duplicati. 
- gestisce le velocità dei dati del nodo finale con l’adaptive data rate (ADR) allo scopo di massimizzare la vita delle batterie dei dispositivi e la capacità totale della rete. Il linea generale, un minor tempo di trasmissione e minore potenza di uscita si traducono in un ovvio risparmio energetico. Il network server assegna a ogni end node che si vuole connettere alla rete, un data rate e una potenza di uscita diversa a seconda della esigenza e della situazione (da 300bps fino a 50kbps). :
	- un data rate maggiore e una potenza di uscita minore ai nodi terminali piu`vicini  al  gateway  in  quanto meno suscettibili alle interferenze (trasmissione rapida e a bassa potenza). 
	- un data rate piu`basso (minore suscettibilità ai rumori) e una maggiore potenza di uscita solo ai nodi che si trovano a distanze notevoli dal gateway (trasmissione lunga e a alta potenza). 
- I dati ricevuti possono essere inviati agli **application server** per le elaborazioni successive oppure è possibile inviare eventuali notifiche agli end device per far attuare un’azione.
- Non ci sono interfacce standard di trasmissione dei dati tra network server ed application server (webservice, websocket, webhook, MQTT sono variamente implementati).

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

## **Implementazioni HW a basso costo di un gateway**

### **1) La scheda LoRa RMF95/W**

<img src="hwiotacrchitecturerfm95.png" alt="alt text" width="1000">

E' un modem Lora che implementa esclusivamente il **livello fisico** dello stack LoraWan spesso indicato semplicemente come **LoRa**. Sopra di esso può essere utilizzato lo **stack applicativo LoRawan** oppure un qualsiasi altro stack (ad es. **6LowPan e REPL**). 

In ogni caso, le funzioni di **rete** ed **applicative** al di sopra del livello fisico, con il **chip RMF95/W**M vanno implementate in SW mediante apposite **librerie**. Se si vuole un **modem** che implementi **in HW** tutto lo **stack LoraWan** si guardi il modulo **Microchip RN2483**.

### **Schema cablaggio**

Il transxeiver LoRa RMF95/W deve essere interfacciato con una scheda MCU a parte. Sotto L'IDE di Arduino si possono utilizzare tutte le schede della famiglia Arduino più le schede ESP8266 e ESP32 di Espressif dotate di modulo WiFi integrato. Di seguito proponiamo un esempio con la scheda ESP32.

<img src="LoRa_ESP32_Wiring2.png" alt="alt text" width="1000">

In questo caso transceiver Semtech SX1276 e MCU ESP32 sono già cablate insieme sulla stessa scheda. Di seguito è riportato il mappaggio del pinout del transceiver Lora su quello della MCU ESP32:
```
SX1276 pin name <--> ESP32 pin number
	ANA: 	Antenna
	GND: 	GND
	DIO3: 	don’t connect
	DIO4: 	don’t connect
	3.3V: 	3.3V
	DIO0: 	GPIO 2
	DIO1: 	don’t connect
	DIO2: 	don’t connect
	GND: 	don’t connect
	DIO5: 	don’t connect
	RESET: 	GPIO 14
	NSS: 	GPIO 5
	SCK: 	GPIO 18
	MOSI: 	GPIO 23
	MISO: 	GPIO 19
	GND: 	don’t connect
```

La lunghezza del filo dipende dalla frequenza:
- 868 MHz: 86,3 mm 
- 915 MHz: 81,9 mm
- 433 MHz: 173,1 mm

Per il nostro modulo dobbiamo utilizzare un filo da 86,3 mm saldato direttamente al pin ANA del ricetrasmettitore. Si noti che l'utilizzo di un'antenna adeguata estenderà il raggio di comunicazione.

### **2) La scheda TTGO LoRa32 SX1276 OLED**

<img src="TTGO-LoRa32-OLED-SX1276-p.jpg" alt="alt text" width="1000">

E' una scheda che integra in un unico modulo **porte GPIO** + **transceiver SX1276** + **OLED SSD1306** da 0.96'' + **MCU ESP32**. Il  transceiver SX1276 implementa esclusivamente il **livello fisico** dello stack LoraWan spesso indicato semplicemente come **LoRa**. Sopra di esso può essere utilizzato lo **stack applicativo LoRawan** oppure un qualsiasi altro stack (ad es. **6LowPan e REPL**). 

In ogni caso, le funzioni di **rete** ed **applicative** al di sopra del livello fisico, con il **SX1276** vanno implementate in SW sulla MCU mediante apposite **librerie**. Se si vuole un **modem** che implementi **in HW** tutto lo **stack LoraWan** si guardi il modulo **Microchip RN2483**.

### **Schema cablaggio**

In questo caso transceiver Semtech SX1276 e MCU ESP32 sono già cablate insieme sulla stessa scheda. Di seguito è riportato il mappaggio del pinout del transceiver Lora su quello della MCU ESP32:
```
SX1276 pin name <--> ESP32 pin number
	MISO		GPIO 19
	MOSI		GPIO 27
	SCK		GPIO 5
	CS		GPIO 18
	IRQ		GPIO 26
	RST		GPIO 14
```

### **3) La scheda Pycom LoPy**

<img src="Screen-Shot-2018-08-04-at-10.01.26-PM-1024x381.png" alt="alt text" width="1000">

E' una scheda che integra in un unico modulo **porte GPIO** + **transceiver LORa/Sigfox** + **MCU ESP32**. Il  transceiver SX1276 implementa esclusivamente il **livello fisico** dello stack LoraWan spesso indicato semplicemente come **LoRa**. Sopra di esso può essere utilizzato lo **stack applicativo LoRawan** oppure un qualsiasi altro stack (ad es. **6LowPan e REPL**). 

In ogni caso, le funzioni di **rete** ed **applicative** al di sopra del livello fisico, con il **transceiver** vanno implementate in SW sulla MCU mediante apposite **librerie**. Se si vuole un **modem** che implementi **in HW** tutto lo **stack LoraWan** si guardi il modulo **Microchip RN2483**.

A causa di un paio di problemi con il design SiPy, il modulo assorbe più corrente di quanto dovrebbe mentre è in Deep Sleep. Il regolatore di commutazione DC-DC rimane sempre in modalità ad alte prestazioni, che viene utilizzata per fornire il ripple di uscita più basso possibile quando il modulo è in uso. In questa modalità assorbe una corrente di riposo di 10mA. Quando il regolatore viene messo in modalità ECO la corrente di riposo scende a 10uA. Sfortunatamente, il pin utilizzato per controllare questa modalità è fuori dal dominio RTC. Ciò significa che non è utilizzabile durante il deep sleep. Ciò fa sì che il regolatore rimanga in modalità PWM, mantenendo la sua corrente di riposo a 10 mA. Inoltre, il chip flash non entra in modalità di spegnimento poiché il pin CS è flottante durante il Deep Sleep. Ciò fa sì che il chip flash consumi circa 2 mA di corrente. Per ovviare a questo problema è disponibile uno "shield per il Deep Sleep" che si collega al modulo e consente di interrompere l'alimentazione dal dispositivo.

### **Schema cablaggio**

In questo caso transceiver LoRa e MCU ESP32 sono già cablate insieme sulla stessa scheda. Di seguito è riportato il mappaggio del pinout del transceiver Lora su quello della MCU ESP32:

<img src="lopy.png" alt="alt text" width="1000">

### **4) La scheda Heltech Wireless Stick Lite**

<img src="heltec.png" alt="alt text" width="1000">

### **Bande di frequenza e potenza**
```
				Lower EU	Upper EU	US (FCC)
Intervallo di frequenze		865 – 868 MHz	915 – 921 MHz	902 – 928 MHz
Potenza irradiata		2 watt		4 watt		4 watt
Canale di banda			200 kHz		400 kHz		400 kHz
```

## **Implementazione del SW di un gateway/concentratore LoRaWan a basso costo**

### **Architettura di riferimento per IoT**

L'**architettura tipica del SW** a bordo di un dispositivo IoT è riassumibile:

<img src="iotarchitecture.png" alt="alt text" width="600">

Il **middleware** in genere è composto da **librerie di terze parti** da **compilare** all'interno di un **IDE** (ad es. Arduino) o all'interno di un **SDK** cioè un pacchetto di sviluppo per applicazioni che fornisce vari strumenti per compilazione, debug e documentazione (ad es. AWS IoT, Azure IoT, ESP-IDF). Oppure esistono **framework** specifici per IoT Open Source come RIOT che, una volta compilati su una macchina Linux, forniscono veri e propri **SO per IoT** con esattamente ciò che serve per la **comunicazione** di un certo dispositivo.

In questo caso, il **middleware** deve realizzare le funzioni di **livello applicativo**, previste dal protocollo LoRaWan, necessarie per lo svolgimento dei **processi di comunicazione** tra un **gateway LoRaWan** e i **nodi di sensori** ad esso afferenti. I processi si occupano della getione di dettagli come autenticazione, multiplazione del canale, classi di servizio e altro ancora.

### **Librerie del progetto**

In questo caso gran parte delle funzioni del middleware, quelle relative alla comunicazione via stack LoRaWan, è implementato all'interno del sistema a microprocessore (MCU). Nello specifico, le funzioni di livello applicativo, che in ambito IoT, sono tutte quelle comprese tra il livello 2 e il livello 7 ISO/OSI sono compito della MCU mentre il livello fisico è realizzato in HW dal chip del modem. 

Dal **punto di vista SW** servono delle **librerie** da scaricare dentro la solita cartella **libraries**:
- **Single Channel LoRaWAN Gateway**. Si scarica da https://github.com/things4u/ESP-1ch-Gateway come ESP-1ch-Gateway-master.zip da scompattare e rinominare semplicemente come **ESP-1ch-Gateway**
- **Arduino Time Library**. Si scarica da https://github.com/PaulStoffregen/Time come Time-master.zip da scompattare e rinominare semplicemente come **Time**
- **ArduinoJson**. Si scarica da https://github.com/bblanchon/ArduinoJson come ArduinoJson-6.x.zip da scompattare e rinominare semplicemente come **ArduinoJson-6.x**
- **arduino-base64**. Si scarica da https://github.com/adamvr/arduino-base64 come arduino-base64-master.zip da scompattare e rinominare semplicemente come **arduino-base64**
- **Streaming**. Si scarica dahttp://arduiniana.org/libraries/streaming/ come Streaming5.zip da scompattare e rinominare semplicemente come **Streaming**. Attenzione che dentro la cartella Streaming5 ci ata la cartella Streaming da scompattare.

Le librerie esposte sopra possono essere recuperate e copiate nella libraries prendendole da: **\ESP-1ch-Gateway\lib**

### **File di configurazione**

Prima di caricare lo schetch **ESP-1ch-Gateway.ino** sulla tua scheda, dovrai apportare alcune modifiche a un paio di file. Ecco una rapida panoramica:

**ESP-sc-gway.h**

Questo file è la principale fonte di configurazione per lo sketch del gateway. Le definizioni che probabilmente dovrai modificare sono:

- **Radio**. 
	- **_LFREQ** -- Imposta la gamma di frequenza su cui comunicherà la radio. Impostalo su 433 (Asia), 868 (UE) o 915 (USA)
    	- **_SPREADING** -- Imposta il fattore di diffusione LoRa. È possibile utilizzare SF7, SF8, SF9, SF10, SF11 o SF12. Tieni presente che ciò influirà sui dispositivi con cui il tuo gateway può comunicare.
    - **_CAD** -- Rilevamento dell'attività del canale. Se abilitato (impostato a 1) CAD consentirà al gateway di monitorare i messaggi inviati a qualsiasi fattore di diffusione. Il compromesso se abilitato: i segnali molto deboli potrebbero non essere captati dalla radio.
- **Hardware**
	- **OLED:** se la scheda non include un OLED, impostalo su 0.
	- **_PIN_OUT**: configura l'SPI e altre impostazioni hardware. Impostalo su 6, aggiungeremo una definizione hardware personalizzata in seguito.
	- **CFG_sx1276_radio**: assicurati che sia definito e che CFG_sx1272_radio non lo sia. Serve a configura la radio LoRa collegata a ESP32.
- **The Things Network (TTN)**
	- **_TTNSERVER** -- Il server per il tuo router LoRa. Per esempio. "router.eu.thethings.network" o "us-west.thethings.network"
	- **_TTNPORT** -- 1700 è la porta standard per TTN
	- **_DESCRIPTION** -- Personalizza il nome del tuo gateway
	- **_EMAIL** -- Il tuo indirizzo email, o quello del proprietario del gateway
	- **_LAT** e **_LON** -- Coordinate GPS del tuo gateway
- **Wifi**
	- Aggiungi almeno una rete WiFi all'array ```wpas wpa[]```, ma lascia vuota la prima voce. Per esempio:
	
```C++
	wpas wpa[] = {
	  { "" , "" },  // Reserved for WiFi Manager
	  { "my_wifi_network", "my_wifi_password" }
	};
```




### **loramodem.h**

Questo file definisce come è configurato il modem LoRa, inclusi quali canali di frequenza può utilizzare e quali pin utilizza ESP32 per comunicare con esso. 

Se il dispositivo non è nella lista delle configurazioni standard va modificata di conseguenza una di quelle predefinite o, in alternativa, aggiunta un'ulteriore configurazione, ad es:

```C++
struct pins {
  uint8_t dio0 = 26;
  uint8_t dio1 = 33;
  uint8_t dio2 = 32;
  uint8_t ss = 16;
  uint8_t rst = 27; // Reset not used
} pins;
#define SCK  14
#define MISO 12
#define MOSI 13
#define SS  16
#define DIO0 26
```

L'array ```int freqs[]``` può essere impostato, se si desidera utilizzare sottobande diverse, ma, oltre a ciò, non c'è molto altro qui che consigliamo di modificare.

### **configGway.h**

**Selezione del pin-out standard**

La libreria supporta 5 configurazioni di pin out out-of-the-box (vedi sotto). Se usi uno di questi, imposta il parametro sul valore corretto. Se le definizioni dei pin sono diverse, aggiorna i file loraModem.h e oLED.h per riflettere queste impostazioni (ad esempio, aggiungendo un campo 6). 

Le configurazioni predefinite di pin sono 5 e si impostano con ```#define _PIN_OUT x``` dove x può essere:
1. HALLARD 
2. pin out COMRESULT 
3. scheda basata su ESP32/Wemos 
4. scheda ESP32 basata su ESP32/TTGO 
5. ESP32/Heltec Wifi LoRA 32 (V2)

**Selezione della classe di servizio**

Bisogna definire la **classe di servizio** supportata dal gateway. 
- La **classe A** è supportata e contiene le operazioni di base per i sensori della batteria.
- La **classe B** prevede la modalità di funzionamento beacon. Il gateway invierà un segnale ai sensori collegati che consente loro di sincronizzare i messaggi di downlink.
- La modalità di funzionamento di **classe C** (continua) contiene il supporto per i dispositivi che probabilmente NON funzionano a batteria e ascolteranno sempre la rete. Di conseguenza, anche la latenza di questi dispositivi è inferiore rispetto ai dispositivi di classe A. I dispositivi di classe C non dipendono dall'alimentazione della batteria e estenderanno le finestre di ricezione fino alla successiva finestra di trasmissione. Infatti, solo le trasmissioni determinano l'interruzione dell'ascolto del canale fintanto che dura una trasmissione. I dispositivi di classe C non possono eseguire operazioni di classe B.

Con:

```#define _CLASS "A"```

Tutti i dispositivi inizieranno come dispositivi di classe A e potrebbero decidere di "aggiornarsi" alla classe B o C. Inoltre il gateway può supportare o meno la classe B, che è un superset della classe A. 

NOTA: è supportata solo la classe A

**Rilevamento dell'attività del canale**

**Channel Activity Detection (CAD)** è una funzione del chip LoRa RFM95 per rilevare i messaggi in arrivo (attività). Questi messaggi in arrivo potrebbero arrivare su uno dei noti fattori di diffusione SF7-SF12. Abilitando CAD, il gateway può ricevere messaggi di qualsiasi fattore di diffusione.

In realtà è usato nel normale funzionamento per dire al ricevitore che un altro segnale sta già utilizzando il canale.

La funzionalità CAD si adopera a fronte di un (piccolo) prezzo da pagare: il chip non sarà in grado di ricevere segnali molto deboli poiché la funzione CAD utilizzerà l'impostazione del registro RSSI del chip per determinare se ha ricevuto o meno un segnale (o solo rumore). Di conseguenza, non vengono ricevuti segnali molto deboli, il che significa che la portata del gateway sarà ridotta in modalità CAD.

#define _CAD 1

**Abilitazione del server web**

Questa impostazione abilita il server web. Sebbene il server web stesso richieda molta memoria, è di grande aiuto per configurare il gateway in fase di esecuzione e ne controlla il comportamento. Fornisce inoltre le statistiche degli ultimi messaggi ricevuti. Il parametro A_REFRESH definisce se il server web deve rinnovarsi ogni X secondi.

```C++
#define A_SERVER 1 // Definisce il WebServer locale solo se questo define è impostato
#define A_REFRESH 1 // il server web è abilitato all'aggiornamento sì/no? (sì va bene) 
#define A_SERVERPORT 80 // porta del server web locale
#define A_MAXBUFSIZE 192 // Deve essere maggiore di 128, ma abbastanza piccolo per funzionare
```

Il parametro ```A_REFRESH``` definisce se è possibile o meno impostare l'impostazione di aggiornamento sì/no nel browser web. L'impostazione nel browser web è normalmente impostata su "no" come impostazione predefinita, ma possiamo lasciare la definizione su "1" per abilitare quell'impostazione nel browser web.

**Impostazione del server TTN (The Thing Network)**

Il gateway consente di connettersi a 2 server contemporaneamente (come fanno la maggior parte dei gateway LoRa). Devi connetterti ad almeno un router LoRa standard, nel caso in cui utilizzi The Things Network (TTN) assicurati di impostare:

```C++
#define _TTNSERVER "router.eu.thethings.network"
#define _TTNPORT 1700
``

Nel caso in cui imposti il tuo server **on premise**, puoi specificare quanto segue utilizzando l'URL del tuo router e la tua porta:
```C++
#define _THINGSERVER "your_server.com" // URL del server del programma server LoRa udp.js
#define _THINGPORT 1701 // Il tuo server UDP dovrebbe ascoltare questa porta
```

**Identità del gateway**
Imposta i parametri di identità per il tuo gateway:
```C++
#define _DESCRIPTION "Gateway ESP"
#define _EMAIL "tua.email@provider.com"
#define _PIATTAFORMA "ESP8266"
#define _LAT 52.00
#define _LON 5.00
#define _ALT 0
```

**Utilizzo del gateway come nodo sensore**

È possibile utilizzare il gateway come nodo. In questo modo vengono riportati i valori del sensore locale/interno. Questa è una funzione che richiede molta memoria e CPU poiché la creazione di un messaggio del sensore coinvolge le funzioni EAS e CMAC.

```#define GATEWAYNODE 0```

Più sotto, nel file di configurazione configNode.h, è possibile impostare l'indirizzo e altre informazioni LoRa del nodo gateway.

**Impostazioni aggiuntive**

Impostazioni ulteriori molto utili possono essere trovate nel file di documentazione del progetto https://github.com/things4u/ESP-1ch-Gateway

```C++
// 1-channel LoRa Gateway for ESP32 and ESP8266
// Copyright (c) Maarten Westenberg 2016-2021 

#define VERSION "V.6.2.8.EU868; PlatformIO 210712 a; GIT"

//
// Based on work done by Thomas Telkamp for Raspberry PI 1ch gateway and many others.
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License
// which accompanies this distribution, and is available at
// https://opensource.org/licenses/mit-license.php
//
// NO WARRANTY OF ANY KIND IS PROVIDED
//
// Author: Maarten Westenberg (mw12554@hotmail.com)
//
// This file contains a number of compile-time settings that can be set on (=1) or off (=0)
// The disadvantage of compile time is minor compared to the memory gain of not having
// too much code compiled and loaded on your ESP device.
//
// NOTE: 
// If version is for ESP32 Heltec board, compile with ESP32 setting and board 
// "ESP32 Dev Module" or "Heltec WiFi Lora 32"
// 
// For ESP8266 Wemos: compile with "Wemos R1 D1" and choose
// the right _PIN_OUT below. Selecting Oled while that is not connected does not 
// really matter (so you can leave that in).
//
// The source has been optimized for PlatformIO usage. That means some variables
// can be defined in the .platformio.ini file. Please look at the example file to see 
// how to set these #defines
//
// ========================================================================================


// Define whether we should do a formatting of SPIFFS when starting the gateway
// This is usually a good idea if the webserver is interrupted halfway a writing
// operation. Also to be used when software is upgraded
// Normally, value 0 is a good default and should not be changed.
#if !defined _SPIFFS_FORMAT
#	define _SPIFFS_FORMAT 0
#endif


// Allows configuration through WifiManager AP setup. Must be 0 or 1	
#if !defined _WIFIMANAGER				
#	define _WIFIMANAGER 0
#endif


// Debug message will be put on Serial is this one is set.
// If set to 0, no printing to USB devices is done.
// Set to 1 it will print all user level messages (with correct debug set)
// If set to 2 it will also print interrupt messages (not recommended)
#if !defined _DUSB
#	define _DUSB 1
#endif


// Define the monitor screen. When it is greater than 0 then logging is displayed in
// the special screen at the GUI.
// If _DUSB is also set to 1 then most messages will also be copied to USB devices.
#if !defined _MONITOR
#	define _MONITOR 1
#endif


// Gather statistics on sensor and Wifi status
// 0= No statistics
// 1= Keep track of messages statistics, number determined by _MAXSTAT
// 2= Option 1 + Keep track of messages received PER each SF (default)
// 3= See Option 2, but with extra channel info (Not used when Hopping is not selected)
#if !defined _STATISTICS
#	define _STATISTICS 3
#endif

// Define the frequency band the gateway will listen on. Valid options are
// EU863_870	Europe 
// US902_928	North America
// AU925_928	Australia
// CN470_510	China
// IN865_867	India
// CN779-787	(Not Used)
// EU433		Europe
// KR920    	South Korea
// AS923		(Not Used)
// You can find the definitions in "loraModem.h" and frequencies in
// See https://www.thethingsnetwork.org/docs/lorawan/frequency-plans.html
#define EU863_870 1
 

// Define the CLASS mode of the gateway
// A: Baseline Class
// B: Beacon/Battery Class
// C: Continuous Listen Class
#define _CLASS "A"


// Define whether to use the old Semtech gateway API, which is still supported by TTN,
// but is more lightweight than the new TTN tcp based protocol.
// NOTE: Only one of the two should be defined! TTN Router project has stopped
//
#define _UDPROUTER 1
//#define _TTNROUTER 1


#if !defined _CHANNEL
#	define _CHANNEL 0
#endif

// The spreading factor is the most important parameter to set for a single channel
// gateway. It specifies the speed/datarate in which the gateway and node communicate.
// As the name says, in principle the single channel gateway listens to one channel/frequency
// and to one spreading factor only.
// This parameters contains the default value of SF, the actual version can be set with
// the webserver and it will be stored in SPIFF
// NOTE: The frequency is set in the loraModem.h file and is default 868100000 Hz.
#if !defined _SPREADING
#	define _SPREADING SF9
#endif

// Channel Activity Detection
// This function will scan for valid LoRa headers and determine the Spreading 
// factor accordingly. If set to 1 we will use this function which means the 
// 1-channel gateway will become even more versatile. If set to 0 we will use the
// continuous listen mode.
// Using this function means that we HAVE to use more dio pins on the RFM95/sx1276
// device and also connect enable dio1 to detect this state. 
#define _CAD 1


// CRCCHECK
// Defines whether we should check on the CRC of RXDONE messages (see stateMachine.ino)
// This should prevent us from getting a lot os stranges messgages of unknown nodes.
// Note: DIO3 must be connected for this to work (Heltec and later Wemos gateways). 
#define _CRCCHECK 1


// Definitions for the admin webserver.
// _SERVER determines whether or not the admin webpage is included in the sketch.
// Normally, leave it in!
#define _SERVER 1				// Define local WebServer only if this define is set
#define _REFRESH 1				// Allow the webserver refresh or not?
#define _SERVERPORT 80			// Local webserver port (normally 80)
#define _MAXBUFSIZE 192			// Must be larger than 128, but small enough to work


// Definitions for over the air updates. At the moment we support OTA with IDE
// Make sure that tou have installed Python version 2.7 and have Bonjour in your network.
// Bonjour is included in iTunes (which is free) and OTA is recommended to install 
// the firmware on your router witout having to be really close to the gateway and 
// connect with USB.
#if !defined _OTA
#	define _OTA 1
#endif


// We support a few pin-out configurations out-of-the-box: HALLARD, COMPRESULT and 
// Heltec/TTGO ESP32.
// If you use one of these, just set the parameter to the right value.
// If your pin definitions are different, update the loraModem.h file to reflect the
// hardware settings.
//	1: HALLARD
//	2: COMRESULT pin out
//	3: ESP32, Wemos pin out (Not used)
//	4: ESP32, Heltec and TTGO pin out (should work for Heltec, 433 and Oled too).
//	5: Other, define your own in loraModem.h (does not include GPS Code)
#if !defined _PIN_OUT
#	define _PIN_OUT 1
#endif


// Single channel gateways if they behave strict should only use one frequency 
// channel and one, or in case _CAD all, spreading factors. 
// The TTN backend replies on RX1 timeslot for spreading factors SF9-SF12. 
// If the 1ch gateway is working in and for nodes that ONLY transmit and receive on the set
// and agreed frequency and spreading factor. make sure to set STRICT to 1.
// In this case, the frequency and spreading factor for downlink messages is adapted by this
// gateway
// NOTE: If your node has only one frequency enabled and one SF, you must set this to 1
//		in order to receive downlink messages. This is the default mode.
// NOTE: In all other cases, value 0 works for most gateways with CAD enabled
#if !defined _STRICT_1CH
#	define _STRICT_1CH 1
#endif


// Extra Microseconds delay added by the Up receiver from the sensor to the server.
// As the tmst is also corrected, this will add to the downlink messages also.
// The server will use this value to compute the receive window.
#if !defined _RXDELAY1
//#	define _RXDELAY1 1000
#	define _RXDELAY1 0
#endif


//
// Also, normally the server will respond with SF12 in the RX2 timeslot.
// For TTN, the RX2 timeslot is SF9, so we should use that one for TTN
#if !defined _RX2_SF
#	define _RX2_SF 9
#endif


// This section defines whether we use the gateway as a repeater
// For his, we use another output channel as the channel (default==0) we are 
// receiving the messages on.
#if !defined _REPEATER
#	define _REPEATER 0
#endif


// Define if Oled Display is connected to I2C bus. Note that defining an Oled display does not
// impact performance negatively, certainly if no Oled is connected. Wrong Oled will not show
// sensible results on the Oled display
// _OLED==0;	No Oled display connected
// _OLED==1;	0.9" Oled Screen based on SSD1306
// _OLED==2;	1.3" Oled screens for Wemos, 128x64 SH1106
#if !defined _OLED
#	define _OLED 1
#endif


// Define whether we want to manage the gateway over UDP (next to management 
// thru webinterface).
// This will allow us to send messages over the UDP connection to manage the gateway 
// and its parameters. Sometimes the gateway is not accesible from remote, 
// in this case we would allow it to use the SERVER UDP connection to receive 
// messages as well.
// NOTE: Be aware that these messages are NOT LoRa and NOT LoRa Gateway spec compliant.
//	However that should not interfere with regular gateway operation but instead offer 
//	functions to set/reset certain parameters from remote.
#define _GATEWAYMGT 0


// Do extensive logging to file(s)
// Use the ESP8266 SPIFS filesystem to do extensive logging.
// We must take care that the filesystem never(!) is full, and for that purpose we
// rather have new records/line of statistics than very old.
// Of course we must store enough records to make the filesystem work
// NOTE:
// Please do NOT USE file logging if you are concerned about downstream timing.
//	The status logging taken about 6 ms (6000 uSec) for each write whereas
//	normally this is less than 100 uSecs.
#if !defined _STAT_LOG
#	define _STAT_LOG 0
#endif


// Set the Server Settings (IMPORTANT)
#define _LOCUDPPORT 1700					// UDP port of gateway! Often 1700 or 1701 is used for upstream comms


// This defines whether or not we would use the gateway as 
// as sort of backend decoding system for local sensors which decodes (such as TTGO T-Beam)
// 0: Do not use _LOCALSERVER
// 1: _LOCALSERVER is used for received messages
// 2: Also transmittes messages are encoded
#if !defined _LOCALSERVER
#	define _LOCALSERVER 1					// See server definitions for decodes
#endif


// ntp
// Please add daylight saving time to NTP_TIMEZONES when desired
#define NTP_TIMESERVER "nl.pool.ntp.org"	// Country and region specific
#define NTP_TIMEZONES	2					// How far is our Timezone from UTC (excl daylight saving/summer time)
#define SECS_IN_HOUR	3600
#define _NTP_INTR 0							// Do NTP processing with interrupts or in loop();


// lora sensor code definitions
// Defines whether the gateway will also report sensor/status value on MQTT 
// (such as battery and GPS)
// after all, a gateway can be a node to the system as well. Some sensors like GPS can be
// sent to the backend as a parameter, some (like humidity for example) can only be sent
// as a regular sensor value.
// Set its LoRa address and key below in this file, See spec. para 4.3.2
// NOTE: The node is switched off by default. Switch it on in the GUI
#if !defined _GATEWAYNODE
#	define _GATEWAYNODE 0
#endif


// We can put the gateway in such a mode that it will (only) recognize
// nodes that are put in a list of trusted nodes.
// Values:
// 0: Do not use names for trusted Nodes
// 1: Use the nodes as a translation table for hex codes to names (in TLN)
// 2: Same as 1, but is nodes NOT in the nodes list below they are NOT shown
// NOTE: We probably will make this list dynamic!
#define _TRUSTED_NODES 1


// ========================================================================
// DO NOT CHANGE BELOW THIS LINE
// Probably do not change items below this line, only if lists or 
// configurations on configNode.h are not large enough for example.
// ========================================================================


// Name of he configfile in SPIFFs	filesystem
// In this file we store the configuration and other relevant info that should
// survive a reboot of the gateway		
#define _CONFIGFILE "/gwayConfig.txt"


// Maximum number of Message History statistics records gathered. 20 is a good maximum 
// (memory intensive). For ESP32 maybe 30 could be used as well
#if !defined _MAXSTAT
#	define _MAXSTAT 20
#endif


// We will log a list of LoRa nodes that was forwarded using this gateway.
// For eacht node we record:
//	- node Number, or known node name
//	- Last seen 'seconds since 1/1/1970'
//	- SF seen (8-bit integer with SF per bit)
// The initial version _NUMMAX stores max this many nodes, please make
// _MAXSEEN==0 when not used
#if !defined _MAXSEEN
#	define _MAXSEEN 20
#endif
#define _SEENFILE "/gwaySeen.txt"


// Define the maximum amount of items we monitor on the screen
#if !defined _MAXMONITOR
#	define _MAXMONITOR 20
#endif


// Timing
#define _PULL_INTERVAL 16					// PULL_DATA messages to server to get downstream in seconds
#define _STAT_INTERVAL 120					// Send a 'stat' message to server
#define _NTP_INTERVAL 3600					// How often do we want time NTP synchronization
#define _WWW_INTERVAL 60					// Number of seconds before we refresh the WWW page
#define _FILE_INTERVAL 30					// Number of timer (in secs) before writing to files
#define _MSG_INTERVAL 31					// Message timeout timer in seconds
#define _RST_INTERVAL 125					// Reset interval in seconds, total chip reset


// Define the correct radio type that you are using
#define CFG_sx1276_radio		
//#define CFG_sx1272_radio


// Serial Port speed
#define _BAUDRATE 115200						// Works for debug messages to serial momitor


// Will we use Mutex or not?
// +SPI is input for SPI, SPO is output for SPI
#if !defined _MUTEX
#	define _MUTEX 0
#endif //MUTEX

// Read server/gateway traffic
#if !defined _GWAYSCAN
#	define _GWAYSCAN 0						// default, NO _GWAYSCAN
#endif //GWAYSCAN


// Init EXPERT mode
#if !defined _EXPERT
#	define _EXPERT 0						// default, NO _EXPERT
#endif //EXPERT

// MQTT definitions, these settings should be standard for TTN
// and need no changing. When _REPEATER function is selected, we do not
// use the backend function to send message to server over MQTT.
#if _REPEATER==0
// TTN Version 2:
//#	define _TTNSERVER "router.eu.thethings.network"
//
// TTN Version 3:
#	define _TTNSERVER "eu1.cloud.thethings.network"	
#	define _TTNPORT 1700							// Standard port for TTN
#endif

```
**configNode.h**

E' il file di configurazione del dispositivo quando è adoperato come nodo (oltre che gateway).

```C++
// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2021 Maarten Westenberg version for ESP8266 and ESP32
//
// based on work done by Thomas Telkamp for Raspberry PI 1ch gateway
// and many other contributors.
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License
// which accompanies this distribution, and is available at
// https://opensource.org/licenses/mit-license.php
//
// NO WARRANTY OF ANY KIND IS PROVIDED
//
// Author: Maarten Westenberg (mw12554@hotmail.com)
//
// This file contains a number of compile-time settings and declarations that are'
// specific to the LoRa rfm95, sx1276, sx1272 radio of the gateway.
//
//
// ------------------------------------------------------------------------------------

// It is possible to use the gateway as a normal sensor node also. In this case,
// substitute the node info below.
#if _GATEWAYNODE==1
	// Valid coding for internal sensors are LCODE and RAW.
	// Make sure to only select one.
#	define _LCODE 1
//#	define _RAW 1
#	define _CHECK_MIC 1
#	define _SENSOR_INTERVAL 300

	// Sensor and app address information
#	define _DEVADDR { 0x26, 0x01, 0x1B, 0x96 }
#	define _APPSKEY { 0x02, 0x02, 0x04, 0x32, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
#	define _NWKSKEY { 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }

	// For ESP32 based T_BEAM/TTGO boards these two are normally included
	// If included make value 1, else if not, make them 0
#	define _GPS 1
#	define _BATTERY 1
#endif //_GATEWAYNODE


#if _TRUSTED_NODES >= 1
struct nodex {
	uint32_t id;				// This is the LoRa ID (coded in 4 bytes uint32_t
	char nm[32];				// Name of the node
};

// Add all your named and trusted nodes to this list
nodex nodes[] = {
	{ 0x26011C6D , "lora-33 Empty node" },						// F=0
	{ 0x260116BD , "lora-34 PIR node" },						// F=0
	{ 0x26011152 , "lora-35 temp+humi node" },					// F=0
	{ 0x2601148C , "lora-36 test node"  },						// F=0
// LoRa 37
	{ 0x26011B90 , "lora-39 temp DS18B20" },					// F=0
	{ 0x260119A6 , "lora-40 airquality" },						// F=0
	{ 0x2601117D , "lora-41 temp+humi SR04T" },
	{ 0x26011593 , "lora-42 temp DS18B20" },					// F=0	
	{ 0x26011514 , "lora-43 temp DS18B20" },					// F=0
	{ 0x2601145C , "lora-44 Distance" },						// F=0
	{ 0x26011D77 , "lora-45 no sensor" },
	{ 0x2601160F , "lora-46 HTU21 metal case" },				// F=0
	{ 0x26011E71 , "lora-47 Dallas temperature" },				// F=0, or on F=1
	{ 0x26011215 , "lora-48 AM2315 temp/Humi" },				// F=0
	{ 0x2601168F , "lora-49 EU433 Sensor" },					// 433, F=0
	{ 0x26011b96 , "lora-50 Internal T-Beam gway" },			// F=0
//	{ 0xAABBCCDD , "lora-51 Heltec OTAA" },						// F=ALL
	{ 0x26011199 , "lora-52 CubeCell sensor" },					// F=ALL
	{ 0x26011E52 , "lora-53 gas sensor" },						// F=ALL
	{ 0x260112A9 , "lora-54 ESP8266 No sensor" },				// F=0
	{ 0x2601152D , "lora-55 ESP8266 No sensor" },				// F=0	
	{ 0x26011F66 , "lora-62 CubeCell Capsule" },				// F=ALL
	{ 0x260110ED , "lora-63 CubeCell Board" },					// F=ALL
	{ 0x26011A2B , "lora-64 Metal temp humi" },					// F=0
	{ 0x260114EA , "lora-65 Waterproof temp humi" },			// F=0
	{ 0x26011D1D , "lora-152 ESP32 No Sensor" },				// F=0
	{ 0x00000000 , "lora-00 well known sensor" }				// F=0
};

#endif //_TRUSTED_NODES


// In some cases we like to decode the lora message at the single channel gateway.
// In thisase, we need the NkwSKey and the AppsSKey of the node so that we can decode
// its messages.
// Although this is probably overkill in normal gateway situations, it greatly helps
// in debugging the node messages before they reach the TTN severs.
//
#if _LOCALSERVER>=1
struct codex  {
	uint32_t id;				// This is the device ID (coded in 4 bytes uint32_t
	unsigned char nm[32];		// A name string which is free to choose
	uint8_t nwkKey[16];			// The Network Session Key of 16 bytes
	uint8_t appKey[16];			// The Application Session Key of 16 bytes
};

// Sometimes we want to decode the sensor completely as we do in the TTN server
// This means that for all nodes we want to view the data of, we need to provide
// the AppsSKey and the NwkSKey

// Definition of all nodes that we want to decode locally on the gateway.
//
codex decodes[] = {
	{	0x26011152 , "lora-35",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x23, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 } 
	},
	{	0x2601148C , "lora-36",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 } 
	},
	{	0x26011B90 , "lora-39",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x27, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
	},
	{	0x26011593 , "lora-42",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
	},
	{	0x26011D77 , "lora-43",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
	},
	{	0x27000599 , "lora-44",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
	},
	{	0x26011B96 , "lora-50",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x32, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
	},
	{	0x26011B96 , "lora-52",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x34, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
	},
	{	0x26011B96 , "lora-62",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
	},
	{	0x26011B96 , "lora-63",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
	},
	{	0x260114EA , "lora-65",	// F=0
		{ 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 },
		{ 0x02, 0x02, 0x04, 0x41, 0x00, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x34, 0x55 }
	},
	{	0x00000000 , "lora-00",	// F=0
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
	}					
};
#endif //_LOCALSERVER


// Wifi definitions
// WPA is an array with SSID and password records. Set WPA size to number of entries in array
// When using the WiFiManager, we will overwrite the first entry with the 
// accesspoint we last connected to with WifiManager
struct wpas {
	char login[32];							// Maximum Buffer Size (and allocated memory)
	char passw[64];
};


// Please fill in at least ONE valid SSID and password from your own WiFI network
// below. This is needed to get the gateway working
//
wpas wpa[] = {
//	{ "mala_w", "DeWestenbergjesZijn1Bosbes!" },
//	{ "mala_z", "DeWestenbergjesZijn1Bosbes!" },
	{ "mala", "DeWestenbergjesZijn1Bosbes!" },
	{ "mala_a", "DeWestenbergjesZijn1Bosbes!" },
//	{ "Livebox-8AFC", "QMWvLKjMgcYMDrRGQZ" },
	{ "platenspeler", "maanlama@16" }
};


// If you have a second back-end server defined such as Semtech or loriot.io
// your can define _THINGPORT and _THINGSERVER with your own value.
// If not, make sure that you do not define these, which will save CPU time
// Port is UDP port in this program
//
// Default for testing: Switched off
// #define _THINGSERVER "westenberg.org"		// Server URL of the LoRa-udp.js handler
// #define _THINGPORT 1700						// Port 1700 is old compatibility


// Define the name of the accesspoint if the gateway is in accesspoint mode (is
// getting WiFi SSID and password using WiFiManager). 
// If you do not need them, comment out.
//#define AP_NAME "Gway-Things4U"
//#define AP_PASSWD "ttnAutoPw"


// Gateway Ident definitions. Where is the gateway located?
#define _DESCRIPTION "ESP Gateway"			// Name of the gateway
#define _EMAIL "mw12554@hotmail.com"		// Owner
#define _PLATFORM "ESP8266"
#define _LAT 52.237367
#define _LON 5.978654
#define _ALT 14								// Altitude


// For asserting and testing the following defines are used.
//
#if !defined(CFG_noassert)
#define ASSERT(cond) if(!(cond)) gway_failed(__FILE__, __LINE__)
#else
#define ASSERT(cond) /**/
#endif
```

### **Caricamento dello scketch**

Con le modifiche apportate, prova a compilare e caricare lo schetch sul tuo ESP32. Dopo che è stato caricato, apri il tuo monitor seriale e imposta la velocità di trasmissione su 115200. I messaggi di debug qui possono essere molto utili e trovare l'**indirizzo IP del tuo gateway** è fondamentale se vuoi monitorare il server web.

Lo schetch potrebbe richiedere molto tempo per la configurazione iniziale: **formatterà** il file **system SPIFFS** e creerà un **file di configurazione** non volatile. Una volta completato, dovresti vedere il tentativo di ESP32 di connettersi alla tua rete WiFi, quindi potrai inizializzare la radio.

Dopo che ESP32 si è connesso, cercalo per stampare un indirizzo IP. Apri il browser web del tuo computer e metti l'IP della scheda sulla barra degli indirizzi. Dovresti essere accolto dal portale web ESP Gateway Config:

<img src="esp-gateway-config-web.png" alt="alt text" width="800">

Questa pagina Web può essere utilizzata per **monitorare** i messaggi in arrivo e su quali **frequenze** e **fattori di diffusione** sono arrivati. 

Può anche essere utilizzata per modificare al volo la **configurazione del gateway**. È possibile regolare il **canale** o il **fattore di diffusione**, o attivare/disattivare **CAD**, o anche attivare il **salto di frequenza**.

Ovviamente, per vedere i messaggi passare avrai bisogno di un dispositivo LoRa (o device_s_) configurato per comunicare con il tuo gateway. 

### **Codice di esempio**

Per vedere un codice di esempio, aprire il file **ESP-sc-gway.ino**:

```C++
// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2021 Maarten Westenberg
// Author: Maarten Westenberg (mw12554@hotmail.com)
//
// Based on work done by Thomas Telkamp for Raspberry PI 1-ch gateway and many others.
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License
// which accompanies this distribution, and is available at
// https://opensource.org/licenses/mit-license.php
//
// NO WARRANTY OF ANY KIND IS PROVIDED
//
// The protocols and specifications used for this 1ch gateway: 
// 1. LoRA Specification version V1.0 and V1.1 for Gateway-Node communication
//	
// 2. Semtech Basic communication protocol between Lora gateway and server version 3.0.0
//	https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT
//
// Notes: 
// - Once call hostbyname() to get IP for services, after that only use IP
//	 addresses (too many gethost name makes the ESP unstable)
// - Only call yield() in main stream (not for background NTP sync). 
//
// ----------------------------------------------------------------------------------------
#define ARDUINO_ARCH_ESP32 1
#define  _OLED 0
#define _GATEWAYNODE 0


#if defined (ARDUINO_ARCH_ESP32) || defined(ESP32)
#	define ESP32_ARCH 1
#	ifndef _PIN_OUT
#		define _PIN_OUT 4									// For ESP32 pin-out 4 is default
#	endif
#elif defined(ARDUINO_ARCH_ESP8266)
	//
#else
#	error "Architecture unknown and not supported"
#endif

// The followion file contains most of the definitions
// used in other files. It should be the first file.
#include "configGway.h"										// contains the configuration data of GWay
#include "configNode.h"		
#include "ESP-sc-gway.h"								// Contains the personal data of Wifi etc.
/*
#include <Esp.h>											// ESP8266 specific IDE functions
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <sys/time.h>
#include <cstring>
#include <string>											// C++ specific string functions
*/
#include <SPI.h>											// For the RFM95 bus
#include <TimeLib.h>										// http://playground.arduino.cc/code/time
#include <ArduinoJson.h>
//#include <FS.h>												// ESP8266 Specific
#include <WiFiUdp.h>
#include <pins_arduino.h>
#include <gBase64.h>										// https://github.com/adamvr/arduino-base64 (changed the name)

// Local include files
#include "loraModem.h"
#include "loraFiles.h"
#include "oLED.h"
/*
"C" {
#	include "lwip/err.h"
#	include "lwip/dns.h"
}
*/
#if (_GATEWAYNODE==1) || (_LOCALSERVER>=1)
#	include "AES-128_V10.h"
#endif

// ----------- Specific ESP32 stuff --------------
#if defined(ESP32_ARCH)
#	include <WiFi.h>
#	include <ESPmDNS.h>
#	include <SPIFFS.h>
//#	include <WiFiManager.h>									// Standard lib for ESP WiFi config through an AP

#	define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

#	if _SERVER==1
#		include <WebServer.h>								// Standard Webserver for ESP32
#		include <Streaming.h>          						// http://arduiniana.org/libraries/streaming/
		WebServer server(_SERVERPORT);
#	endif //_SERVER

#	if _OTA==1
//#		include <ESP32httpUpdate.h>							// Not yet available
#		include <ArduinoOTA.h>
#	endif //_OTA


// ----------- Specific ESP8266 stuff --------------
#elif defined(ARDUINO_ARCH_ESP8266)
	"C" {
#		include "user_interface.h"
#		include "c_types.h"
	}
#	include <ESP8266WiFi.h>									// Which is specific for ESP8266
#	include <ESP8266mDNS.h>
#	include <WiFiManager.h>									// Library for ESP WiFi config through an AP

#	define ESP_getChipId()   (ESP.getChipId())

#	if _SERVER==1
#		include <ESP8266WebServer.h>
#		include <Streaming.h>          						// http://arduiniana.org/libraries/streaming/
		ESP8266WebServer server(_SERVERPORT);				// Standard IDE lib
#	endif //_SERVER

#	if _OTA==1
#		include <ESP8266httpUpdate.h>
#		include <ArduinoOTA.h>
#	endif //_OTA
					

#else
#	error "Architecture not supported"

#endif


#include <DNSServer.h>										// Local DNSserver


// ----------- Declaration of variables --------------

uint8_t debug=1;											// Debug level! 0 is no msgs, 1 normal, 2 extensive
uint8_t pdebug= P_MAIN ;									// Initially only MAIN and GUI
uint32_t txDones=0;											// Count the number of TXDONE calls in stateMachine for a send

#if _GATEWAYNODE==1
#	if _GPS==1
#		include <TinyGPS++.h>
		TinyGPSPlus gps;
		HardwareSerial sGps(1);
#	endif //_GPS
#endif //_GATEWAYNODE

using namespace std;

bool		sx1276 = true;									// Actually we use sx1276/RFM95
uint8_t		MAC_array[6];
uint8_t		currentMode = 0x81;

// ----------------------------------------------------------------------------
//
// Configure these values only if necessary!
//
// ----------------------------------------------------------------------------

uint8_t protocol	= _PROTOCOL;

// Set spreading factor (SF7 - SF12)
sf_t sf 			= _SPREADING;							// Initial value of SF					

// Set location, description and other configuration parameters
// Defined in ESP-sc_gway.h
//
float lat			= _LAT;									// Configuration specific info...
float lon			= _LON;
int   alt			= _ALT;
char platform[24]	= _PLATFORM; 							// platform definition
char email[40]		= _EMAIL;    							// used for contact email
char description[64]= _DESCRIPTION;							// used for free form description 

// JSON definitions
StaticJsonDocument<312> jsonBuffer;							// Use of arduinoJson version 6!
	
// define servers

IPAddress ntpServer;										// IP address of NTP_TIMESERVER
IPAddress ttnServer;										// IP Address of thethingsnetwork server
IPAddress thingServer;										// Only if we use a second (backup) server

WiFiUDP Udp;

time_t startTime = 0;										// The time in seconds since 1970 that the server started. 
uint32_t eventTime = 0;										// Timing of _event to change value (or not).
uint32_t sendTime = 0;										// Time that the last message transmitted
uint32_t doneTime = 0;										// Time to expire when CDDONE takes too long
uint32_t statTime = 0;										// last time we sent a stat message to server
uint32_t pullTime = 0;										// last time we sent a pull_data request to server
uint32_t rstTime  = 0;										// When to reset the timer
uint32_t fileTime = 0;										// Write the configuration to file

#define TX_BUFF_SIZE  1024									// Upstream buffer to send to MQTT
#define RX_BUFF_SIZE  1024									// Downstream received from MQTT
#define STATUS_SIZE	  512									// Should(!) be enough based on the static text .. was 1024

#if _SERVER==1
	uint32_t wwwtime = 0;
#endif
#if _NTP_INTR==0
	uint32_t ntptimer = 0;
#endif
#if _GATEWAYNODE==1
	uint16_t LoraUp.fcnt = 0;								// We write this to SPIFF file
	uint16_t LoraDown.fcnt = 0;								// LoraDown framecount init 0
#endif
#ifdef _PROFILER
	uint32_t endTmst=0;
#endif

// Init the indexes of the data we display on the webpage
// We use this for circular buffers
uint16_t iMoni=0;
uint16_t iSeen=0;
uint16_t iSens=0;

// volatile bool inSPI This initial value of mutex is to be free,
// which means that its value is 1 (!)
// 
int16_t mutexSPI = 1;

uint8_t buff[64]; 											// Buffer to use for sx1276, set to 64 characters
uint8_t buff_down[TX_BUFF_SIZE];							// Buffer for downstream
IPAddress remoteIpNo;
unsigned int remotePortNo;




// ----------------------------------------------------------------------------
// FORWARD DECLARATIONS
// These forward declarations are done since other .ino fils are linked by the
// compiler/linker AFTER the main ESP-sc-gway.ino file. 
// And espcecially when calling functions with ICACHE_RAM_ATTR the complier 
// does not want this.
// Solution can also be to specify less STRICT compile options in Makefile
// ----------------------------------------------------------------------------



#if _MUTEX==1
	void ICACHE_FLASH_ATTR CreateMutux(int *mutex);
	bool ICACHE_FLASH_ATTR GetMutex(int *mutex);
	void ICACHE_FLASH_ATTR ReleaseMutex(int *mutex);
#endif



// ============================================================================
// MAIN PROGRAM CODE (SETUP AND LOOP)


// ----------------------------------------------------------------------------
// Setup code (one time)
// _state is S_INIT
// ----------------------------------------------------------------------------
void setup() {

	char MAC_char[19];										// XXX Unbelievable
	MAC_char[18] = 0;
	char hostname[12];										// hostname space

	initDown(&LoraDown);
	initConfig(&gwayConfig);
		
#	if _DUSB>=1
		Serial.begin(_BAUDRATE);							// As fast as possible for bus
		delay(500);
		Serial.flush();
#	endif //_DUSB

#	if _OLED>=1
		init_oLED();										// When done display "STARTING" on Oled
#	endif //_OLED

#	if _GPS==1
		// Pins are defined in LoRaModem.h together with other pins
		sGps.begin(9600, SERIAL_8N1, GPS_TX, GPS_RX);		// PIN 12-TX 15-RX
#	endif //_GPS

	delay(500);

	if (SPIFFS.begin()) {
#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_MAIN)) {
			mPrint(F("SPIFFS begin"));
		}
#		endif //_MONITOR
	}
	else {													// SPIFFS not found
		if (pdebug & P_MAIN) {
			mPrint(F("SPIFFS not found, formatting"));
		}
		msg_oLED("FORMAT");
		SPIFFS.format();
		delay(500);
		initConfig(&gwayConfig);							// After a format reinit variables
	}
	
	// If we set SPIFFS_FORMAT in 
#	if _SPIFFS_FORMAT>=1
	msg_oLED("FORMAT");
	SPIFFS.format();										// Normally disabled. Enable only when SPIFFS corrupt
	delay(500);
	initConfig(&gwayConfig);
	gwayConfig.formatCntr++;
	if ((debug>=1) && (pdebug & P_MAIN)) {
		mPrint("SPIFFS Format Done");
	}
#	endif //_SPIFFS_FORMAT>=1

#	if _MONITOR>=1
		msg_oLED("MONITOR");
		initMonitor(monitor);
		
#		if defined CFG_noassert
			mPrint("No Asserts");
#		else
			mPrint("Do Asserts");
#		endif //CFG_noassert
#	endif //_MONITOR

	delay(500);
	
	// Read the config file for all parameters not set in the setup() or configGway.h file
	// This file should be read just after SPIFFS is initializen and before
	// other configuration parameters are used.
	// It will overwrite any settings given by initConfig
	//
	if (readGwayCfg(_CONFIGFILE, &gwayConfig) > 0) {			// read the Gateway Config
#		if _MONITOR>=1
		if (debug>=0) {
			mPrint("readGwayCfg:: return OK");
		}
#		endif
	}
	else {
#		if _MONITOR>=1
		if (debug>=0) {
			mPrint("setup:: readGwayCfg: ERROR readGwayCfg Failed");
		}
#		endif	
	};							
	delay(500);

#	if _WIFIMANAGER==1
		msg_oLED("WIFIMGR");
#		if _MONITOR>=1
			mPrint(F("setup:: WiFiManager"));
#		endif //_MONITOR
		delay(500);
	
		wifiMgr();
#	endif //_WIFIMANAGER

	msg_oLED("WIFI STA");
	WiFi.mode(WIFI_STA);									// WiFi settings for connections
	WiFi.setAutoConnect(true);
	WiFi.macAddress(MAC_array);
    sprintf(MAC_char,"%02x:%02x:%02x:%02x:%02x:%02x",
		MAC_array[0],MAC_array[1],MAC_array[2],MAC_array[3],MAC_array[4],MAC_array[5]);

#	if _MONITOR>=1
		mPrint("MAC: " + String(MAC_char) + ", len=" + String(strlen(MAC_char)) );
#	endif //_MONITOR


	// Setup WiFi UDP connection. Give it some time and retry x times. '0' means try forever
	while (WlanConnect(0) <= 0) {
#		if _MONITOR>=1
		if ((debug>=0) && (pdebug & P_MAIN)) {
			mPrint("setup:: Error Wifi network connect(0)");
		}
#		endif //_MONITOR
	}

	yield();

#	if _MONITOR>=1
	if ((debug>=1) & (pdebug & P_MAIN)) {
		mPrint("setup:: WlanConnect="+String(WiFi.SSID()) );
	}
#	endif
	
	// After there is a WiFi router connection, we set the hostname with last 3 bytes of MAC address.
#	if defined(ESP32_ARCH)
		// ESP32
		sprintf(hostname, "%s%02x%02x%02x", "esp32-", MAC_array[3], MAC_array[4], MAC_array[5]);
		WiFi.setHostname(hostname);
		MDNS.begin(hostname);
#	else
		//ESP8266
		sprintf(hostname, "%s%02x%02x%02x", "esp8266-", MAC_array[3], MAC_array[4], MAC_array[5]);
		wifi_station_set_hostname(hostname);
#	endif	//ESP32_ARCH

#	if _MONITOR>=1
	if (debug>=0) {
		String response = "Host=";
#		if defined(ESP32_ARCH)
			response += String(WiFi.getHostname());
#		else
			response += String(wifi_station_get_hostname());
#		endif //ESP32_ARCH

		response += " WiFi Connected to " + String(WiFi.SSID());
		response += " on IP=" + String(WiFi.localIP().toString() );
		mPrint(response);
	} 
#	endif //_MONITOR

	delay(500);
	// If we are here we are connected to WLAN
	
#	if _UDPROUTER==1
		// So now test the UDP function
		if (!connectUdp()) {
#			if _MONITOR>=1
				mPrint("Error connectUdp");
#			endif //_MONITOR
		}
#	elif _TTNROUTER==1
		if (!connectTtn()) {
#			if _MONITOR>=1
				mPrint("Error connectTtn");
#			endif //_MONITOR
		}
#	else
#		if _MONITOR>=1
			mPrint("Setup:: ERROR, No Router Connection defined");
#		endif //_MONITOR	
#	endif //_UDPROUTER

	delay(200);
	
	// Pins are defined and set in loraModem.h
    pinMode(pins.ss, OUTPUT);
	pinMode(pins.rst, OUTPUT);
    pinMode(pins.dio0, INPUT);								// This pin is interrupt
	pinMode(pins.dio1, INPUT);								// This pin is interrupt
	//pinMode(pins.dio2, INPUT);								// XXX future expansion


#if defined(ESP32_ARCH)
	SPI.begin(SCK, MISO, MOSI, SS);							// Init the SPI pins
#else
	SPI.begin();
#endif //ESP32_ARCH
	// define the SPI settings for reading/writing messages
	SPISettings settings(SPISPEED, MSBFIRST, SPI_MODE0);
	SPI.beginTransaction(settings);

	delay(500);
	
	// We choose the Gateway ID to be the Ethernet Address of our Gateway card
    // display results of getting hardware address
	//
#	if _MONITOR>=1
	if (debug>=0) {
		String response= "Gateway ID: ";
		printHexDigit(MAC_array[0], response);
		printHexDigit(MAC_array[1], response);
		printHexDigit(MAC_array[2], response);
		printHexDigit(0xFF,			response);
		printHexDigit(0xFF,			response);
		printHexDigit(MAC_array[3], response);
		printHexDigit(MAC_array[4], response);
		printHexDigit(MAC_array[5], response);

		response += ", Listening at SF" + String(sf) + " on ";
		response += String(freqs[gwayConfig.ch].upFreq) + " Hz.";
		mPrint(response);
	}
#	endif //_MONITOR

	// ---------- TIME -------------------------------------
	msg_lLED("GET TIME",".");
	ntpServer = resolveHost(NTP_TIMESERVER, 15);
	if (ntpServer.toString() == "0:0:0:0")	{					// Experimental
#		if _MONITOR>=1
			mPrint("setup:: NTP Server not found, found="+ntpServer.toString());
#		endif
		delay(10000);											// Delay 10 seconds
		ntpServer = resolveHost(NTP_TIMESERVER, 10);
	}

	// Set the NTP Time
	// As long as the time has not been set we try to set the time.
#	if _NTP_INTR==1
		setupTime();											// Set NTP time host and interval
		
#	else //_NTP_INTR
	{
		// If not using the standard libraries, do manual setting
		// of the time. This method works more reliable than the 
		// interrupt driven method.
		String response = ".";
		while (timeStatus() == timeNotSet) {					// time still 1/1/1970 and 0:00 hrs

			time_t newTime;
			if (getNtpTime(&newTime)<=0) {
#				if _MONITOR>=1
				if (debug>=0) {
					mPrint("setup:: ERROR Time not set (yet). Time="+String(newTime) );
				}
#				endif //_MONITOR
				response += ".";
				msg_lLED("GET TIME",response);
				delay(800);
				continue;
			}
			response += ".";
			msg_lLED("GET TIME",response);
			delay(1000);
			setTime(newTime);
		}
		
		// When we are here we succeeded in getting the time
		startTime = now();										// Time in seconds
#		if _MONITOR>=1
		if (debug>=0) {
			String response= "Time set=";
			stringTime(now(),response);
			mPrint(response);
		}
#		endif //_MONITOR

		writeGwayCfg(_CONFIGFILE, &gwayConfig );
	}
#	endif //_NTP_INTR

	delay(100);


#if _GWAYSCAN==1
	mPrint("setup:: _GWAYSCAN=1: Setup OK");

#else

	// ---------- TTNSERVER or THINGSERVER -------------------------------	
#	ifdef _TTNSERVER
		ttnServer = resolveHost(_TTNSERVER, 10);			// Use DNS to get server IP
		if (ttnServer.toString() == "0:0:0:0") {			// Experimental
#			if _MONITOR>=1
			if (debug>=1) {
				mPrint("setup:: TTN Server not found");
			}
#			endif
			delay(10000);									// Delay 10 seconds
			ttnServer = resolveHost(_TTNSERVER, 10);
		}	
		delay(100);
#	endif //_TTNSERVER

#	ifdef _THINGSERVER
		thingServer = resolveHost(_THINGSERVER, 10);		// Use DNS to get server IP
		delay(100);
#	endif //_THINGSERVER

#endif //_GWAYSCAN

	// The Over the Air updates are supported when we have a WiFi connection.
	// The NTP time setting does not have to be precise for this function to work.
#if _OTA==1
	setupOta(hostname);										// Uses wwwServer 
#endif //_OTA

	readSeen(_SEENFILE, listSeen);							// read the seenFile records

#if _SERVER==1	
	// Setup the webserver
	setupWWW();
#endif //_SERVER

	delay(100);												// Wait after setup
	
	// Setup and initialise LoRa state machine of _loraModem.ino
	_state = S_INIT;
	initLoraModem();
	
	if (gwayConfig.cad) {
		_state = S_SCAN;
		sf = SF7;
		cadScanner();										// Always start at SF7
	}
	else { 
		_state = S_RX;
		rxLoraModem();
	}
	LoraUp.payLoad[0]= 0;
	LoraUp.size = 0;										// Init the length to 0

	// init interrupt handlers, which are shared for GPIO15 / D8, 
	// we switch on HIGH interrupts
	if (pins.dio0 == pins.dio1) {
		attachInterrupt(pins.dio0, Interrupt_0, RISING);	// Share interrupts
	}
	// Or in the traditional Comresult case
	else {
		attachInterrupt(pins.dio0, Interrupt_0, RISING);	// Separate interrupts
		attachInterrupt(pins.dio1, Interrupt_1, RISING);	// Separate interrupts
		//attachInterrupt(pins.dio2, Interrupt_2, RISING);	// Separate interrupts		
	}

	writeConfig(_CONFIGFILE, &gwayConfig);					// Write config
	printSeen(_SEENFILE, listSeen);							// Write the last time record  is seen

	// activate Oled display
#	if _OLED>=1
		acti_oLED();
		addr_oLED();
#	endif //_OLED

	writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);		// Allow all
	writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);			// Reset all interrupt flags

	// Now print a few registers
#	define printReg(x) {int i=readRegister(x); if(i<=0x0F) Serial.print('0'); Serial.print(i,HEX);}
	Serial.print("RegInvertiQ :: "); printReg(REG_INVERTIQ);  Serial.println();
	Serial.print("RegInvertiQ2:: "); printReg(REG_INVERTIQ2); Serial.println();

	mPrint(" --- Setup() ended, Starting loop() ---");

}//setup



// ----------------------------------------------------------------------------
// LOOP
// This is the main program that is executed time and time again.
// We need to give way to the backend WiFi processing that 
// takes place somewhere in the ESP8266 firmware and therefore
// we include yield() statements at important points.
//
// Note: If we spend too much time in user processing functions
// and the backend system cannot do its housekeeping, the watchdog
// function will be executed which means effectively that the 
// program crashes.
// We use yield() to avoid ANY watch dog activity of the program.
//
// NOTE2: For ESP make sure not to do large array declarations in loop();
// ----------------------------------------------------------------------------
void loop ()
{
	int packetSize;
	uint32_t nowSeconds = now();
	
	// If we are not connected, try to connect.
	// We will not read Udp in this loop cycle if not connected to Wlan
	if (WlanConnect(1) < 0) {
#		if _MONITOR>=1
		if ((debug>=0) && (pdebug & P_MAIN)) {
			mPrint("loop:: ERROR reconnect WLAN");
		}
#		endif //_MONITOR
		yield();
		return;												// Exit loop if no WLAN connected
	} //WlanConnect()

	yield();												// 200403 to make sure UDP buf filled

	// So if we are connected 
	// Receive UDP PUSH_ACK messages from server. (*2, par. 3.3)
	// This is important since the TTN broker will return confirmation
	// messages on UDP for every message sent by the gateway. So we have to consume them.
	// As we do not know when the server will respond, we test in every loop.
	//
	while( (packetSize= Udp.parsePacket()) > 0) {
#		if _MONITOR>=1
		if ((debug>=3) && (pdebug & P_TX)) {
			mPrint("loop:: readUdp available");
		}
#		endif //_MONITOR

		// DOWNSTREAM
		// Packet may be PUSH_ACK (0x01), PULL_ACK (0x03) or PULL_RESP (0x04)
		// This command is found in byte 4 (buffer[3])
		// Only PULL_RESP carries more data for sensor and needs action,
		// others are for Gateway only.
		//
		if (readUdp(packetSize) < 0) {
#			if _MONITOR>=1
			if (debug>=0)
				mPrint("v readUdp ERROR, returning < 0");
#			endif //_MONITOR
			break;
		}
		// Now we know we succesfully received message from host
		// If return value is 0, we received a NTP message,
		// otherwise a UDP message with other TTN content, all ACKs are 4 bytes long
		else {
			//_event=1;										// Could be done double if more messages received
			//mPrint("v udp received="+String(micros())+", packetSize="+String(packetSize));
		}
	}

	yield();

	// check for event value, which means that an interrupt has arrived.
	// In this case we handle the interrupt ( e.g. message received)
	// in userspace in loop().
	//
	stateMachine();											// do the state machine
	
	// After a quiet period, make sure we reinit the modem and state machine.
	// The interval is in seconds (about 15 seconds) as this re-init
	// is a heavy operation. 
	// So it will kick in if there are not many messages for the gateway.
	// Note: Be careful that it does not happen too often in normal operation.
	//
	if ( ((nowSeconds - statr[0].time) > _MSG_INTERVAL) &&
		(msgTime <= statr[0].time) ) 
	{
#		if _MONITOR>=1
		if ((debug>=2) && (pdebug & P_MAIN)) {
			String response="";
			response += "REINIT:: ";
			response += String( _MSG_INTERVAL );
			response += (" ");
			mStat(0, response);
			mPrint(response);
		}
#		endif //_MONITOR

		yield();											// Allow buffer operations to finish

		if ((gwayConfig.cad) || (gwayConfig.hop)) {
			_state = S_SCAN;
			sf = SF7;
			cadScanner();
		}
		else {
			_state = S_RX;
			rxLoraModem();
		}

		msgTime = nowSeconds;
	}

#	if _OTA==1
	// Perform Over the Air (OTA) update if enabled and requested by user.
	// It is important to put this function early in loop() as it is
	// not called frequently but it should always run when called.
	//
	yield();
	ArduinoOTA.handle();
#	endif //_OTA

	// If event is set, we know that we have a (soft) interrupt.
	// After all necessary web/OTA services are scanned, we will
	// reloop here for timing purposes. 
	// v as less yield() as possible.
	if (_event == 1) {
		return;
	}
	else yield();

#	if _SERVER==1
	// Handle the Web server part of this sketch. Mainly used for administration 
	// and monitoring of the node. This function is important so it is called at the
	// start of the loop() function.
	server.handleClient();
#	endif //_SERVER



	// stat PUSH_DATA message (*2, par. 4)
	// Down send to server
	//	
    if ((nowSeconds - statTime) >= _STAT_INTERVAL) {		// Wake up every xx seconds
		yield();											// on 26/12/2017
        sendStat();											// Show the status message and send to server
#		if _MONITOR>=1
		if ((debug>=2) && (pdebug & P_MAIN)) {
			mPrint("Send Pushdata sendStat");
		}
#		endif //_MONITOR

		// If the gateway behaves like a node, we do from time to time
		// send a node message to the backend server.
		// The Gateway node message has nothing to do with the STAT_INTERVAL
		// message but we schedule it in the same frequency.
		//
#		if _GATEWAYNODE==1
		if (gwayConfig.isNode) {
			// Give way to internal some Admin if necessary
			yield();

			// If the 1ch gateway is a sensor itself, send the sensor values
			// could be battery but also other status info or sensor info

			if (sensorPacket() < 0) {
#				if _MONITOR>=1
				if ((debug>=1) || (pdebug & P_MAIN)) {
					mPrint("sensorPacket: Error");
				}
#				endif //_MONITOR
			}
		}
#		endif//_GATEWAYNODE
		statTime = nowSeconds;
    }


	// send PULL_DATA message (*2, par. 4)
	//
	// This message will also restart the server which taken approx. 3 ms.
	// Byte 0:		Prtocol Version
	// Byte 1-2:	Arbritary Token Value
	// Byte 3:		PULL_DATA ident ==0x02
	// Byte 4-7:	Gateway EUI
	//
	nowSeconds = now();
    if ((nowSeconds - pullTime) >= _PULL_INTERVAL) {		// Wake up every xx seconds

		yield();
        pullData();											// Send PULL_DATA message to server
		pullTime = nowSeconds;
		
#		if _MONITOR>=1
		if ((debug>=3) && (pdebug & P_RX)) {
			String response = "^ PULL_DATA:: ESP-sc-gway: message micr=";
			printInt(micros(), response);
			mPrint(response);
		}
#		endif //_MONITOR
    }


	// send RESET_DATA message (*2, par. 4)
	// 				MMM Do we need this as standard?
	// This message will also restart the server which taken approx. 3 ms.
	nowSeconds = now();
    if ((nowSeconds - rstTime) >= _RST_INTERVAL) {			// Wake up every xx seconds

		yield();
		startReceiver();
		rstTime = nowSeconds;
		
#		if _MONITOR>=1
		if ((debug>=2) && (pdebug & P_MAIN)) {
			String response = "^ ESP-sc-gway:: RST_DATA message sent: micr=";
			printInt(micros(), response);
			mPrint(response);
		}
#		endif //_MONITOR
    }

	
	// If we do our own NTP handling (advisable)
	// We do not use the timer interrupt but use the timing
	// of the loop() itself which is better for SPI
#	if _NTP_INTR==0
		// Set the time in a manual way. v not use setSyncProvider
		//  as this function may collide with SPI and other interrupts
		// Note: It can be that we do not receive a time this loop (no worries)
		yield();
		if (nowSeconds - ntptimer >= _NTP_INTERVAL) {
			yield();
			time_t newTime;
			if (getNtpTime(&newTime)<=0) {
#				if _MONITOR>=1
				if (debug>=2) {
					mPrint("loop:: WARNING Time not set (yet). Time="+String(newTime) );
				}
#				endif //_MONITOR
			}
			else {
				setTime(newTime);
				if (year(now()) != 1970) {					// Do not when year(now())=="1970"
					ntptimer = nowSeconds;					// beacause of "FORMAT"
				}
			}
		}
#	endif //_NTP_INTR

#	if _MAXSEEN>=1
		if ((nowSeconds - fileTime) >= _FILE_INTERVAL) {
			printSeen(_SEENFILE, listSeen);
			fileTime = nowSeconds;
		}
#	endif //_MAXSEEN

}//loop

```
**ESP-sc-gway.h**

```C++
void ICACHE_RAM_ATTR Interrupt_0();
void ICACHE_RAM_ATTR Interrupt_1();

int sendPacket(uint8_t *buf, uint8_t len);								// _txRx.ino forward

void printIP(IPAddress ipa, const char sep, String & response);			// _wwwServer.ino
void setupWWW();														// _wwwServer.ino forward

void mPrint(String txt);												// _utils.ino
int getNtpTime(time_t *t);												// _utils.ino
int mStat(uint8_t intr, String & response);								// _utils.ino
void SerialStat(uint8_t intr);											// _utils.ino
void printHexDigit(uint8_t digit, String & response);					// _utils.ino
int inDecodes(char * id);												// _utils.ino
static void stringTime(time_t t, String & response);					// _utils.ino

int initMonitor(struct moniLine *monitor);								// _loraFiles.ino
void initConfig(struct espGwayConfig *c);								// _loraFiles.ino
int printSeen(const char *fn, struct nodeSeen *listSeen);				// _loraFiles.ino
int readGwayCfg(const char *fn, struct espGwayConfig *c);				// _loraFiles.ino
int readSeen(const char *fn, struct nodeSeen *listSeen);
int writeConfig(const char *fn, struct espGwayConfig *c);
int writeGwayCfg(const char *fn, struct espGwayConfig *c);
int addSeen(struct nodeSeen *listSeen, struct stat_t stat);
int initSeen(struct nodeSeen *listSeen);

void init_oLED();														// _oLED.ino
void acti_oLED();														// _oLED.ino
void addr_oLED();														// _oLED.ino
void msg_oLED(String mesg);												// _oLED.ino
void msg_lLED(String mesg, String mesg2);

void setupOta(char *hostname);											// _otaServer.ino

void initLoraModem();													// _loraModem.ino
void rxLoraModem();														// _loraModem.ino
void writeRegister(uint8_t addr, uint8_t value);						// _loraModem.ino
void cadScanner();														// _loraModem.ino
void startReceiver();													// _loraModem.ino
void initDown(struct LoraDown *LoraDown);
uint8_t readRegister(uint8_t addr);

void stateMachine();													// _stateMachine.ino

bool connectUdp();														// _udpSemtech.ino
int readUdp(int packetSize);											// _udpSemtech.ino
int sendUdp(IPAddress server, int port, uint8_t *msg, uint16_t length);	// _udpSemtech.ino
void sendStat();														// _udpSemtech.ino
void pullData();														// _udpSemtech.ino
IPAddress resolveHost(String svrName, int maxTry);
int WlanConnect(int maxTry);

void hop();
void opmode(uint8_t mode);
void  setFreq(uint32_t freq);
void setRate(uint8_t sf, uint8_t crc);
uint8_t receivePkt(uint8_t *payload);
void writeRegister(uint8_t addr, uint8_t value);
uint8_t readRegister(uint8_t addr);
void rxLoraModem();
 
int loraWait(struct LoraDown *LoraDown);
void txLoraModem(struct LoraDown *LoraDown);
 
void updateOtaa();

int base64_decode(char * output, char * input, int inputLen);
int base64_dec_len(char * input, int inputLen);
int base64_enc_len(int plainLen);
int base64_encode(char *output, char *input, int inputLen);
uint8_t encodePacket(uint8_t *Data, uint8_t DataLength, uint16_t FrameCount, uint8_t *DevAddr, uint8_t *AppSKey, uint8_t Direction);
 
```

### **Sitografia:**

- https://learn.sparkfun.com/tutorials/esp32-lora-1-ch-gateway-lorawan-and-the-things-network/all#single-channel-lorawan-gateway-code
- https://redmine.laas.fr/attachments/download/1505/LMIC-v2.3.pdf
- https://github.com/matthijskooijman/arduino-lmic/blob/master/doc/LMiC-v1.5.pdf
- https://randomnerdtutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/
- https://jackgruber.github.io/2020-04-13-ESP32-DeepSleep-and-LoraWAN-OTAA-join/
- https://gitmemory.com/issue/JackGruber/Arduino-Pro-Mini-LoRa-Sensor-Node/2/678644527
- https://lora-developers.semtech.com/library/tech-papers-and-guides/lorawan-class-b-devices/
- https://cpp.hotexamples.com/it/examples/-/-/LMIC_sendAlive/cpp-lmic_sendalive-function-examples.html
- https://www.semiconductorstore.com/pdf/Migrating-Sensor-Design-LoRaWAN-WhitePaper_FINAL.pdf
- https://www.ictpower.it/tecnologia/lora-nozioni-di-base-e-approfondimenti.htm
- https://github.com/RAKWireless/WisNode-Arduino-Library
- https://wikifactory.com/@prusalab/prusahive/v/7f51dd4/file/Firmware/PrusaHiveV0.2/lib/TheThingsNetwork_esp32-master/src/lmic/lmic/radio.c
- https://docs.pycom.io/gitbook/assets/lopy-pinout.pdf
- https://docs.pycom.io/datasheets/development/sipy/
- https://www.thethingsnetwork.org/labs/story/program-your-lopy-from-the-arduino-ide-using-lmic
- https://www.studiopieters.nl/esp32-pinout/
- https://github.com/beegee-tokyo/SX126x-Arduino/

>[Torna all'indice generale](index.md)


