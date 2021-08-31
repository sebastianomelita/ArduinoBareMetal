>[Torna all'indice generale](index.md)
## **Gateways LoRa-IP**

Per una discussione sintetica di tutti i tipi di BUS semplici dal punto di vista generale si rimanda a [Cablati semplici](cablatisemplici.md ).

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

### **Bande di frequenza e potenza**
```
				Lower EU	Upper EU	US (FCC)
Intervallo di frequenze		865 – 868 MHz	915 – 921 MHz	902 – 928 MHz
Potenza irradiata		2 watt		4 watt		4 watt
Canale di banda			200 kHz		400 kHz		400 kHz
```

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

In questo caso gran parte delle funzioni del middleware, quelle relative alla comunicazione via stack LoRaWan, è implementato all'interno del sistema a microprocessore (MCU). Nello specifico, le funzioni di livello applicativo, che in ambito IoT, sono tutte quelle comprese tra il livello 2 e il livello 7 ISO/OSI sono compito della MCU mentre il livello fisico è realizzato in HW dal chip del modem.

Dal **punto di vista SW** seve **4 librerie** da scaricare dentro la solita cartella **libraries**:
- **Arduino-LMIC library**. Si scarica da https://github.com/things4u/ESP-1ch-Gateway come ESP-1ch-Gateway-master.zip da scompattare e rinominare semplicemente come **ESP-1ch-Gateway**


```C++

```


### **Sitografia:**

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

>[Torna all'indice generale](index.md)


