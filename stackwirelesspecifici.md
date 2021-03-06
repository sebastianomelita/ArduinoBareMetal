>[Torna a reti di sensori](sensornetworkshort.md#stack-wireless-specifici-per-IOT)

## **Stack wireless specifici**

### **Stack wireless open source**

Esistono anche **stack open source** rilasciati da alleanze di sviluppatori pensati per sostituire il **livello applicativo** dei chip IoT attualmente più diffusi per orientarli verso ambiti di utilizzo più **specializzato**. Un esempio è **OpenThread** che si pone sopra i chip **802.15.4** realizzando con l'implementazione dello stack **Thread** una alternativa a Zigbee per l'**industria**. Sempre basandosi sul livello fisico di Zigbee (**802.15.4**), lo stack aperto per l'industria **ISA100.11a** (IEC62734) lo integra con il livello di rete **6LowPan** e, a **livello applicativo**, realizza **funzioni** specifiche, di **monitoraggio**, **controllo** (catena aperta e chiusa) e **sicurezza** (sensori di gas). Oppure **OpenWSN** che utilizza l'implementazione di **TSCH** IEEE 802.15.4g/e come protocollo di accesso al mezzo e protocolli standard IETF come protocolli di livello superiore (6LowPan, REPL e UDP), o anche **OSS-7** che implementa la pila di protocolli **DASH7**. **DASH7** ha la particolarità di avere un **livello fisico** praticamente **agnostico**, cioè supporta la maggior parte dei protocolli di accesso disponibili come, ad esempio, LoRa o IEEE 802.15.4g inoltra possiede la **particolarità insolita** di mapparsi perfettamente sullo stack ISO/OSI, cioè definisce un **protocollo per ogni livello OSI**.

<img src="openwsn-dash7stack.png" alt="alt text" width="1000">

Gli **stack personalizzati** o **opensource** sono utili per realizzare **soluzioni ad hoc** calate in un ben preciso **contesto aziendale** come, ad esempio, la gestione dell'automazione **HVAC** (riscaldamento, ventilazione e condizionamento dell'aria). In questo caso per una **stessa funzione**, magari in zone diverse, sono dislocata **tecnologie diverse**, cioè **reti WSN eterogenee** oppure **miste** wireless e cablate, che magari si ha l'interesse a rendere **interoperabili** ai **livelli superiori** del loro stack OSI. In **questo contesto** sono ancora dominanti, in ambito WSN, il **WiFi** e le tecnologie cellulari (**NB-IoT**) indoor. In ambiti specializzati cercano di farsi strada, tra gli stack e i protocolli non citati, Insteon, Wavenis, Webree, Z-wave, ANT+ e CSRMesh.

Uno **stack commerciale**, essendo completo fino al **livello di applicazione**, ha il vantaggio di garantire una grande **interoperabilità** con tutti i dispositivi in commercio a prescindere dalla loro marca, l'importante è che sia **standardizzato** e tutti i **vendor** derivino i loro dispositivi da esso. Si consideri, ad esempio, una **rete mesh (magliata) Zigbee** (ma la situazione sarebbe analoga per il Blutooth BLE). Questa è composta da **router**, cioè nodi di smistamento, che, dispiegati in numero adeguato, hanno il compito di allargare le dimensioni della rete, ben oltre la linea di raggiungibilità del nodo sorgente dei comandi, ipoteticamente in ogni angolo della casa. In un contesto domestico (**home automation**), i **router** sono essi stessi **dispositivi commerciali** acquistati non curandosi affatto della loro funzione di rete (routing) ma semplicemente tenendo conto di una **funzione** di utilità domestica come ad esempio creare un punto di illuminazione (lampadina). Se i dispositivi **condividono** tutti lo **stesso stack WSN** il vantaggio per l'utente è chiaro. Con l'acquisto di un dispositivo con **alimentazione costante** (magari perchè **collegato alla rete**) l'utente compra anche il **dispositivo** che permette di **espandere la rete**, e ciò a beneficio non solo dei dispositivi del brand del prodotto acquistato ma anche a vantaggio di quelli **già installati** e di quelli che **si acquisteranno** in futuro. La **rete** diventa una infrastruttura completamente **trasparente** all'utente **basata** fisicamente e funzionalmente su tutti gli **oggetti smart** con cui egli popola la propria abitazione. Il **protocollo** di **routing dinamico** della rete garantisce l'**affidabilità** dell'infrastruttura ricalcolando, in maniera **trasparente all'utente**, il percorso di un messaggio di azionamento in caso di **guasto** di un nodo di transito.


### **Stack wireless orientati allo smart metering**

**Wireless M-Bus**

Detto anche **Wireless Meter-Bus** è lo **standard europeo (EN 13757-4)** che specifica la **comunicazione** tra **contatori** delle utenze e **gateway** per contatori intelligenti.
Sviluppato come standard per soddisfare la necessità di un sistema wireless di lettura dei contatori delle utenze in Europa, Wireless M-Bus viene adesso utilizzato come base per la nuova Infrastruttura di Misurazione Avanzata (**AMI**).
Il **DLMS** è uno standard di tipo aperto ratificato come IEC 61334-4-41. E' un protocollo di **livello applicativo** che definisce i requisiti minimi per un **contatore** affinché possa comunicare con altri dispositivi DLMS. Vengono, in particolare, stabiliti tutti i **tipi di misure** che il contatore deve essere in grado di rilevare e le relative **modalità di invio**, su richiesta di un client remoto, oltre ai requisiti in termini di **performance** che il dispositivo deve garantire. Il DLMS, inoltre, è uno standard adatto non solo alle sole reti elettriche, ma anche alle reti di altri **vettori energetici**, quali **gas**, **calore** ed **acqua**. 

**IEEE 802.15.4g**

Lo **IEEE 802.15** WPAN Task Group 4g (TG4g) ha proposto questo standard per estendere il corto raggio dello standard di base IEEE 802.15.4 (livello fisico Zigbee) per le reti di **smart metering** intelligenti o Smart Utility Network (SUN). Definisce tre livelli fisici intercambiabili, uno basato su frequency shift keying (FSK), accesso  multiplo a divisione di frequenza ortogonale (OFDMA), e sfasamento in quadratura offset keying (OQPSK). Fornisce copertura di diversi km. Le velocità dati supportate vanno da 40 kbps a 1 Mbps a seconda del livello fisico e regione in cui opera. Funziona secondo il principio di CSMA/CA e supporta stella, mesh, e altre topologie. E' il **livello fisico** dello stack commerciale **Zigbee NAN**, di quello **Wi SUN**, oltre che di quello open source **DASH7**. Nati per il contesto dello **smart metering** ambiscono ad un utilizzo per l'IoT generico in ambito **LPWA** a corto raggio.

**NB-Fi**

WAVIoT ha sviluppato il protocollo **LPWAN aperto**, chiamato NB-Fi, che opera nella banda radio ISM senza licenza. Il protocollo NB-Fi consente comunicazioni wireless a **lunghissimo raggio** (fino a 10 km in aree urbane, fino a 30 km in aree rurali) a **basso consumo** energetico. Poiché le bande ISM sub-1 GHz sono affollate, i gateway sono progettati per funzionare con un algoritmo di **prevenzione delle interferenze**. Per ottenere una migliore efficienza nell'allocazione della banda e migliori prestazioni, impiega algoritmi  basati sulla **tecnologia SDR**, **reti neurali** e tecniche di **intelligenza artificiale**. Nato per il contesto dello **smart metering** si vorrebbe utilizzarlo in quello **più ampio** dell'**LPWA** in generale.

### **Stack wireless orientati all'energy harvesting**

La tecnologia **EnOcean** utilizza un **protocollo ottimizzato** per il risparmio energetico (**energy harvesting**) standardizzato come ISO/IEC 14543-3-10 Home Electronic Systems (HES) protocollo Wireless Short-Packet (WSP). 
I **pacchetti** di dati wireless EnOcean sono relativamente **piccoli**, essendo lunghi solo 14 byte e trasmessi a **125 kbit/s**. L'**energia RF** viene trasmessa solo per **gli 1** dei dati binari, riducendo la quantità di potenza richiesta. **Tre** pacchetti vengono inviati a **intervalli pseudo-casuali** riducendo la possibilità di collisioni di pacchetti RF.

L'**harvesting** consiste nel **recupero** dell'energia necessaria alla trasmissione o alla ricezione dall'**ambiente** mediante lo sfruttamento di movimenti meccanici e altri potenziali ambientali come la luce interna e le differenze di temperatura. Per trasformare le fluttuazioni di energia magnetica, solare e termica raccolte dall'ambiente in energia elettrica per alimentare il dispositivo, vengono utilizzati **convertitori di energia** molto compatti.

I **dispositivi**, come sensori e interruttori della luce, funzionano **senza batterie** e i **segnali radio** di questi sensori e interruttori possono essere trasmessi in modalità wireless fino a una **distanza** di 300 metri all'**aperto** e fino a 30 metri all'**interno** degli edifici. 

<img src="mbus-enoean.jpg" alt="alt text" width="1000">

>[Torna a reti di sensori](sensornetworkshort.md#stack-wireless-specifici-per-IOT)



