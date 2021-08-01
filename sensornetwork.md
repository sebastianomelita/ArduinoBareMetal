>[Torna all'indice generale](index.md)
## **Reti ethernet da ufficio**

Le **reti di ufficio** sono realizzate con tecnologie ethernet a topologia fisica a stella o a stella gerachica (albero). Lo **switch** ha la funzione dI:
- **concentratore di dispositivi**. Un **collegamento punto punto** è dedicato ad ogni din paralleloispositivo che occupa esattamente una porta sul concentratore. Servono tante porte (e tanti cavi) quanti sono i dispositivi **in prossimità** del concentratore. L'**uso esclusivo** di un cavo da parte di un dispositivo è detto **microsegmentazione** e di fatto azzera il problema delle collisioni. Se la porta è **condivisa da più dispositivi** tramite un **collegamento a BUS** o un **HUB** si dice che lo switch realizza una **macrosegmentazione**, situazione in cui le collisioni ci sono e sono **arbitrate** dal protocollo Ethernet (di tipo CSMA/CD) abilitato sulla porta.
- **commutazione** cioè lo **smistamento** di una **trama MAC** tra le **dorsali** della rete LAN che, comprendendo certi switch e non altri, realizza un **percorso** (path) fino ai router di confine della stessa (**gateway**).

**Più switch** collegati insieme **realizzano una rete LAN** che si estende fino al primo router che essi incontrano. I **router** sono i dispositivi che, di fatto, **delimitano** una LAN Ethernet.

Lo **switch** è generalmente regolato dal **protocollo STP** che, secondo lo standard, limita i collegamenti a cascate a poche unità (profondità nominale di 3 dispositivi secondo standard EIA/TIA).

Il **cablaggio** può risultare **oneroso** in presenza di un elevato numero di dispositivi poichè richiederebbe l'impiego di **un cavo a parte** per ogni sensore. Per questo motivo dispositivi di commutazione e architettura **tradizionali** sono generalmente ritenuti poco adatti per la realizzazione delle **ampie reti di sensori** diffuse in ambito industriale.

![core-access-dist-banner_1](hierarchical-internetworking-model-for-distribution-switch-vs-access-Switch-vs-core-switch.jpg)

## **Reti ethernet industriali**

Le **reti industriali o ferrotramviarie** sono anch'esse a **tecnologia ethernet** ma, oltre la tradizionale **topologia a stella**, per connettere tra loro i dispositivi **terminali** e per connettere tra loto **più switch** spesso utilizzano **topologia fisica a BUS o ad anello**.
Un **anello** è composto da lunghe **cascate** di switch collegati in serie ed equipaggiati con protocollo STP modificato o con altri protocolli proprietari.
Il **cablaggio**, in presenza di un cluster numeroso di dispositivi **risulta più economico** dato che con **un unico cavo** si possono collegare più switch.
Possibilità di topologie **ridondate a doppio anello** (treni, industria)

![industrialnet](industrialnet.jpg)

## **Reti di sensori e attuatori**

Spesso sono **reti miste** cioè composte da **sottoreti eterogenee**: 
- **più reti di sensori** costituite da dispositivi sensori e da nodi di smistamento. Sono spesso realizzate in tecnologie non IP proprietarie.
- **una rete centrale di distribuzione**. E' una rete IP e può essere una LAN, spesso interconessa ad Internet, oppure Internet stessa.

 <img src="sensornet1.png" alt="alt text" width="1000">
 
Tra le due tipologie di reti ci sono dei **nodi di confine** aventi una interfaccia **nella rete di sensori** ed un'altra **nella rete principale IP**. Questi nodi **cerniera** sono detti **gateway** e devono trovare un modo di **rendere compatibili** i messaggi che circolano nelle due tipologie di reti, spesso realizzate con tecnologie **completamente diverse**.

La **rete principale**, **è di tipo ethernet** con dorsali fisiche a stella cablate e collegamenti periferici cablati o wireless WiFi. Ha principalmente la funzione di **distribuzione** dei dati **dai nodi gateway** con le reti secondarie **fino al server di gestione** dei dati. Va attentamente progettata perchè sia in grado di smaltire il traffico complessivo di **tutti i sensori**. Può diventare **critica** se, oltre ai sensori, sono presenti **sorgenti multimediali** quali **microfoni** o **telecamere** di videosorvelianza.

Le **reti di sensori** sono molto **eterogenee**. Tutte hanno un loro modo di **accedere al mezzo trasmissivo** stabilito dai rispettici protocolli di **livello fisico**. Allo stesso modo, tutte hanno un loro modo di **fornire un servizio all'utente** in maniera più o meno astratta, più o meno espressiva, più o meno varia dal punto di vista della QoS cioè della qualità del servizio, stabilito dal protocollo di **livello applicativo**. 

Il **gap** tra il livello **fisico** e quello **applicativo** in genere è colmato da tutta una serie di **protocolli** il cui scopo è essenzialmente: risolvere determinati **problemi di rete** (indirizzamento, controllo di errore, routing, cifratura, ecc.), ottenere la **QoS richiesta dall'applicazione** (velocità, BER, tipo di interazione, ecc...). **Entrambi gli obiettivi** precedenti devono essere raggiunti **organizzando**, tra livello fisico e livello applicativo, una **pila (stack) di protocolli** che permette di raggiungerli rispettando i **vincoli** posti dall'**ambito di utilizzo** pratico della rete (consumi, dimensioni, distanze, facilità d'uso, ecc...).

L'**ISO/OSI** ha catalogato e organizzato in una **architettura a 7 strati** tutti i **protocolli** concepiti nel tempo organizzandoli in base alle **categorie di problemi** di rete che essi risolvono. L'architettura è normalmente rappresentata sotto forma di **stack** (pila) e, per ogni elemento dello stack, identifica una **categoria di protocolli** che deve risolvere **un determinato insieme di problemi** della comunicazione in rete. L'architettutura è **modulare** nel senso che uno strato può essere sostituito senza che quelli sopra di lui se ne accorgano permettendo una grande **flessibilità**. Questà flessibilità può essere utilizzata per **adattare** un protocollo di livello superiore a **vari mezzi fisici** o per ottenere su un certo mezzo fisico una determinata **qualità del servizio**. **Obiettivo finale** è garantire l'**interoperabilità tra i dispositivi** da collegare a dispetto di qualunque differenza reciproca riguardo a marca, modello e tecnologie. 

**I canali virtuali** si dividono in link **end to end**, chiamati così perchè collegano livelli che sono presenti **soltanto** negli **host** cioè i **dispositivi terminali**, quelli su cui si **interfaccia l'utente**. Più in basso (sui **primi 3** livelli), stanno i **link IS-IS** (collegamento tra  Intemediate systems), chiamati così perchè collegano tra loro i **dispositivi di rete**, cioè quelli che creano la rete. Anche gli host posseggono questi livelli e pertanto sono, a tutti gli effetti, dispositivi di rete anch'essi .

<img src="iso-osi1.png" alt="alt text" width="1000">

Un **vincolo dell'architettura** è che uno strato può dialogare solo con quelli adiacenti, o sopra o sotto di lui. Nelle **reti a pacchetto**, in fase di **trasmissione**, si usa una tecnica, detta **imbustamento multiplo**, con la quale si **incapsulano** i messaggi di un livello, le **PDU** (Protol Data Unit),  nel campo dati, le **SDU** (Service Data Unit), del livello **immediatamente inferiore**. Questa operazione parte dal livello applicativo ed è **ripetuta** in tutti i livelli, escluso il fisico. Il **carico utile** o **payload** è il **messaggio** che la rete ha in carico, da parte dell'utente, per essere **spostato** fino alla destinazione. Ogni livello **aggiunge** al messaggio utente informazioni di controllo sue proprie nel campo **header**. Il **livello fisico**, a seguito di tutti gli annidamenti, ha il **messaggio più lungo** perchè è composto dal messaggio utente a dalla **somma di tutti gli header**, detta, in gergo tecnico, **overhead**. 

All'**imbustamento multiplo** in **trasmissione** corrisponde lo **sbustamento multiplo in ricezione**, un'operazione analoga ma **inversa**, dove tutti i messaggi ricevuti a **livello fisico** incapsulati uno dentro l'altro, vengono, salendo da un livello all'altro, **sbustati** eliminando l'intestazione del livello immediatamente inferiore. Arrivati al **livello applicativo**, come risultato, si ottiene il **messaggio utente**.

**L'imbustamento multiplo** permette la creazione dei cosidetti **canali virtuali** cioè dei collegamenti **apparenti** e **diretti** tra **strati corrispondenti** di dispositivi **remoti**. La particolarità di questi collegamenti è quella di realizzare una **virtualizzazione della rete** e dei suoi dispositivi che, man mano che si sale dal livello fisico a quello applicativo, si fa sempre più spinta, **nascondendo** i dispositivi degli strati inferiori e rendendo **impercettibili** le loro **differenze** ai livelli sopra di essi. Nel caso di una rete IP il **livello TCP** di trasporto già non **"vede"** più le differenze tecnologiche tra reti realizzate con un HUB WiFi o con uno switch oppure con più router in cascata, essendo stati questi dispositivi tutti **"astratti"**, cioè riassuntati in un **unico collegamento** di tipo **TCP** tra **due host terminali**.

Il **canale reale** è il **mezzo trasmissivo** che unisce il **livello fisico** dei due interlocutori. Si sviluppa in **orizzontale** lungo il mezzo trasmissivo ma anche in **verticale** lungo le interfacce tra i vari strati, **a scendere** in trasmissione, **a salire** in ricezione.
I messaggi dei vari livelli, **incapsulati** uno dentro l'altro, vengono trasmessi in un **blocco unico** lungo il **canale reale**. Ma per comodità, all'**interno dei dispositivi**, sono stati **destrutturati** evidenziando, nei 7 **canali virtuali** dei singoli livelli, l'**abbinamento** tra il messaggio trasmesso da quel livello e il messaggio ricevuto, nello stesso livello, dalla controparte. **In sostanza**, i messaggi **generati** e **trasmessi** dai 7 livelli **separatamente** lungo i 7 **canali virtuali**, diventano un **unico messaggio matrioska** di 7 messaggi, **annidati** uno dentro l'altro, quando vengono trasmessi lungo il **canale reale**. 

I **canali virtuali non esistono fisicamente** ma sono **ugualmente reali** perchè, dal punto di vista dei **messaggi scambiati**, le **entità** pari (peer entity), cioè gli interlocutori corrispondenti nei vari livelli, si comportano come se essi ci fossero davvero e attraverso questi **effettivamente dialogassero**. Sono una **visione semplificata** con cui, ad un **livello superiore**, si possono **osservare** le funzioni svolte e i messaggi scambiati dai livelli inferiori. Semplificata, ma comunque reale.

In figura la lettera H sta per **header** cioè intestazione (ad es. AH=Application header).
<img src="iso-osi.png" alt="alt text" width="1000">

Lo **stack di protocolli OSI** è un **modello architetturale** di riferimento. Per **ogni strato** sono stati studiati nel tempo un gran numero di protocolli, ciascuno con i propri **pregi e difetti**. Un'**architettura reale**, quella che poi verrà standardizzata ed implementata in un dispositivo commerciale, per ogni strato della propria pila, sceglierà, tra i tanti disponibili in letteratura tecnica, un certo **tipo di protocollo** del quale realizzerà e standardizzerà la propria **particolare versione**. Attualmente, per **accedere ad Internet** e all'interno della maggior parte delle **reti locali LAN**, si adopera la cosidetta suite **TCP/IP**. Ma molte **reti di sensori**, per funzionare localmente al loro ambiente di lavoro, **non sempre usano** la suite TCP/IP. Inoltre, anche le reti di sensori che l'adoperano, ai livelli inferiori come **i livelli data link e il livello fisico**, spesso utilizzano protocolli **diversi** da quelli che sono stati standardizzati per le LAN. 

In ogni caso, un qualsiasi **ente di telecomunicazioni internazionale** (IEE, IETF, ITUT, ecc.) o **alleanza di case produttrici** di dispositivi di rete (Zigbee, Bluetooth) o anche **singole aziende** (LoraWan e Sigfox) per realizzare reti di dispositivi devono **instanziare** una **propria versione** dello **stack ISO/OSI**. 

Occorre però tenere presente che una **interoperabilità completa** tra dispositivi di **tecnolgie diverse** si può ottenere **solamente**: 
- **creando un'unica rete** utilizzando livelli di rete **compatibili** o **praticamente uguali** come sono **IPV6 e 6LowPan**. Lo stesso deve accadere per i **livelli superiori**. In particolare nell'ultimo, il **livello applicativo**, il **payload dell'applicazione**, cioè il messaggio utente, deve poter viaggiare **direttamente** dal **client** al **sensore**, dove poi verrà elaborato e utilizzato.
- **utilizzando un gateway** tra livelli **di applicazione** diversi (detto talvolta **bridge**), che si occupa di **tradurre** i messaggi del livello di applicazione dal formato in uso nella rete  IP a quello in uso nella rete non IP **invocando le API** di questa. Un problema potrebbe sorgere se allo stesso gateway colleghiamo due reti proprietarie non IP diverse, questo sarebbe tenuto a conoscere anche le API applicative dell'altra rete di sensori.

**Unica rete con IPV6 e 6LowPan**:

<img src="jsan-02-00235-g002.webp" alt="alt text" width="700">

**Gateway applicativo**:

<img src="jsan-02-00235-g001.webp" alt="alt text" width="700">

Il **gateway** ha tante **schede di interfaccia** quanti sono i **tipi diversi di BUS** a cui si collega. Il **gateway** deve possedere anche **una interfaccia** capace di traffico ethernet (cablata o wifi) che lo collega alla **rete di distribuzione** (in genere cablata).

Il **gateway** ha anche la funzione di adattare il **formato dei servizi** offerti dalle varie **sottoreti di sensori** nel **formato di servizio unificato** (ad esempio un particolare messaggio JSON) con cui i sensori sono interrogati nella rete di distribuzione IP. I **protocolli di livello applicativo** utilizzati a questo scopo in genere sono **HTTPS** o **COAP** per il paradigma di interazione **Request/response** oppure **MQTT** o **Telegram** per il paradigma di interazione **Publish/Subscribe**, oppure **Websocket**, **Webhooks** e **WebRTC** per richieste asincrone, l'ultimo anche per quelle multimediali.

Talvolta il livello applicativo dello stesso stack di protocolli è non compatibile tra una marca e l'altra di dispositivi. In più i **gateway** verso la rete di sensori, nonostante abbiano nella rete di distribuzione una **interfaccia MAC/IP**, sono scatolotti **proprietari chiusi**, cioè, **non modificabili**. I fabbricanti magari tendono a renderli **non bypassabili** perchè non rilasciano di pubblico dominio le API per comunicare con i sensori. Questo significa che potrebbe essere necessario per loro un **ulteriore livello di traduzione**, cioè un altro gateway, per convertire il **formato** delle **API** e dei **protocolli** che espongono (ad es COAP) con quelle in uso nella rete di distribuzione per tutti i sensori (ad es. MQTT). Questa situazione, per i **protocolli più diffusi** come Zibgee e Bluetooth, **tende a scomparire** perchè quasi tutti i produttori ormai si sono accordati e **adottano** per le API dei servizi uno **standard comune** e pubblico, per cui non è insolito riuscire a comandare dispositivi di una marca col gateway di un'altra. 

Alla **rete di distribuzione IP** si collegano, quindi, una o più **reti secondarie** che servono da **rete di accesso** per i dispositivi sensori o attuatori con **interfacce** spesso di tipo **non ethernet** che necessitano di un **gateway** di confine con possibili funzioni di:        
  - **Traduzione di formato** dei messaggi da rete a bus a rete ethernet con evenruale realizzazione del **bridge** tra livello applicativo in uso nella rete di sensori e quello in uso nella rete di distribuzione.
  - **Interrogazione periodica** (polling) dei dispositivi (master di una architettura master/slave)
  - **Raccolta e memorizzazione** delle informazioni per essere trasferite in un secondo momento al server di gestione
    
## **Server di gestione dei servizi**

In genere è localizzato all'intermo della **rete principale** con una collocazione **on-premise** all'interno del sistema.

Tendenze sempre più diffuse portano al **trasferimento crescente di funzioni** anche sul **cloud**. Una **soluzione estrema** è quella di spostare **tutte le funzioni** sul cloud tenendo ben presente che un **guasto della connessione ad internet** causerebbe una cessazione delle **funzioni** di regolazione e controllo che sono state progettate per essere eseguite **centralizzate sul server**. Le funzioni gestite in maniera autonoma e peer to peer **completamente a bordo** dei dispositivi non dovrebero risentire di particolari problemi.

In ogni caso è necessario un **server di gestione** con funzioni di:
- Processamento (elaborazione nuovo stato e comando attuatori)
- Memorizzazione (storage) ed estrazione (mining) delle informazioni
- Analisi dei dati per estrarre reportistica di aiuto alle decisioni (risparmio energetico)
- Pubblicazione in Internet delle informazioni su un un sito o su un WebService (opendata)
- Segnalazione anomalie
- Backup dei dati e gestione disaster recovery di dati e servizi
- Aggiornamento via cavo o via etere (OTA) del firmware ai nodi
- Gestione della sicurezza

Queste prese in considerazione sono generalmente tutte funzioni di **livello applicativo** in quanto si limitano a gestire servizi.

## **Server di gestione della rete**

Sono delle macchine sede di elaborazioni necessarie per lo svolgimento delle funzioni di rete di tutti i dispositivi ad essa collegati (**funzioni di controllo**). Si tratta di funzioni che interessano i primi 3 livelli della pila ISO/OSI tipicamente il **livello fisico** come la gestione della potenza dei terminali e parametri di modulazione (ad es. spreading factor), il **livello di di linea** come la gestione dei messaggi duplicati; il **livello di rete** come la gestione centralizzata del routing.

Quindi sono macchine che partecipano attivamente alle **funzioni di rete** e pertanto fanno esse stesse parte della **infrastruttura di rete**. Un'analogia con le LAN potrebbero essere i server DHCP e DNS che pur non essendo a rigore dispositivi di rete, sono comunque macchine **essenziali** per il **suo funzionamento**. Un'altro esempio può essere il **controller** che gestise il **roaming** degli utenti autorizzati tra le aree di copertura di una rete **WiFi** o **cellulare**. Il concetto è che **fanno parte** dell'infrastruttura e non sono dislocate presso gli utenti in quanto le funzioni che esse svolgono sono **comuni** e fruite da tutti.

Sono presenti in quasi tutte le **infrastrutture LPWA** a lungo raggio come **LoraWan**, **Sigfox** e **NB-IoT**.

<img src="lpwan.png" alt="alt text" width="700">


##  **Topologia delle reti di sensori cablata** 

Nelle **reti industriali** sono molto comuni topologie complesse a più livelli. Per le applicazioni di **nostro interesse** le **topologie** più adoperate sono quelle classiche a **stella** e a **bus**. **I collegamenti** dei sensori, in questo caso, si attestano sulle **porte** di **schede programmabili** che ospitano dispositivi a microcontrollore. Le stesse schede hanno, in genere, funzione di **gateway** verso la **rete di distribuzione** per cui posseggono almeno **due interfacce**: una **verso il BUS** ed un altra, ethernet ed IP **verso la rete LAN**. Inoltre le schede ospitano le **librerie** SW di **gestione del BUS** e il codice per realizzare il **bridge** tra il **livello applicativo** in uso nella rete di sensori e quello in uso nella rete IP. Il **bridge** è generalmente realizzato o da librerie di **terze parti** oppure direttamente dal **programmatore** in C/C++ o, se la potenza di calcolo della scheda lo consente, mediante linguaggi di scripting come **Python** o **Node.js**. Si realizzano spesso anche in maniera **visuale** adoperando **framework** come **Nodered** e **Openhab**.

### **Esempi di collegamenti base con topologia a stella**:

  <img src="Star-network-1024x466.png" alt="alt text" width="700">
  
  Accesso in scrittura e in lettura su due **porte digitali** di un sensore ad ultrasuoni:
  - si collega su una delle porte **digitali** del microcontrollore e trasporta bit codificati
  - sono **punto-punto**, cioè **un filo** (escluso GND che comunque va collegato al sensore) per ogni sensore
  - il **collegamento** è in genere **simplex** cioè in una direzione sola, da o verso il sensore
      - se in **ingresso** al microcontrollore si collega ad una porta impostata in modalità **INPUT**
      - se in **uscita** al microcontrollore si collega ad una porta impostata in modalità **OUTPUT**

  <img src="ultrasonic-sensor-with-arduino-hc-sr04.jpg" alt="alt text" width="700">
  
  Accesso in in lettura su una **porta analogica** di un sensore di luminosità:
  - si collega su una delle porte **analogiche** del microcontrollore e trasporta bit codificati
  - sono **punto-punto**, cioè **un filo** (escluso GND che comunque va collegato al sensore) per ogni sensore
  - il collegamento è in genere **simplex** cioè in una direzione sola, da o verso il sensore
      - se in **ingresso** al microcontrollore si collega ad una porta **ADC** (Analog Digital Converter) del microcontrollore
      - se in **uscita** al microcontrollore si collega ad una porta **PWM** (Pulse Width Modulation) del microcontrollore
  
  <img src="potenziometro.png" alt="alt text" width="500">

### **Esempi di collegamenti base con topologia a bus:**

Il BUS è un mezzo ad **accesso multiplo** composto da un **unico filo** per **tutte** le stazioni. Il tipo del collegamento **in parallelo** dei dispositivi lo rende un **mezzo broadcast** cioè un mezzo dove se una stazione **parla** tutte le altre ricevono ed **ascoltano**. I mezzi broadcast sono soggetti a **collisioni** che devono essere risolte costringendo le stazioni a parlare **una alla volta** mediante un **protocollo di arbitraggio**. Questi si dividono in due categorie: **centralizzati** e **distribuiti**. Quelli con cui si collegano i **dispositivi sensori** sono spesso **centralizzati** e comunque lo sono la maggior parte di quelli che vedremo di seguito (tranne l'ultimo).

**Arbitraggio centralizzato Master/Slave**

In un sistema **centralizzato** una stazione è più importante delle altre perché può avere l’**iniziativa di cominciare** una comunicazione ed è detta **Master**. Le altre stazioni hanno un **ruolo passivo**, nel senso che rispondono solo se sono interrogate dal master mediante una tecnica detta polling e sono dette pertanto **Slave**. Il **polling** equivale all’**interrogazione periodica** di più dispositivi seguita da una **risposta immediata**. Il master decide **chi** parla e **quando** parla a rotazione, seguendo un ordine prestabilito detto **round robin**, secondo il quale si scandisce ogni interazione richiesta-risposta. 
Esistono sistemi centralizzati con un **solo master** e altri **multimaster** in cui il controllo del BUS passa alternativamente da un master all'altro. La presenza di più master richiede comunque la concomitante presenza un **meccanismo di sincronizzazione** per evitare **sovrapposizioni** nell'accesso al BUS (lo devono controllare in maniera esclusiva ed uno alla volta).

**Arbitraggio distribuito CSMA/CA**

In un sistema **distribuito a filo** spesso si usano **sistemi di arbitraggio CSMA/CA non distruttivi** come (Carrier sensing multiple access/collision arbitration). La sincronizzazione dei bit tra le stazioni deve esse molto accurata. Ogni elemento ascolta il bus. Quando lo sente libero, avvia la comunicazione. Se due lo fanno insieme, un meccanismo di arbitrazione permette ad una sola trasmissione di continuare.
Nel protocollo CAN si definiscono i bit come **"dominante"** e come **"recessivo"**. Un bit dominante vince su un bit recessivo. Se un nodo trasmette un bit dominante e contemporaneamente un altro nodo trasmette un bit recessivo allora sul bus sara’ presente quello dominante. Nel protocollo CAN si definisce il bit logico **0** come bit **dominante** e il bit logico **1** come bit **recessivo**. Dal punto di **vista elettrico** e di interfaccia si può pensare che il bit 1 sia forzato sul bus da un resistore di pull-up mentre lo zero dominante dal un trasnsistor con il drain sul bus.
Se un nodo **rileva** un **livello dominante** quando **sta inviand**o esso stesso un **livello recessivo**, uscirà immediatamente dal processo di arbitraggio e si metterà in **ascolto**, in attesa della **fine della trasmissione dominante**.


 <img src="bus.png" alt="alt text" width="600">
 
In genere i BUS si trovano in due stati **IDLE** (riposo) e **comunicazione**. Lo stato di **IDLE** è quello **predefinito** ed è ottenuto tramite una **resistenza di pullup** colegata sulla linea. Durante lo stato di **comunicazione** il **valore bit 0** è codificato con il livello basso L=0Volt mentre il **valore bit 1** è codificato con lo stesso valore dell'IDLE cioè **H**. L'**inizio della trasmissione** di una trama è in genere **segnalata** da uno livello basso **L iniziale** detto **start bit**.

Ci sono **sistemi a BUS** che si **connettono direttamente** alle **porte digitali** di un **microcontrollor**e senza **driver HW** aggiuntivi. I più diffusi sono 1-wire (Dallas), two-wire (I2C) e 3 o 4-wire (SPI). Il collegamento diretto, **senza driver HW**  (bit banging), alle porte digitali del microcontrollore genera sul BUS **segnali non bilanciati** circostanza che **limita la lunghezza** ammissibile dei collegamenti da **qualche metro** ad una **decina di metri**.

**Tutti i tipi di BUS** richiedono l'**installazione** di **librerie SW** per poter gestire l'**accesso al mezzo** dall'interno del **codice** di un applicativo. In genere, ciò si ottiene mediante **API** di servizio di **alto livello** e quindi comode da utilizzare. Allo scopo, nel codice del programma, o in un file di configurazione, va riportato il **mappaggio** tra il **numero di porta** e il **ruolo** della stessa all'interno del **protocollo**, per tutte quelle **adoperate** per realizzare il **BUS**.
 
Esempio di bus **Dallas** detto anche a 1 filo (**one-wire**):
- è costituito da **un solo filo**, escluso il filo **GND**, comune a tutti i collegamenti e che **va collegato** anch'esso **al sensore**.
- il dispositivo riceve l'alimentazione in CC **miscelata insieme ai dati**. Durante i bit zero, non ricevendo alimentazione dal cavo, il dispositivo la riceve da un condensatore tampone.
- alcuni dispositivi posseggono comunque una linea di **alimentazione separata** da quella dati esibendo quindi, complessivamente **3 terminali**.

 <img src="Schematic-dallas-18s20.gif" alt="alt text" width="600">

Esempio di bus **I2C** detto anche a 2 fili (**two-wire**):
- è costituito da **2 fili**, escluso il filo **GND**, comune a tutti i collegamenti e che **va collegato** anch'esso **al sensore**
- terminale **SDA** (Serial DAta) per i dati
- terminale **SCL** (Serial CLock) per il clock (per la presenza di questo segnale l'I2C è un bus sincrono)
 
 <img src="I2C-communication-protocol-ESP32.png" alt="alt text" width="700">
 
Esempio di bus **SPI** detto anche 4 fili (**four-wire**):
- è costituito da **4 fili**, escluso il filo **GND**, comune a tutti i collegamenti e che **va collegato** anch'esso **al sensore**
- terminale **SCLK** - SCK: Serial Clock (emesso dal master)  per il clock (per la presenza di questo segnale l'SPI è un bus sincrono)
- terminale SDI – **MISO** – SOMI – DI - SO: Serial Data Input, Master Input Slave Output (ingresso per il master e uscita per lo slave)
- terminale SDO – **MOSI** – SIMO – DO – SI: Serial Data Output, Master Output Slave Input (uscita dal master)
- terminale **CS** – SS – nCS – nSS – STE: Chip Select, Slave Select, emesso dal master per scegliere con quale dispositivo slave vuole comunicare (dalla figura, il segnale SS negato, si comprende che per comunicare con il dispositivo slave deve venire messo a livello logico basso)

 <img src="spi.png" alt="alt text" width="600">
 
 Esempio di bus **RS485**:
 - è costituito da **2 fili**, escluso il filo **GND**, comune a tutti i collegamenti che **NON** va collegato anch'esso al sensore
 - il **BUS RS485** è **bilanciato** per cui non è possibile collegare i suoi terminali direttamente sulle porte digitali di un microcontrollore ma bisogna interporre **un driver HW** che trasli i livelli da logica TTL (a 5V o 3.3V) a logica bilanciata AB.
 - i **fili** del BUS sono **attestati** sui **morsetti A e B** del driver. La differenza A-B codifica il valore del bit trasmesso.
 - le **linee bilanciate** permettono distanze che vanno da un **minimo** di 30cm ad un **massimo** di circa 1Km. 
 - il **numero di dispositivi** connettibile dipende dalle implementazioni e dalla massima corrente erogabile sul BUS, accreditato è un numero di almeno 32 dispositivi.
 - può essere arbitrato dal protocollo **Modbus** che gestisce diversi servizi codificati nel campo **function code** contenuto nel telegramma di ciascuna richiesta
 - può essere arbitrato dal protocolllo **DMX** (Digital MultipleX), uno standard di comunicazione digitale usato principalmente per il controllo dell'illuminazionedi scena nell'industria dello spettacolo.

 <img src="rs485network.png" alt="alt text" width="600">
 
  Esempio di attuatori collegati con BUS **RS485** arbitrato dal protocollo Modbus:

 <img src="Modbus-system-example.png" alt="alt text" width="600">
 
 Esempio di bus **CAN**:
 - è diffuso in automotive, in home automation è una possibilità ancora poco utilizzata.
 - a differenza di tutti i BUS precedenti, l'**accesso** è regolato con un protocollo di **arbitraggio distribuito CSMA/CA**
 - **scelta economica** se la scheda microntrollore possiede già con un driver logico (CAN Controller) a bordo realizzato in HW che espone una interfaccia CAN TTL nella pedinatura.
 - è costituito da **2 fili** CANH e CANL
 - il **CANBus** è **bilanciato** per cui non è possibile collegare i suoi terminali direttamente sulle porte digitali di un microcontrollore ma bisogna interporre **un driver HW** che trasli i livelli da logica TTL (a 5V o 3.3V) a logica bilanciata HL.
 - i **fili** del BUS sono **attestati** sui **morsetti CANH e CANL** del driver. La differenza CANH-CANL codifica il valore del bit trasmesso.
 - le **linee bilanciate** permettono distanze che vanno da un **minimo** di 40m ad un **massimo** di circa 13Km (1Mbps-5kbps). 
 - il **numero di dispositivi** connettibile dipende dalle implementazioni, dalla massima corrente erogabile sul BUS e dai ritardi, accreditato è un numero di almeno un centinaio di dispositivi.
 
 <img src="What-is-Canbus.jpg" alt="alt text" width="600">
 
Abbiamo visto BUS **centralizzati** e di uso generale che **definiscono** il **livello fisico** di una comunicazione via cavo. Degli altri livelli OSI o **non forniscono** affatto la definizione o ne forniscono una **molto essenziale** sotto forma di **function code**. Esistono molti altri **stack commerciali** che si occupano di domotica e automazione che a livello fisico utilizzano sia **reti cablate** che **wireless**. **Esempi notevoli** sono **KNX**, **BACNet**, **DALI**. 

Molti di questi forniscono una **definizione elaborata**, **standard** ed **esaustiva** di tutti i possibili **tipi di servizi** adoperabili nei vari **scenari pratici** dell'automazione (dall'accensione di una lampadina all'azionamento di una tapparella, ecc.) spesso **catalogati e raccolti** in strutture dati dette **profili**. Molti di questi BUS, per **accrescere l'affidabilità**, adoperano un **arbitraggio** del mezzo trasmissivo di tipo **peer to peer** usando, ad esempio, protocolli di tipo **ALOHA** o **CSMA/CA**. 

### **Stack cablati specifici per domotica e sensoristica**:

Molte architetture commerciali adoperano complessi BUS di campo adatti per impianti di grandi dimensioni e molto strutturati, con una cablatura parallela ed a parte rispetto a quella ethernet utilizzata dai dati.

<img src="tassobus.png" alt="alt text" width="700">

Due stack sofisticati utilizzati per la gestione dell'automazione degli edifici sono BACNet e KNX:

<img src="bacnet-knx.png" alt="alt text" width="1000">

**BACnet** è stato progettato per consentire la comunicazione dei sistemi di automazione e controllo degli edifici per applicazioni come il **controllo** del **riscaldamento**, della **ventilazione** e del **condizionamento** dell'**aria** (**HVAC**), il controllo dell'**illuminazione**, il controllo degli **accessi** e i sistemi di **rilevazione incendi** e le relative apparecchiature. Ha una **struttura centralizzata** er la gestione della rete e l'arbitaggio del BUS è di tipo **distribuito deterministico a gettone (token**). E' possibile utilizzare il suo stack all'interno del framework **RIOT OS**.

Il **protocollo BACnet** definisce un numero di collegamenti **ati/livelli fisici**, inclusi **ARCNET**, **Ethernet**, **BACnet/IP**, **BACnet/IPv6**, **BACnet/MSTP**, **Point-to-Point** su RS-232, **Master-Slave/Token-Passing** su RS- 485, **ZigBee** e **LonTalk**. 

**LonTalk** è un protocollo **molto sofisticato**, perché copre i 7 livelli ISO/OSI, può essere adoperato sopra **qualunque strato fisico**, utilizzando l'opportuno **transceiver**. L'accesso al bus è **distribuito** di tipo **CSMA/CD** e gestisce sottoreti fino a 128 nodi (con router fino a 32.000 nodi). La sua **velocità** arriva fino a 1,25 Mbit/s, ma può essere limitata dai mezzi trasmissivi. Una applicazione di questa tecnologia, **LonWorks**, è ampiamente utilizzata per la **telelettura** dei contatori dell'energia elettrica (**smartmetering**) e per il **telecontrollo** dell'illuminazione stradale.

**KNX** è uno **standard** di building automation **aperto**, **coperto da royalty**. Il marchio KNX non è una semplice dichiarazione del produttore, ma si basa sul superamento di **prove di conformità** effettuate dai laboratori di KNX. Le **aziende** interessate al marchio, al prezzo di una tassa versata all'associazione KNX, effettuano le prove ed ottengono la **certificazione** (non basta una **autocertificazione**)

Esistono **tre modalità** di KNX: **Automatic-Mode**, **Easy-Mode**, **System-Mode**.

Lo **standard KNX** prevede **diversi mezzi trasmissivi** che possono essere utilizzati in combinazione con uno o più modi di configurazione in funzione della particolare applicazione:
- **TP-1** (Twisted Pair, tipo 1): Mezzo trasmissivo basato su cavo a **conduttori intrecciati** con bitrate di 9600 bit/s, proveniente da EIB. Le comunicazioni sono **bidirezionali** e **half duplex** regolate da un protocollo di **arbitraggio peer to peer** di tipo **CSMA/CA**. I dispositivi comunicano direttamente tra loro senza l'ausilio di un master centrale. 
- **PL-110** (Power Line, 110 kHz): Mezzo trasmissivo ad **onda convogliata (power-line)** con bitrate di 1200 bit/s, proveniente da EIB. I prodotti certificati EIB e KNX PL-110 funzionano e comunicano fra di loro sulla stessa rete di distribuzione dell'alimentazione elettrica. Usano la modulazione **S-FSK** (spread frequency-shift keying) che genera un canale half dupex arbitrato da **CSMA/CA**.
- **RF** (Radio Frequency, 868 MHz): Mezzo trasmissivo in radiofrequenza con bitrate di 38.4 kBit/s, sviluppato direttamente all'interno della piattaforma standard KNX.
- **Ethernet** (KNXnet/IP): Mezzo trasmissivo ottenuto mediante il **tunneling** di frame KNX incorporati in frame IP (Internet Protocol) convogliate su **reti LAN** di qualunque tipo (**Ethernet**, **WiFi**).

### **Tendenze future**

Le **prestazioni** elevate delle **interfacce general purpose** (**Ethernet**, ecc.), la loro possibilità di alimentare via **POE** i dispositivi e la costante diminuzione del **prezzo**, sta introducendo una tendenza all’ utilizzo di queste ultime, che compensano con l’ eccesso di prestazioni e i bassi costi il fatto di non essere concettualmente adatte all’impiego in automazione. I **sensori** e gli **attuatori** o escono dalla fabbrica già dotati di **interfacce ethernet** oppure si collegano con i **protocolli** di livello fisico **basilari**, con collegamento a **stella** o a **BUS**, su **piccole schede** a microprocessore che, dotate di connettività ethernet, ricoprono il ruolo di **gateway**. Questa filosofia permette di creare reti a BUS **molto snelle** che si strutturano in maniera più articolata poggiandosi sulla stessa rete IP adoperata per i dati attraverso un **dispositivo gateway** con una interfaccia **sul BUS** e l'atra **sulla rete  LAN**.


 
##  **Topologia delle reti di sensori wireless**     
 
Le reti di **sensori wireless** hanno una estensione nello spazio variabile e la loro **topologia** preferita è **a stella** o **a maglia**.

![hops](hops.png)

La **rete principale ethernet** può **estendersi nello spazio** secondo un'architettura (ad albero o ad anello) che può comprendere molti switch. Lo **smistamento completo** di una trama dati lungo la rete può prevedere una catena di **smistamenti successivi** lungo gli switch (hops) più o meno lunga. Nel caso di una rete ethernet questo non è generalmente un problema eccessivo nè dal punto di vista dei ritardi nè rispetto a quello dei consumi (gli switch sono alimentati attraverso la rete elettrica).

La situazione potrebbe essere molto diversa nel caso delle **reti di sensori**. Queste sono spesso **wireless** e realizzate con dispositivi **alimentati a batteria** che si vorrebbe fosse sostituita idealmente  **non prima di un paio di anni**.

A seconda dello schema adoperato, dal **punto di vista energetico** non è indifferente considerare se **un comando** o **l'accesso in lettura ad un sensore** avvengono connettendosi direttamente **all'unico** dispositivo hub centrale (magari distante) o se avvengono connettendosi **al più vicino** di una cascata di nodi:
- **Hop singolo**: ciascun nodo del sensore è collegato all'**unico gateway centrale** posto generalmente in **posizione baricentrica** rispetto a tutti i dispositivi. Sebbene sia possibile la trasmissione a lunga distanza, il consumo di energia sarà significativamente superiore a quello speso per la misura del valore del sensore e per la sua elaborazione. Tecnica molto utilizzata per le **grandi distanze** dai gateway **LoraWan** e **Sigfox**, ma anche dalle BS (Base Station) **NB-IoT**  e **LTE-M** del 5G. Mentre per le **medio-piccole** è adoperata dagli AP **WiFi** e **Bluetooth BLE** in **modalità infrastruttura** (non ad-hoc).
- **Hop multiplo**: esiste un percorso verso i nodi terminali passando attraverso dei nodi intermedi. L’energia che un **singolo dispositivo** impiega per raggiungere **un nodo remoto** (ad esempio per accenderlo) si limita a quella per raggiungere il **nodo più prossimo**. Su reti mesh (magliate) di **grandi dimensioni** anche questa energia può essere non trascurabile. Su **reti indoor** è invece la soluzione decisamente **più conveniente** dato che spesso **il primo** dispositivo di smistamento raggiungibile (router o switch) non è a più di un metro di distanza mentre quello in **posizione baricentrica** può essere anche a 10-50m di distanza.

Le reti di sensori wireless (WSN) ad **hop multiplo** in realtà sono di **due tipi** di base più una loro **ibridazione**:
- a **stella gerarchica** In questa categoria, molto popolare è il protocollo Low Energy Adaptive Clustering Hierarchy **(LEACH)**. In LEACH i nodi dei sensori sono divisi in diversi **cluster**. In ogni cluster, il nodo con il livello di energia **più alto** è generalmente scelto come la **testa del cluster** o CH. I CH cono **centro stella** delle comunicazioni con funzione di **aggregazione** e **compressione** dei dati provenienti dai diversi sensori per **ridurre** il numero di **messaggi trasmessi** alla destinazione. Il **ruolo del CH** viene **ruotato** allo scopo di distribuire il carico energetico tra i sensori. Benchè abbia una **grande efficienza energetica** ha delle limitazioni dovute ad alcune ipotesi di progetto. Una di queste queste, il vincolo che i nodi abbiano una potenza di trasmissione tale da poter sempre raggiungere la BS, lo rende **inadatto a reti estese**. Ritenuto attualmente particolarmente adatto ai **requisiti di una WSN** è invece il protocollo **IETF RPL**. Progettato per le reti **6LoWPAN** è un protocollo di tipo **distance vector** che genera, attraverzo lo scambio di messaggi DV con i nodi vicini, un albero di instradamento ottimo **unico** per tutti i nodi avente per **radice** la BS (gateway). E' il protocollo di routing adottato da molti framework per IoT come **TinyoS** o **RIOT**. Anche le **piconet** nelle reti di dispositivi **Buetooth** (IEEE 802.15.1) utilizzano una loro implementazione di routing gerarchico.
- a **maglia** anche detti **flat** o **peer to peer**. Rete **omogenea** di dispositivi in cui tutti i nodi di smistamento (router) **sono simili** in termini di **energia**, **risorse di calcolo** e **memoria**. In questo caso  **ogni nodo** si calcola **il proprio** albero di instradamento ottimo per raggiungere gli altri nodi e la base station. Gli alberi, avendo, di volta in volta, per **radice un nodo** differente sono generalmente anch'essi diversi. Il protocollo di instradamento è spesso distribuito e dinamico e, rispetto all'altra soluzione, garantisce **percorsi ottimali** per **tutti i nodi**, **non solo** per la base station. Un protocollo per reti flat molto usato è **AODV** ed è un derivato per reti ad-hoc del protocollo **distance vector**. Generalmente i protocolli flat mostrano **migliori prestazioni complessive** in termini di **banda** e **ritardo** ma il loro **consumo energetico** **molto elevato** riduce la durata di attività di ciascun **nodo router** a meno che esso non sia **perennemente alimentato**. Tecnica adoperata dai dispositivi **Zigbee** (IEEE 802.15.4).
- **mista** delle precedenti, ovvero **albero di cluster** dove un **cluster** è formato da una **rete a maglia** di piccole dimensioni. Viene così realizzata una rete peer-to-peer con un **minimo overhead** di routing, proprietà che induce un **più basso consumo** dei dispositivi router. Il **routing interno** tra i nodi interni al cluster è con con alberi di istradamento **individuali** per ogni nodo. Il **routing esterno** tra i cluster è con un albero di instradamento **unico** per tutti cluster (spanning tree ottimo). Anche questa tecnica è adoperata dai dispositivi **Zigbee** (IEEE 802.15.4).

![cluster-flat](cluster-flat.png)

Un'altra funzione **potenzialmente energivora** è il **polling dei sensori** ovvvero la loro lettura periodica con annessa **trasmissione in remoto** dei dati. In questo caso se il **primo nodo** di smistamento della catena è parecchio distante (è il caso di tenologie outdoor come LoraWan o Sigfox) o sebbene indoor si adopera una **trasmissione** in una **tecnologia  energivora** (come è in modalità standard il WiFi) allora sono possibili almeno due **soluzioni operative** per abbattere i consumi:
- **allungare l'intervallo di polling** facendolo passare dall'ordine dei secondi a quello dei minuti o delle ore.
- **memorizzare le misure in locale** sul dispositivo e, ad intervalli regolari adeguatamente lunghi, inviare dei **dati aggregati nel tempo** come **medie e varianze** o statistiche in genere.

Quasi **tutte le tecnologie wireless** poi permettono di mettere, nell'intervallo di tempo tra una misura e l'altra, il dispositivo in modalità di **sleep** o **standby** profondo che **rallenta** di molto il clock della CPU permettendo un grande **risparmo di energia**.

Il **consumo di energia** è generalmente proporzionale alla **velocità di trasmissione** e alla lunghezza dei messaggi. Tecnologie **general purpose** che sono **ottimizzate** per la trasmissione di **file o stream** più che di **brevi messaggi** in genere sono più complesse e esibiscono consumi **più elevati**.

Il **grafico sotto** mostra **il posizionamento** delle varie tecnologie wireless che lavorano in **banda ISM** in base alla **bitrate** e alla **distanza** e potrebbe essere usato come **tassonomia di riferimento** (classificazione) delle varie teconologie proprio in base a **queste proprieta**. Al netto di qualche sovrapposizione, sono abbastanza distinguibili **tre zone**: 
- **corto raggio**. A distanze previssime ci stanno i sensori **RFID**, **NFC** e anche **BLE** utili per le **BAN** (Body Area Network), il tag degli oggetti, il  **tracciamento** di persone ed oggetti e il riconoscimento. **Bluetooth** si differenzia per il tipo di servizio, cioè la **connessione  punto** punto diretta tra dispositivi, che per le distanze (da 10m a 100m) tipiche di una **PAN** (Perosnal Area Network). **BLE**, **6LowPan** e **Zigbee** sono le tecnologie principali con cui entro i 100m si realizzano **WSN a basso consumo**. **BLE** in particolare, fornisce anche servizi di  **tracciamento** e **posizionamento indoor**. Possono essere tutti usati sia indoor che outdoor ma indoor forniscono servizi che altre tecnologie hanno difficolta a fornire con uguale semplicità. 
- **medio raggio**. E' il dominio del WiFi e della telefonia cellulare intesa come **modem dati a pacchetto**. Sono tipici in quest'ambito servizi **M2M** (Machine to Machine). Il 5G ha incorporato le tecnologie preesitenti **Cat-NB Narrowband IoT (NB-IoT)** and **Class M LTE (LTE-M)** che offrono servizi a **basso bit rate**, anche se a **consumi** significativamente **superiori** rispetto a LoraWan, però le **basse latenze** li rendono adatti ad **applicazioni mission critical** quali **robotica**, **guida autonoma** e **telecontrollo remoto**.  Una novità recente è l'introduzione delle **reti LTE private** (RAN private), cioè l'installazione di infrastrutture di telefonia cellulare private che possono essere **licensed** o **ulicensed**, cioè utilizzare o meno frequenze licenziate in concessione all'operatore o lavorare nella **banda comune NR-U** libera e simile concettualmente alla **banda ISM**.  Le **RAN private** possono essere anche **dipendent o indipendent** a seconda che siano gestite direttamente o meno dall'operatore telefonico. Il **WiFi** può realizzare reti di **notevoli dimensioni** (città intere o grandi aree rurali) ma ogni dispositivo ha range di copertura limitato (tipicamente 25m). E' possibile realizzare anche con il wifi una copertura di **tipo cellulare** che supporti il **roaming** cioè la **mobilità dei dispositivi**. Tutte queste tecnologie richiedono una **infrastruttura** (cablata o wireless) **complessa** e al momento dai **consumi elevati** soprattutto per dispositivi come i sensori. Se l'interesse preminente è il **basso consumo** e la **semplicità della rete** si rientra nella categoria delle soluzioni **LPWA** che, al momento, taglia fuori il wifi in favore di **LoraWan** e **NB-IoT**, per citare le tecnologie più comosciute ed adoperate. 
- **lungo raggio**. **LPWAN** (Low Power Wide Area Network) garantiscono una trasmissione a **lungo raggio** (da qualche chilometro a qualche decina di chilometri), un **consumo energetico contenuto** per la trasmissione/ricezione dati e un **basso costo** di realizzazione dei dispositivi al prezzo di una **ridotta velocità** di trasferimento dati (poche decine di kilobit al secondo). Forniscono anche **servizi outdoor** di **posizionamento** e **tracciamento** di oggetti. Sono adatti sia per applicazioni indoor che autdoor ma **outdoor** forniscono servizi che altre tecnologie hanno difficolta a fornire con uguale semplicità. L'infrastruttura è leggera dato che anche migliaia di dispositivi possono essere serviti da un'unica BS (Stazione Base) chiamata Gateway. Il resto dell'infrastruttura può essere realizzato semplicemente in house e utilizzato ancora più semplicemente se realizzato in cloud. Le principali **tecnologie LPWA** attualmente disponibili sono **LoraWan**, **Sigfox** e, recentemente, **NB-IoT**.  **VSAT** è una tecnologia satellitare piuttosto datata ma a basso costo e disponibile ovunque nel globo, è utilizzata in regioni rurali, spesso con alimentazione solare.

<img src="Data-rate-vs-range-in-communication-networks-Source-27.png" alt="alt text" width="700">

Versione più **dettagliata**:

## **Interfaccia radio**

Il **mezzo trasmissivo radio** è partizionabile in **frequenza**, **tempo**, **spazio** e **potenza**. Delle grandezze precedenti quella in assoluto **più limitata** è la **frequenza** essendo **proprietà dello Stato** e ceduta in **concessione** sotto ben precise condizioni (**licenze**). Questo è il motivo per cui, nel realizzare qualsiasi tipo di comunicazione radio, per prima cosa, bisogna cominciare col **riservare** una porzione del campo delle **frequenze radio** (spettro) **allocando** degli intervalli di frequenze detti **canali**. 

I **canali** sono allocabili all'interno di intervalli di frequenze dette **bande**. Le bande si dividono in **licenziate** cioè quelle per le quali bisogna acquistare dallo stato la **concessione** per trasmmetterci e in quelle **non licenziate** per le quali, sotto ben precise **condizioni tecniche** (modulazione, potenza, duty cicle, ecc.), o **vincoli burocratici** (possesso di autorizzazioni o patenti presso autorità regolatorie) la trasmissione è **libera**. Una di queste ultime è la banda **ISM** (Industrial Scientific Medical). L'**ampiezza del canale** dipende dalla **tecnologia adoperata**. Nelle bande ISM è inoltre possibile trasmettere **senza particolari restrizioni** burocratiche (acquisto di licenze o patentini), in particolare, **senza avvisare** nessuna autorità ma semplicemente utilizzando **dispositivi certificati** cioè che rispondono ai regolamenti tecnici in vigore **nella nazione** in cui sono adoperati. La libertà di accesso alla banda ISM è il motivo per cui essa è **mediamente affollata** e le sue trasmissioni **soggette ad interferenze** più o meno intense. Le **interferenze** sono **gestite** utilizzando **modulazioni robuste alle interferenze** e adoperando **segnali** a bassa densità spettrale, basso duty cicle e, soprattutto, **potenza limitata**. 

<img src="wifi-bands.png" alt="alt text" width="1000">

Recentemente alla banda ISM si è aggiunta un'altra **banda non licenziata**, la **NR-U** centrata all'incirca intorno ai **6GHz** che è stata assegnata al **WIFI 6** (IEE 802.11ax) e alle **RAN (Radio Access Network)** della telefonia cellulare che possono essere o **gestite dall'aperatore** di telefonia o di **proprietà dell'utente** che così realizza una **rete 5G privata**. La **potenza** di un AP in banda NR-U non può eccedere i 30dBm per cui una **BS 5G** crea un'**interferenza** analoga a quella di un **AP wifi**. A parità di area, **densità** di distribuzione degli AP 5G dovrebbe essere circa la metà del WIFi ma con **costi** per dispositivo molto maggiori.

<img src="duplexer.png" alt="alt text" width="700">

Il **mezzo radio** possiede la **proprietà** di essere intrinsecamente **multipunto (mezzo broadcast)** cioè, se un interlocutore trasmette **tutti gli altri ascoltano**. Inoltre, essendo in pratica un BUS, è soggetto al fenomeno delle **collisioni** (in quest'ambito sono fisicamente delle interferenze distruttive) che si possono risolvere come sempre, cioè facendo in modo che in **certo canale**, in un **dato posto** e in un **dato tempo** si parli **uno alla volta**. Il **canale radio** cioè, è intrinsecamente **half duplex**. 

Una trasmissione radio **full duplex** può essere realizzata o utilizzando **due canali diversi** nelle **due direzioni** di trasmissione (tecnica FDM) o **partizionando** ulteriormente il canale nelle grandezze residue **ancora non partizionate** in **due sottocanali** (TDM o CDM). Normalmente si usa allo scopo il **tempo** con tecniche TDM, cioè **due interlocutori** diversi parlano nello stesso canale in **istanti diversi**. Ma il meccanismo dell'**alternanza nel tempo** può anche essere esteso ad un **numero arbitrario** di interlocutori che parlano **a turno** secondo un criterio di dialogo assimilabile a quello della **"tavola rotonda"**. Ciascun interlocutore **è numerato** ed è **abbinato** ad **uno o più** numeri di **altri** interlocutori. Un partecipante con **un certo numero**, per realizzare una comunicazione **punto punto** o **multipunto**, dovrebbe **ascoltare** soltanto i discorsi pronunciati **da un solo** interlocutore o **da tutti** gli interlocutori aventi però uno dei numeri a cui egli **è abbinato**.

<img src="6.25kHz-transmission-mask.jpg" alt="alt text" width="1000">

<img src="Spectrum-efficiency-1.jpg" alt="alt text" width="1000">

La partizione **TDM** nel tempo del canale può essere statica o dinamica. Quella **statica** utilizza la tecnica di multiplazione **TDMA**. Quella **dinamica** utilizza un **protocollo di arbitraggio** che, con un meccanismo di **ack**, permette di realizzare **l'alternanza nel tempo** delle comunicazioni. Si basa essenzialmente sui protocolli **ALOHA** o **CSMA/CA**. 

La multiplazione TDM di **parecchie sorgenti** richiede però che **tutte** siano **sincronizzate** con precisione presso un **nodo router**, cioè non possono comunicare **direttamente tra loro**, in maniera peer to peer, ma solo in presenza di un **nodo master** da cui devono essere **ragiungibili**.

Rappresentazione grafica del protocollo **ALOHA**. I **riquadri chiari** indicano le trame ricevute correttamente. I **riquadri ombreggiati** indicano le trame che hanno **colliso** e che verranno scartate perchè arrivate **corrotte**. Il **servizio radio** potrebbe prevedere la funzione di **conferma**, cioè che **tutti i messaggi trasmessi** debbano essere **confermati** presso il **nodo** che li **ha trasmessi** mediante la **ricezione** di un **messaggio di ack** inviato dal **ricevente** al loro arrivo. Le **trame corrotte** non saranno confermate dal ricevente e, allo **scadere di un timeout**, verranno **ritrasmesse**.
 
<img src="Pure_ALOHA1.svg.png" alt="alt text" width="600">

Spesso il **TDM** non partiziona direttamente il canale FDM ma tra esso e il canale FDM si **interpone** una ulteriore divisione nel **dominio della potenza** o nel **dominio della frequenza**. 

**La prima** è detta **multiplazione a divisione di codice** o **CDMA** che è associata ad un certo **cluster di utenti** afferenti ad un **certo dispositivo** (ad esempio un AP WiFi). **Dispositivi diversi** hanno **codici** (detti ortogonali) **diversi**. Ad es, il **BSSID** di un AP (Access Point wifi) è associato ad un **codice ortogonale** diverso da quello degli altri AP in modo che una sua comunicazione sia **distinguibile** da quelle degli altri AP, pur interferndo completamente con quelle.  E' la strategia generalmente adottata nei sistemi LPWA, a **bassa potenza**, e **bassa bitrate** e **basso costo**.

**Riassumendo la divisione nel dominio della potenza (spettro espanso)**, si possono **annidare** le multiplazioni una dentro l'altra differenziando per **caratteristiche fisiche** diverse le **singole comunicazioni** e associandole a **gruppi di sorgenti diverse**. In questo caso:
- la **banda** è **divisa** tra più **gruppi** di dispositivi (ad es. AP WiFi), ogni gruppo sul **suo canale FDM**, tramite una multiplazione **FDMA**.
- un **canale FDM**, assegnato ad un gruppo di AP o BS, è **diviso** tra **più AP**, ogni AP col **suo BSSID (canale CDM)**, tramite multiplazione **CDMA**.
- un **canale CDM**, assegnato ad un gruppo di sorgenti (quelle **attestate** su **un AP**), è **diviso** tra **più sorgenti**, ogni sorgente col suo **slot temporale (canale TDM)**, tramite multiplazione **TDM** (dinamica o statica).  

**La seconda** è detta **multiplazione a divisione di frequenze ortogonali** o **OFDM**. **Più canali** sono divisi in **sottoportanti diverse**. Ogni **sottoportante** può essere ulteriormente **partizionata nel tempo** tramite un **TDM** schedulato centralmente (nell'AP o nella BS) o tramite un TDM a contesa che la assegna a **più sorgenti**. In pratica, per ottenere **l'allocazione** del canale ad **una sorgente**, questo è **partizionato** **nel tempo** (con OFDMA anche **nella frequenza**). E' la strategia generalmente adottata nei sistemi **WiFi** e sistemi di telefonia mobile **4G** e **5G**, caratterizzati da **alte potenze**, **elevate bitrate** e **alto costo**.

In questo contesto l'**arbitraggio CSMA/CA** ha la particolarità di gestire non solo la **contesa** per l'**accesso al mezzo** tra dispositivi di **uno stesso AP**, ma anche quella di tutti i dispositivi, quelli appartennti ad un'**altro AP**, ma anche quelli appartenenti ad **altri sistemi** che in quella zona accedono il canale a vario titolo. Se l'**energia** delle inerferenze dovute ad altri sistemi **è elevata**, il canale è considerato **occupato** per cui viene **dilazionato** il tentativo di accedervi ad un **momento migliore**.
 
Normalmente le **multiplazioni** sono gestite a **livello fisico** in HW utilizzando delle **schede radio dedicate** su cui si impostano i **registri** per la **configurazione** e da cui provengono le **notifiche** dei vari **eventi** legati alla **trasmissione** o alla **ricezione**.

A seconda della tecnologia adoperata si possono realizare comunicazioni **punto-punto** o **multipunto**, al limite anche **broadcast**. 

L'**isolamento punto punto** nei mezzi a BUS con **arbitraggio a contesa** (ALOHA e CSMA/CA) è in genere **logico** e si ottiene, a **livello di scheda** facendo in modo che una stazione radio che non riconosca il **proprio indirizzo** nella destinazione dei messaggi **ignori** le comunicazioni non proprie. Diventa più effettivo con la **cifratura** delle informazioni.

In ogni caso, con il **TDMA** o con il **CSMA** è possibile pure **condividere** lo stesso canale per **più** comunicazioni **punto-punto**. Il mezzo radio è quindi anche assimilabile ad un **BUS condiviso** o mediante un **protocollo di arbitraggio** (ALOHA, CSMA/CA) o mediante una **tecnica di multiplazione statica** (FDMA, TDMA, CDMA, SDMA).

<img src="multiple-access-protocol.png" alt="alt text" width="600">

La **multiplazione SDM** (divisione di spazio) del mezzo radio è sempre in qualche misura possibile **controllando accuratamente le potenze** di trasmissione dato che la **propagazione** delle onde radio, anche **nel vuoto**, è soggetta ad un **intenso fenomeno di attenuazione** che cresce con il **quadrato della frequenza** e con il **quadrato della distanza**. Un esempio di SDM radio è la **divisione in celle** dell'area di **copertura del servizio** radio avente la proprietà che **celle vicine** usano frequenze molto diverse mentre **celle distanti** riutilizzano la stessa frequenza.

<img src="cellulare.png" alt="alt text" width="300">

I dispositivi mobili che si spostano da una cella all'altra vengono presi in carico automaticamente e in modo trasparente dalla nuova cella senza perdere il collegamento (meccanismo del roaming).


## **Servizi di accesso radio per WSN**

Abbiamo visto che l'**interfaccia radio** si accede **allocando per primo** un **unico canale radio** di una multiplazione **FDM**. L'allocazione può essere sia **statica** (eseguita dal sistemista) che dinamica cioè variabile nel tempo ed **automatica**.

**Successivamente** il canale FDM **potrebbe** essere ulteriormente **partizionato in gruppi di utenti** tutti collegati ad uno **stesso dispositivo**. I dispositivi (ad es. gli AP wifi di uno stesso palazzo) interferiscono tra loro ma le comunicazioni sono rese **distinguibili e private** mediante multiplazione a spettro espanso **CDM** (Code Division Multiplexing) che associa un SSID diverso ad ogni dispositivo. Ciò è dovuto al fatto che usiamo uno dei canali della banda ISM su cui non esistono coordinamento e controllo alcuno. Anche se questa tecnica è adesso usata solo in dispositivi LPWA mentre WiFi 6 e telefonia mobile 5G preferiscono ad essa la tecnica OFDM.

### **DSSS**

In realtà DSSS più che una tecnica di  **multiplazione** per l' **accesso al canale radio** è una vera e propria tecnica di **modulazione numerica** complementare alle tradizionali ASK, PSK, FSK, ecc.).

Nel **CDM** la sorgente viene:
- **traslata**: cioè portata alla frequenza centrale della banda di trasmissione
- **espansa**: cioè trasformata da segnale a banda stretta a segnale che occupa tutta la banda di lavoro detto segnale a banda espansa.  

L’**espansione in frequenza** si ottiene trasformando artificialmente un segnale **lentamente variabile** nel tempo in uno **rapidamente variabile** nel tempo.
Ad ogni sorgente è associato un **codice di espansione** da usare in trasmissione. Lo stesso codice, **usato in ricezione**, riporta a **banda stretta** solo lei lasciando espanse tutte le altre sorgenti.

<img src="spread1.png" alt="alt text" width="1000">

La **proprietà** che viene **divisa** fisicamente è la **potenza** di trasmissione. La proprietà che viene suddivisa logicamente è un insieme di **codici ortogonali**
**In TX** la sorgente viene espansa: cioè trasformata da segnale a banda stretta a segnale che occupa tutta la banda di lavoro, detto segnale a spettro espanso.  

L’**espansione** in frequenza di un segnale:

- mantiene **immutata l’energia** complessiva del segnale originale, cioè l’area sottesa al segnale nel diagramma potenza – frequenza rimane la stessa (si allarga la base del segnale ma, nel contempo, si diminuisce l’altezza).
- si **ottiene nel tempo** moltiplicando il segnale di partenza per un altro segnale di frequenza molto maggiore detto **chirp (o spreading code)**. L’**energia** viene così **diffusa** su **tutte** le frequenze della banda ma con una **ampiezza** molto bassa. La **moltiplicazione** trasforma un segnale lentamente variabile in uno rapidamente variabile **sostituendo** il **bit originale** con una **sequenza di bit** veloci detti **chirp**.

<img src="spread2.png" alt="alt text" width="1000">

Nel CDM le **sorgenti** occupano e **condividono** tutta la **banda** di frequenza nello **stesso tempo** e in **tutto lo spazio** sovrapponendo l’energia di una con quella dell’altra (interferenza completa). Il **ricevitore** riceve i segnali di più sorgenti sotto **forma espansa** e sovrapposti tra loro (segnali interferenti) che sono, in pratica, indistinguibili.

**In ricezione** la moltiplicazione nel tempo di un segnale ricevuto per lo stesso chirp (o spreading code) con cui era stato espanso in trasmissione, riporta il segnale ricevuto a banda stretta, permettendone la decodifica dei bit. La moltiplicazione con quel codice avviene anche con gli altri segnali ricevuti ma, essendo questo diverso dal loro spreading code, li lascia a spettro espanso. 

E' una tecnica di accesso usata dal protocollo **Zigbee** e **6LowPan** (IEEE 802.15.4).

### **FHSS**

Anche FHSS potrebbe essere vista come una tecnica di **modulazione numerica**.

Alcune **tecnologie radio** realizzano un **accesso multiplo** al canale radio, **apparentemente** allocando **molti canali** nello **stesso istante**. In realtà la tecnica  alloca un canale alla volta ma **saltando** da uno all'altro con una **velocità** che **alla sorgente** appare **istantanea**. Questo fenomeno ha l'effetto di sparpagliare (**spreading**) l'energia di un canale su **tutta la banda** disponibile. Si tratta di una **variante di CDM** ottenuta per altra via. Infatti, la **sequenza di saltellamento** è diversa per ogni comunicazione ed è associata ad un **SSID**. Le **sequenze** sono **ortogonali** e quindi permettono la ricostruzione in ricezione della **comunicazione originale**. Anche una eventuale **multiplazione TDM** effettuata dentro il canale FDM non si accorgerebbe del saltellamento e procederebbe come al solito, assegnando **diversi slot** temporali a **comunicazioni diverse**. Un messaggio potrebbe **partire** in un canale, passare ad altri 7 e completarsi solo nell' ottavo canale, ovvero l'ultimo visitato. La tecnica si chiama **Frequency hopping** o anche detta **FHSS**. La **distribuzione** della comunicazione su una **gamma ampia** di frequenze rende questo tipo di modulazione **meno sensibile** alle **interferenze**.

<img src="fhss.png" alt="alt text" width="600">

E' una tecnica di accesso usata dal protocollo **Bluetooth LBE**

### **TSCH**

Un una tecnologia radio realizza **accesso al canale singolo**, nel senso che l sorgente invia **per intero** un messaggio **in un canale** per un tempo stabilito da una **schedulazione TDM** costante e preordinata ma, alla **trama successiva** quello **slot** temporale sarà assegnato ad **un'altra frequenza** per tutta la sua durata e così accade per tutti gli altri slot di quella trama. In caso di trasmissione non riuscita a causa di interferenze esterne o dissolvenza multi-percorso, la sua ritrasmissione avviene su una frequenza diversa, spesso con una migliore probabilità di successo rispetto all'utilizzo della stessa frequenza di prima. Si noti che, identificando una comunicazione con i suoi dispositivi endpoint, allora accade che tutti questi dispositivi, per ogni trama, devono condividere lo stesso slot e lo stesso canale.  La tecnica si chiama **Time Slotted Channel Hopping** o anche detta **TSCH**. Gli hop sono **più lenti** del **segnale modulato** per cui non è considerabile come una tecnica di modulazione del segnale portante.

<img src="Time-Slotted-Channel-Hopping-TSCH-slot-channel-matrix-with-a-simple-network-topology.png" alt="alt text" width="600">

E' usata dal protocollo IEEE 802.15.4g/e, livello fisico degli stack **RIIoT**, **OpenWSN** e dalle ultime versioni di **Zigbee** e **6LowPan**.


### **OFDM**

E' una tecnologia in cui i bit di una comunicazione, tramite una conversione seriale-parallelo, vengono inviati contemporaneamente su **più porzioni** di un canale di grande ampiezza (20 Mhz per il WiFi) dette **sottoportanti**. Le sottoportanti non solo sono vicine ma si **sovrappongono** in parte di uno spiazzamento stabilito. Lo **spiazzamento calibrato** opportunamente fornisce la proprietà di **ortogonalità** che garantisce alle sottoportanti di **non interferirsi** nonostante le sovrapposizioni reciproche. In pratica una **comunicazione veloce** viene suddivisa in più **comunicazioni lente** che hanno il pregio di poter essere gestite in maniera **meno critica** rispetto ai problemi associati ad un'unica veloce. 

La **distribuzione uniforme** delle sottoportanti lungo tutta la banda fa si che, per effetto di una **interferenza distruttiva** o di un **fading** improvviso (attenuazione) in corrispondenza di una certa sottoportante, si perderanno, o nel caso del fading si amplificheranno, le informazioni trasmesse su quella ma non le rimanenti trasmesse sulle altre sotoportanti. Questo perchè, in ricezione, le sottoportanti vengono demodulate separatamente e solo dopo i bit ricevuti sono ricomposti in un unico flusso. E' il criterio dei **compartimenti stagni** di una nave (le **sottoportanti**) applicato al **canale** di una sorgente (l'intera **nave**). 

All'**interno** del canale OFDM viena fatta una multiplazione **TDM** a contesa sia per renderlo **full duplex** sia per consentire l'**accesso multiplo** al canale da parte di più sorgenti, per cui, al variare del tempo, sullo  **stesso canale** verranno allocate **sorgenti diverse**. In ogni caso la trasmissione di **una sorgente** avviene sempre su **tutte** le sottoportanti (**no FDM**).

Le **sorgenti lente** impiegheranno un certo **tempo** per recuperare un numero sufficiente di bit per alimentare adeguatamente tutte le sottoportanti, ne consegue che la loro trasmissione sarò caratterizzata da **brevi impulsi** distanziati da **lunghi ritardi** e trasmessi con grande spreco di **potenza**.

Oppure potrebbero esserci molte **trasmissioni brevi** che competono per il canale mediante il TDM a contesa **CSMA/CA**. Questa situazione genera uno scambio di **messaggi di controllo** aventi **dimensione** analoga a quella dei **messaggi dati**. Vuol dire che il canale potrebbe essere **più occupato** dai messaggi necessari per portare avanti il protocollo CSMA /CA che dai messaggi dati (**overhead eccessivo del CSMA/CA**).

<img src="xofdm2.png" alt="alt text" width="1000">

E' adoperato dal **WiFi 5** e dalla telefonia mobile **4G**. Ha la proprità di trasmettere dati ad **alta velocità** in ambienti con **forti interferenze**. 


### **OFDMA**

Realizza una **parallelizzazione** dell'accesso al canale radio da parte di **più sorgenti** tramite **FDM**. Infatti, nelll'OFDM, Benchè venga effettuato un **TDM**, può accadere che le **bitrate** delle sorgenti siano **molto diverse**, e generino **brevi messaggi** che occupano **tutte** le sottoportanti **uno slot** temporale alla volta, alla massima bitrate del canale. L'**analogia del canale** è quella dei **TIR** che in **giorni diversi** portano poco carico quando il **carico complessivo** dei tre giorni avrebbe potuto benisssimo essere accomodato su **un unico TIR**, direttamente al primo giorno, **riducendo** i **tempi di attesa**. 

**Servono** per far ciò: 
- la possibilità di inviare, nello stesso slot temporale, messaggi di **sorgenti diverse** su **sottoportanti diverse** dello stesso canale (multiplazione FDM)
- una **schedulazione efficace** che sappia **riordinare** i messaggi sulle sottoportanti (slot FDM) cercando di rispettare le richieste in termini di **latenza** che erano state **prenotate** per ogni sorgente in fase di **setup** della connessione. 

<img src="ofdma.jpg" alt="alt text" width="1000">

La **principale differenza** tra un sistema **OFDM** e un sistema **OFDMA** (Orthogonal Frequency Division Multiple Access) sta nel fatto che nell'OFDM l'utente è allocato nel dominio del **tempo soltanto**. Durante l'utilizzo di un **sistema OFDMA**, l'utente viene allocato sia **in base al tempo** che **in base alla frequenza**. Il sistema in un **certo istante** è in grado di trasmettere o ricevere comunicazioni di **più dispositivi** in **parallelo** (su sottoportanti diverse) diminuendo i **tempi di attesa**. 

<img src="scheduler.png" alt="alt text" width="700">

OFDMA divide lo spettro in **unità di risorse (RU)** tempo-frequenza. Un'**entità di coordinamento centrale** (l'AP in 802.11ax) assegna le RU per la ricezione o la trasmissione a non più di **una stazione** alla volta. La **pianificazione centralizzata** delle RU permette, inviando più messaggi brevi contemporaneamente sul mezzo radio, di evitare un **sovraccarico** (overhead) di contesa CSMA del canale, il che aumenta l'efficienza in contesti **affollati** di brevi messaggi come le **reti IoT**. OFDMA in, sostanza **scala** meglio ler risorse adeguandole a differenti **mix di traffico**, nel contempo, **riducendo l'overhead** delle comunicazioni. Per l'utente un **minore overhead** si traduce in un **ritardo** più basso.

Si potrebbe anche pianificare la **QoS** in base alla **frequenza**. Ad esempio, sarebbe possibile sfruttare il fatto che l'utente  potrebbe avere una **migliore qualità** del collegamento radio su specifiche sottoportanti della banda disponibile, evitando di trasmettere sulle altre che in quel momento sono molto disturbate. 

<img src="ofdm_ofdma.png" alt="alt text" width="700">

E' adoperato dal **downlink** di **WiFi 6**, telefonia mobile **5G** e **NB IoT**.

### **SC-FDMA**

**SC-FDMA** ha attirato grande attenzione come **alternativa** interessante a OFDMA, specialmente nelle **comunicazioni uplink** poichè il rapporto di potenza di picco su potenza media (**PAPR**) basso avvantaggia notevolmente **il terminale mobile** in termini di efficienza della potenza di trasmissione e costi ridotti dell'amplificatore di potenza. È stato adottato come schema di accesso multiplo del 5G e del NB-IoT.

Il segnale iniziale viene processato da **due blocchi**. Il **secondo** è un normale **OFDMA** mentre **il primo** si chiama **DFT** ed è una **elaborazione numerica** il cui effetto finale è di **convertire** i simboli dei dati dal dominio del tempo trasportandoli nel dominio della frequenza. Il **risultato** è che essi sono **disposti in frequenza** allo stesso modo in cui erano prima **disposti nel tempo**. Una volta nel dominio della frequenza, vengono trasportati nella posizione desiderata nella larghezza di banda complessiva del canale. 

Nella **figura** sottostante, **4 simboli**, con **4 colori diversi**, erano **inizialmente** trasmessi **in parallelo** su **4 sottoportanti** diverse ma con **ampiezze nel tempo** molto **variabil**i. Dopo il **blocco DFT**, hanno **ampiezza costante** nel tempo ma variabile nella frequenza. Inoltre la DFT, di fatto, ha effettuatto una **conversione** dei simboli **da parallelo a seriale** perchè vengono **trasmessi nel tempo in serie**, uno dopo l'altro.

<img src="sc-fdma.png" alt="alt text" width="800">

E'adoperato dall'**uplink** di **WiFi 6**, della telefonia mobile **5G** e **NB IoT**.


### **Classificazione dei servizi radio**

Qualunque sia la modalità di accesso, alla fine, su **questo canale risultante**, privato ed eventualmente associato ad un certo **SSID**, a seconda del **servizio richiesto**, **possono parlare**:
- **due interlocutori**.
- **molti interlocutori**

In entrambi i casi sono possibili **due tipi** di **servizio** di **accesso radio**:
- **servizio sincrono**. Viene realizzato con un **TDM statico** ed è adatto **operazioni periodiche** come il **polling** di un grappolo di sensori.
- **servizio asincrono**. E' adatto a **operazioni una tantum**, cioè di cui non si riesce a sapere **in anticipo** l'occorenza (accadere nel tempo), ad esempio il comando di accensione di una lampadina.  In questo caso riservare uno slot temporale ad una sorgente che per la maggior parte del tempo non lo utilizzerebbe è certamente **uno spreco**. Si preferisce utilizzare la tecnica di accesso **TDM a contesa** con accesso al canale arbitrato dai protocolli **ALOHA** O **CSMA/CA**.

Ulteriori **differenziazioni del servizio** distinguono tra:
- la presenza o meno di una funzione di **conferma** dei messaggi trasmessi (protocollo confermato).
- la presenza o meno di **classi di prestazione**, magari abbinate ad uno o più dei servizi precedenti.
    
Molti sistemi (wifi, zigbee, bluetooth BLE, LoRaWan, Sigfox) permettono di impostare **contemporaneamente**, sulla **stessa interfaccia** radio, un **servizio sincrono** mediante **TDMA** per le sorgenti che eseguono il **polling** di sensori e un **servizio asincrono** con **ALOHA** o **CSMA/CA** per le sorgenti che devono inoltrare il **comando** di un pulsante di accensione di un attuatore. Ciò è ottenuto **attivando** sul canale la funzionalità **beacon** con le cosidette **superframe**.

Un **beacon** contiene informazioni che **identificano** la rete e i suoi servizi nonchè una sequenza di **bit di sincronizzazione**. Vengono trasmessi **periodicamente**, servono ad **annunciare** la presenza di una LAN wireless e a **sincronizzare** i membri sui suoi servizi (sincroni e asincroni).

<img src="IEEE-802154-MAC-superframe-structure-in-beacon-enabled-mode-active.png" alt="alt text" width="600">

In una **supertrama** due **beacon** fungono da **limiti** (iniziale e finale) e forniscono la **sincronizzazione** con altri dispositivi e informazioni di configurazione. Un superframe è costituito da un certo numero di **slot** temporali divisi in **due gruppi**. Il **primo** è riservato per le sorgenti **asincrone** che accedono in **modalità a contesa** con i protocolli di arbitraggio ALOHA o CSMA, **il secondo** è dedicato alle sorgenti **sincrone** che, in quella porzione della trama, hanno uno **slot** esclusivamente **dedicato** a loro per tutto il tempo in cui esse **risultano attive**.

##  **Stack wireless specifici per IOT**

<img src="lpwan-taxonomy.png" alt="alt text" width="700">

Ci si potrebbe chiedere se è proprio necessario sapere i **dettagli sui protocolli** adoperati da questo o quel dispositivo. La risposta è legata al **livello di astrazione** con cui la nostra applicazione **vede** la **risorsa rete**. 

Con alcuni protocolli essa ha della rete e dei suoi protocolli una visione che si può ridurre all'**elenco di servizi** di livello applicativo che lo **stack espone**, per cui, quando l'applicazione utilizza un servizio, sullo stack viene scelto, strato per strato, il **protocollo più adatto** per realizzarlo. Addirittura in Blutooth BLE e in Zigbee i **servizi applicativi**, cioè la definizione dei cai d'uso come accendere una lampadina o aprire una tapparella, sono dettagliatamente **catalogati** e **definiti** nello stack.

Normalmente solo la **parte fisica** dello stack è implementata in **HW** mentre tutto il resto dello stack è realizzato in **SW**. Molte volte, per progettare una rete industriale di sensori, si mantiene **solo il chip** con il suo livello fisico, mentre per i **livelli superiori** si preferisce utilizare uno stack di protocollo alternativo, **diverso da quello originale**, realizzando, in pratica, una vera e propria **ibridazione**. In questo caso si possono ancora utilizare stack standardizzati da organizzazioni trasnazionali come IETF o IEEE ma questi possono essere molto articolati per cui, all'interno dello stack, si sceglie la classe di protocolli che si ritiene più adatta alle **proprie esigenze**. Una **proprietà**, spesso non garantita dalle grandi suite come Zigbee o LoraWan, potrebbe essere l'utilizzo del **livello di rete IPV6**, proprietà che potrebbe permettere la connessione diretta del dispositivo **in Internet** trasmettendo, senza modifiche, i suoi messaggi su reti LAN o WiFi. **Un'altra motivazione** potrebbe essere l'interesse a non adoperare la parte, di per se complessa, dello stack che gestisce la rete di molti dispositivi ma soltanto realizzare una **connessione punto punto**, cioè un **ponte radio numerico** tra due dispositivi e per far ciò un semplice supporto di rete IPV6 (senza funzioni di routing) potrebbe essere sufficiente. 

La **parte logica** dei due stack sotto (LoraWan a sinistra e Zigbee a destra) potrebbe essere sostituita con IPV6 inserendo il rpotocollo 6LowPAN. La **parte fisica** di entrambi verrebe mantenuta perchè realizzata **in HW**. Nel caso di LoraWan l'implementazione HW è **proprietaria** è costruita da una sola azienda (Semtech) e non è standardizzata industrialmente. Nel caso dello Zigbee la **parte fisica** è **standardizzata** con le sigla **802.15.4**.

<img src="ibridi.png" alt="alt text" width="1000">

Molti **framework per IoT** come **TinyOS**, **Contiki** e **RIOT** posseggono una **struttura modulare** che permette loro di includere, senza particolare sforzo, **stack personalizzati** di protocolli  in maniera tale da adattarli alle esigenze più particolari svicolandosi dagli stack protocollari completi certificati dall'industria (Zigbee, LoraWan). 

### **Stack wireless opensource**

Esistono anche **stack opensource** rilasciati da alleanze di sviluppatori pensati per sostituire il **livello applicativo** dei chip IoT attualmente più diffusi per orientarli verso ambiti di utilizzo più **specializzato**. Un esempio è **OpenThread** che si pone sopra i chip **802.15.4** realizzando con l'implementazione dello stack **Thread** una alternativa a Zigbee per l'**industria**. Sempre basandosi sul livello fisico di Zigbee (**802.15.4**), lo stack aperto per l'industria **ISA100.11a** (IEC62734) lo integra con il livello di rete **6LowPan** e, a **livello applicativo**, realizza **funzioni** specifiche, di **monitoraggio**, **controllo** (catena a perta e chiusa) e **sicurezza** (sensori di gas). Oppure **OpenWSN** che utilizza l'implementazione di **TSCH** IEEE 802.15.4g/e come protocollo di accesso al mezzo e protocolli standard IETF come protocolli di livello superiore (6LowPan, REPL e UDP), o anche **OSS-7** che implementa la pila di protocolli **DASH7**. **DASH7** ha la particolarità di avere un **livello fisico** praticamente **agnostico**, cioè supporta la maggior parte dei protocolli di accesso disponibili come, ad esempio, LoRa o IEEE 802.15.4g inoltra possiede la **particolarità insolita** di mapparsi perfettamente sullo stack ISO/OSI, cioè definisce un **protocollo per ogni livello OSI**.

<img src="openwsn-dash7stack.png" alt="alt text" width="1000">

Gli **stack personalizzati** o **opensource** sono utili per realizzare **soluzioni ad hoc** calate in un ben preciso **contesto aziendale** come, ad esempio, la gestione dell'automazione **HVAC** (riscaldamento, ventilazione e condizionamento dell'aria). In questo caso per una **stessa funzione**, magari in zone diverse, sono dislocata **tegnologie diverse**, cioè **reti WSN eterogenee** oppure **miste** wireless e cablate, che magari si ha l'interesse a rendere **interoperabili** ai **livelli superiori** del loro stack OSI. In **questo contesto** sono ancora dominanti, in ambito WSN, il **WiFi** e le tecnologie cellulari (**NB-IoT**) indoor. In ambiti specializzati cercano di farsi strada, tra gli stack e i protocolli non citati, Insteon, Wavenis, Webree, Z-wave, ANT+ e CSRMesh.

Uno **stack commerciale**, essendo completo fino al **livello di applicazione**, ha il vantaggio di garantire una grande **interoperabilità** con tutti i dispositivi in commercio a prescindere dalla loro marca, l'importante è che sia **standardizzato** e tutti i **vendor** derivino i loro dispositivi da esso. Si consideri, ad esempio, una **rete mesh (magliata) Zigbee** (ma la situazione sarebbe analoga per il Blutooth BLE). Questa è composta da **router**, cioè nodi di smistamento, che, dispiegati in numero adeguato, hanno il compito di allargarne le dimensioni della rete, ben oltre la linea di raggiungibilità del nodo sorgente dei comandi, ipoteticamente in ogni angolo della casa. In un contesto domestico (**home automation**), i **router** sono essi stessi **dispositivi commerciali** acquistati non curandosi affatto della loro funzione di rete (routing) ma semplicemente tenendo conto di una **funzione** di utilità domestica come ad esempio creare un punto di illuminazione (lampadina). Se i dispositivi **condividono** tutti lo **stesso stack WSN** il vantaggio per l'utente è chiaro. Con l'acquisto di un dispositivo con **alimentazione costante** (magari perchè **collegato alla rete**) l'utente compra anche il **dispositivo** che permette di **espandere la rete**, e ciò a beneficio non solo dei dispositivi del brand del prodotto acquistato ma anche a vantaggio di quelli **già installati** e di quelli che **si acquisteranno** in futuro. La **rete** diventa una infrastruttura completamente **trasparente** all'utente **basata** fisicamente e funzionalmente su tutti gli **oggetti smart** con cui egli popola la propria abitazione. Il **protocollo** di **routing dinamico** della rete garantisce l'**affidabilità** dell'infrastruttura ricalcolando, in maniera **trasparente all'utente**, il percorso di un messaggio di azionamento in caso di **guasto** di un nodo di transito.


### **Stack wireless orientati allo smart metering**

**Wireless M-Bus**

Detto anche **Wireless Meter-Bus** è lo **standard europeo (EN 13757-4)** che specifica la **comunicazione** tra **contatori** delle utenze e **gateway** per contatori intelligenti.
Sviluppato come standard per soddisfare la necessità di un sistema wireless di lettura dei contatori delle utenze in Europa, Wireless M-Bus viene adesso utilizzato come base per la nuova Infrastruttura di Misurazione Avanzata (**AMI**).
Il **DLMS** è uno standard di tipo aperto ratificato come IEC 61334-4-41. E' un protocollo di **livello applicativo** che definsce i requisiti minimi per un **contatore** affinché possa comunicare con altri dispositivi DLMS. Vengono, in particolare, stabiliti tutti i **tipi di misure** che il contatore deve essere in grado di rilevare e le relative **modalità di invio**, su richiesta di un client remoto, oltre ai requisiti in termini di **performance** che il dispositivo deve garantire. Il DLMS, inoltre, è uno standard adatto non solo alle sole reti elettriche, ma anche alle reti di altri **vettori energetici**, quali **gas**, **calore** ed **acqua**. 

**IEEE 802.15.4g**

Lo **IEEE 802.15** WPAN Task Group 4g (TG4g) ha proposto questo standard per estendere il corto raggio dello standard di base IEEE 802.15.4 (livello fisico Zigbee) per le reti di **smart metering** intelligenti o Smart Utility Network (SUN). Definisce tre livelli fisici intercambiabili, uno basato su frequency shift keying (FSK), accesso  multiplo a divisione di frequenza ortogonale (OFDMA), e sfasamento in quadratura offset keying (OQPSK). Fornisce copertura di diversi km. Le velocità dati supportate vanno da 40 kbps a 1 Mbps a seconda del livello fisico e regione in cui opera. Funziona secondo il principio di CSMA/CA e supporta stella, mesh, e altre topologie. E' il **livello fisico** dello stack commerciale **Zigbee NAN**, di quello **Wi SUN**, oltre che di quello opensource **DASH7**. Nati per il contesto dello **smart metering** ambiscono ad un utilizzo per l'IoT generico in ambito **LPWA** a corto raggio.

**NB-Fi**

WAVIoT ha sviluppato il protocollo **LPWAN aperto**, chiamato NB-Fi, che opera nella banda radio ISM senza licenza. Il protocollo NB-Fi consente comunicazioni wireless a **lunghissimo raggio** (fino a 10 km in aree urbane, fino a 30 km in aree rurali) a **basso consumo** energetico. Poiché le bande ISM sub-1 GHz sono affollate, i gateway sono progettati per funzionare con un algoritmo di **prevenzione delle interferenze**. Per ottenere una migliore efficienza nell'allocazione della banda e migliori prestazioni, impiega algoritmi  basati sulla **tecnologia SDR**, **reti neurali** e tecniche di **intelligenza artificiale**. Nato per il contesto dello **smart metering** si vorrebbe utilizzarlo in quello **più ampio** dell'**LPWA** in generale.

### **Stack wireless orientati all'energy harvesting**

La tecnologia **EnOcean** utilizza un **protocollo ottimizzato** per il risparmio energetico (**energy harvesting**) standardizzato come ISO/IEC 14543-3-10 Home Electronic Systems (HES) protocollo Wireless Short-Packet (WSP). 
I **pacchetti** di dati wireless EnOcean sono relativamente **piccoli**, essendo lunghi solo 14 byte e trasmessi a **125 kbit/s**. L'**energia RF** viene trasmessa solo per **gli 1** dei dati binari, riducendo la quantità di potenza richiesta. **Tre** pacchetti vengono inviati a **intervalli pseudo-casuali** riducendo la possibilità di collisioni di pacchetti RF.

L'**harvesting** consiste nel **recupero** dell'energia necessaria alla trasmissione o alla ricezione dall'**ambiente** mediante lo sfruttamento di movimenti meccanici e altri potenziali ambientali come la luce interna e le differenze di temperatura. Per trasformare le fluttuazioni di energia magnetica, solare e termica raccolte dall'ambiente in energia elettrica per alimentare il dispositivo, vengono utilizzati **convertitori di energia** molto compatti.

I **dispositivi**, come sensori e interruttori della luce, funzionano **senza batterie** e i **segnali radio** di questi sensori e interruttori possono essere trasmessi in modalità wireless fino a una **distanza** di 300 metri all'**aperto** e fino a 30 metri all'**interno** degli edifici. 

<img src="mbus-enoean.jpg" alt="alt text" width="1000">

##  **Canali di comunicazione principali in una rete di sensori**

**Riassumendo**, sono necessari almeno due canali di comunicazione che, insieme, complessivamente, realizzano la **comunicazione tra sensori e gestore** delle informazioni:
- **tra sensori e gateway** verso la LAN realizzato dalle sottoreti dei sensori:
    - **A filo** con accesso:,
        - **singolo dedicato**: un filo o un canale per sensore in tecnologia SDM o TDM (multiplexer, UART, porta analogica, porta digitale)
        - **multiplo condiviso** cioè tramite mezzo broadcast (BUS) con **arbitraggio** di tipo **master slave** (Modubus, Dallas, I2C, SPI) o **peer to peer** (CanBUS, KNX, ecc) o misto (ProfiBUS). 
        - Spesso **bidirezionale** specie se in presenza di attuatori
        
    - **Senza filo** cioè wireless con accesso:
        - **singolo dedicato**: link punto-punto analogico digitalizzato con AX25 oppure digitale con un radio modem (Yarm ACME Systems, 6LoWPAN, LoRa) resi full duplex con l'uso di multiplazioni FDM o TDM.
        - **Multiplo e condiviso** (BUS) di tipo half duplex reso bidirezionale (full duplex) tramite tecniche asincrone CSMA/CA (Zigbee, wifi, LoRa) o sincrone TDMA (Zigbee, Bluetooth).
- **Tra gateway e gestore** delle informazioni realizzato dalla rete principale:
     - Tipicamente tramite **LAN ethernet** e architettura **Client/Server**
     - Interazioni di tipo PUSH o PULL
     - Paradigma Request/Response (HTTPS, COAP), Publish/Subscriber (MQTT) oppure canale persistente bidirezionale (BSD socket o WebSocket)
![radiolinks](radiolinks.jpg)

## **RETI CELLULARI PRIVATE**

Il 5G, per sua natura, ha una conformazione dei suoi servizi parecchio **scalabile** sia in termini di **banda allocabile** ai singoli dispositivi sia in termini di **ritardi** per andare incontro alle applicazioni **Real Time**. Inoltre 5G ha **inglobato** nelle sue specifiche anche le teclologie del 4G **NB-IoT** che offrono **servizi a bassisimo bit rat**e utili per realizzare **WSN di grandi dimensioni**, in concorrenza diretta con tecnologie come **Sigfox** e **LoraWan**.

Il **5G fa uso** delle seguenti **frequenze di trasmissione**:
- **Banda bassa** (inferiore a 1 GHz): 694-790 MHz
- **Banda media** (inferiore a 6 GHz): 3,6-3,8 GHz
- **Banda elevata** (a onde millimetriche): 26,6-27,5 GHz  (5G NR Band n258 in banda K)
propriamente dette partono dal valore minimo di 30 GHz. La banda 26,6-27,5 GHz viene
tuttavia assimilata alle onde millimetriche in quanto sufficientemente vicina a questo
valore minimo.)

Ricordando che nel 5G, a **parità di potenza** trasmessa, se si **aumenta la frequenza** di
trasmissione si **riduce la distanza** a cui il segnale è utilizzabile, ma si **aumenta la banda**
disponibile, si può dire che:
- la **banda bassa** è utile per garantire una copertura maggiore del territorio anche **nelle
aree rurali**;
- la **banda media** è utile per offrire un **misto** di **copertura e di capacità** (quello che
tecnicamente si chiama throughput);
- la **banda elevata** è utile in quelle applicazioni che richiedano **alta capacità**, ma
localizzata in **aree molto limitate** per estensione (a queste alte frequenze il segnale si
indebolisce velocemente con il propagarsi dall’antenna), con elevata direttività del
segnale.

Per quanto riguarda la **banda bassa (sub GHz)** e la **banda media (sub 6 GHz)**, **non** si prevede un aumento significativo del numero di stazioni radio base, in quanto queste frequenze sono analoghe a quelle attualmente in uso per le generazioni precedenti. Per quanto riguarda la banda a onde millimetriche, l’elevata attenuazione subita dal  segnale, unita alla necessità di supportare capacità dell’ordine dei Gigabit per secondo, renderà necessaria la **capillare installazione** di stazioni radio base in **prossimità degli utenti**. Tuttavia, tali stazioni radio base non saranno dislocate su tutto il territorio, ma verranno impiegate soltanto laddove saranno **strettamente necessarie**, per esempio in **centri commerciali**, **stadi**, **stazioni** e **aeroporti**, ovvero in luoghi dove la **richiesta di capacità**, unita al **numero di dispositivi** connessi, sarà **elevata**.

<img src="NB-IoT-deployement.png" alt="alt text" width="600">

Il **canale NB-IoT** (la cui larghezza di banda è 180 kHz) può essere **allocato** con **tre modalità**:
- Independent deployment (**stand-alone mode**): banda frequenziale indipendente che non si sovrappone con quella dei canali LTE
- Guard-band deployment (**guard-band mode**): banda frequenziale localizzata nelle bande di guardia dei canali LTE
- In-band deployment (**in-band mode**): utilizza la banda frequenziale e le risorse allocate per un canale LTE

Si è calcolato che per poter soddisfare servizi ad **alta bit rate**, la qualità del segnale ricevuto deve essere estremamente buona e questo si traduce in una **distanza** fra trasmettitore e ricevitore molto corta (nell’ordine delle decine di metri). Al contrario, altri servizi del 5G utilizzano delle capacità estremamente ridotte (nell’ordine del kilobit per secondo), e possono essere soddisfatti anche se la distanza fra trasmettitore e ricevitore è elevata (nell’ordine dei chilometri).

<img src="5g-services.png" alt="alt text" width="1000">

Una novità dei prossimi anni sarà l'introduzione delle **reti 5G private**, cioè di **proprietà dell'utente**, che permetteranno l'**accesso alla rete LAN** con gli **stessi dispositivi** in uso per la **rete cellulare**. La **rete 5G privata** potrà essere **installata** e **gestita** da un **operatore telefonico**, oppure installata e gestita **da terzi**, oppure **installata** da **terzi** e **gestita** direttamente **dall'utente**, al limite, se ne ha le capacità tecniche, fa **tutto in house** l'utente. Le reti 5G private, per ragioni di costi, dovranno essere composte da **dispositivi di rete** necessariamente **multivendor**, cioè produttori diversi dovranno fabbricare dispositivi tra loro **interoperabili** (cosa che non è accaduta finora in ambito operatore di telefonia).

Il **processo di standardizzazione** è ancora in atto sotto il nome **Open RAN (O-RAN)** e include la standardizzazione di **protocolli** e di **interfacce** tra i vari dispositivi (potenzialmente di marca diversa). La **scalabilità dei servizi** e l'**esigenza di semplificazione** hanno suggerito di includere in questo processo anche la **virtualizzazione della rete** con l'introduzione dei **network slice**, piani di servizio e gestione paralleli, resi disponibili da **tecnologie di automazione** e virtualizzazione basate su **SDN** (Software Defined Network).

La **rete 5G privata** può essere implementata in **due modi**:
- Il **primo** è implementare una **rete 5G privata** fisicamente isolata (isola 5G) indipendente dalla rete 5G pubblica dell'operatore di telefonia mobile (come se si costruisse una LAN cablata o una WLAN Wi-Fi nell'azienda). In questo caso, la rete 5G privata può essere realizzata da **imprese specializzate** o **operatori mobili**.
- Il **secondo** è costruire reti 5G private **condividendo** le risorse di **rete 5G pubbliche** dell'operatore mobile. In questo caso, il **gestore costruirà** reti 5G private per le imprese.

La RAN gestisce lo spettro radio, assicurandosi che sia utilizzato in modo efficiente e soddisfi i requisiti di qualità del servizio di ogni utente. Corrisponde a un insieme distribuito di stazioni base.

La **RAN** è il collegamento finale tra la rete e il telefono. È il pezzo visibile e include le antenne che vediamo sulle torri, in cima agli edifici o negli stadi, più le stazioni base. Quando effettuiamo una chiamata o ci connettiamo a un server remoto, ad es. per guardare un video di YouTube, l'antenna trasmette e riceve segnali da e verso i nostri telefoni o altri dispositivi portatili. Il segnale viene quindi digitalizzato nella stazione base RAN e connesso alla rete.

Man mano che i produttori di apparecchiature miglioravano le capacità, l'industria si consolidava attorno a quelli con l'offerta più forte e spesso con funzionalità proprietarie. Ma oggi gli operatori vogliono un ecosistema di fornitori più diversificato e stanno ridefinendo i loro requisiti per l'architettura di rete, specialmente nella RAN.

In un **ambiente RAN aperto**, la RAN è **disaggregata** in **tre elementi** costitutivi principali:

- **l'Unità Radio (UR)**
- **l'Unità Distribuita (DU)**
- **l'Unità Centralizzata (CU)**

L'**RU** è il luogo in cui vengono **trasmessi**, **ricevuti**, **amplificati** e **digitalizzati** i segnali in radiofrequenza. L'RU si trova vicino o integrato nell'antenna. La DU e la CU sono le parti di calcolo della stazione base, che inviano il segnale radio digitalizzato nella rete. Il **DU** si trova fisicamente **presso** o **vicino** all'**RU** mentre il **CU** può essere posizionato **più vicino** al**Core**.

<img src="o-RAN1.png" alt="alt text" width="1000">

Il concetto chiave di **Open RAN** è "**aprire**" i **protocolli** e le **interfacce** tra questi vari elementi costitutivi (radio, hardware e software) nella RAN. L'**O-RAN ALLIANCE** ha definito **3** diverse **interfacce** all'**interno** della **RAN**, incluse quelle per:

- **Fronthaul** tra l'**unità radio** e l'**unità distribuita**
- **Midhaul** tra l'**Unità Distribuita** e l'**Unità Centralizzata**
- **Backhaul** che collega la **RAN** al **Core**

<img src="open-RAN-architecture-simplified-1920x1080.png" alt="alt text" width="800">

Al momento la **complessità** e il **costo** del **livello fisico** della RAN ha portato alla proposta di un'architettura mista che suddivide, sia in downlink che in uplink, la **gestione fisica dei mezzo** su **due apparati** distinti, uno **abbinato all'antenna**, un'altro a parte **comune a più antenne**. Di seguito è illustrato il dettaglio del livello fisico di **downlink** e **uplink**:

<img src="o-RAN2.png" alt="alt text" width="1000">

O-RAN **Fronthaul** definisce i seguenti **piani operativi**:

- **C-Plane** (Piano di controllo): i messaggi del piano di controllo definiscono la pianificazione, il coordinamento richiesto per il trasferimento dei dati, la formazione del fascio delle antenne, ecc.
- **U-Plane (User Plane)**: i messaggi del piano utente per un trasferimento dati efficiente entro i rigorosi limiti di tempo delle numerologie 5G.
 - **S-Plane (Piano di sincronizzazione)**: il piano di sincronizzazione è responsabile degli aspetti di temporizzazione e sincronizzazione tra O-DU e O-RU. Nelle implementazioni Cloud RAN, per svolgere tutta una serie di processi, è necessaria una sincronizzazione molto accurata tra O-DU e O-RU ottenuta sincronizzandosi con l'orologio ad alte prestazioni disponibile sul lato O-DU.
- **Piano M (piano di gestione)**: i messaggi del piano di gestione vengono utilizzati per **gestire l'unità radio**. **M-Plane** fornisce una varietà di funzioni di gestione per impostare i parametri sul lato O-RU come richiesto da C/U-Plane e S-Plane. Ad es. gestire il software O-RU, eseguire la gestione dei guasti, ecc. La specifica del fronthaul O-RAN per quanto riguarda l'M-Plane fornisce tra le varie cose modelli di dati che eliminino la dipendenza dall'implementazione di ciascun fornitore e rende possibile una vera Open RAN multi-vendor
    - Supporta il modello gerarchico/ibrido
    - C/U Plane IP e gestione dei ritardi
    - FCAPS inclusa la configurazione e lo stato della sincronizzazione
    
**FCAPS** è l'acronimo di **fault**, **configuration**, **accounting**, **performance**, **security**, le categorie di gestione in cui il modello ISO definisce le **attività di gestione** della rete. Nelle organizzazioni **senza fatturazione** la **contabilità** viene talvolta sostituita con l'**amministrazione**.

<img src="o-RAN3.png" alt="alt text" width="1000">

Un'altra **caratteristica** di **Open RAN** è il RAN Intelligent Controller (**RIC**) che aggiunge programmabilità alla RAN.

Un ambiente aperto espande l'ecosistema e, con più fornitori che forniscono gli elementi costitutivi, c'è più innovazione e più opzioni per gli operatori. Possono anche aggiungere nuovi servizi. Ad esempio, l'**Intelligenza Artificiale** può essere introdotta tramite il **RIC** per **ottimizzare** la rete in prossimità di uno stadio di calcio **il giorno** di una partita.

Il **Mobile Core** è un insieme di funzionalità (al contrario di un dispositivo) che serve a diversi scopi.
- Fornisce **connettività Internet (IP)** sia per i dati che per i servizi vocali.
- Assicura che questa connettività soddisfi i **requisiti di QoS** promessi.
- **Tiene traccia della mobilità** degli utenti per garantire un servizio ininterrotto.
- **Tiene traccia dell'utilizzo** dell'abbonato per la fatturazione e l'addebito.

Il **Mobile Core** in **4G** è chiamato Evolved Packet Core (**EPC**) e in **5G** è chiamato Next Generation Core (**NG-Core**).

La **figura sottostante** mostra un possibile uso delle **reti mobili private**. Un uso **indipendente dal gestore** con la **rete di telefonia aziendale** collegata alla LAN e **suddivisa** in **classi di servizio fisse** (realizzate magari con reti **fisicamente separate**), solo quelle necessarie ai processi aziendali. Un uso **dipendente dalla rete pubblica** dell'operatore telefonico che ritaglia servizi su misura per quell'azienda utilizzando per questo scopo le proprie **funzioni dinamiche** di **network slicing**.

<img src="Private 5G definition (en).png" alt="alt text" width="600">

Per comprendere l'impatto delle tecnologie e delle pratiche cloud applicate alla rete di accesso (RAN), è utile prima capire cosa è importante per il cloud. **Il cloud** ha cambiato radicalmente il modo in cui elaboriamo e, soprattutto, il ritmo dell'innovazione. Lo ha fatto attraverso una **combinazione** di quanto segue.

- **Disaggregazione**: scomposizione di sistemi integrati verticalmente in componenti indipendenti con interfacce aperte.

- **Virtualizzazione**: essere in grado di eseguire più copie indipendenti di tali componenti su una piattaforma hardware comune.

- **Commoditizzazione**: essere in grado di scalare in modo elastico quei componenti virtuali attraverso i **mattoncini hardware** di base in base al **carico di lavoro**.

Lo **slicing della rete 5G** è l'uso della **virtualizzazione della rete** per dividere le **singole connessioni** di rete in **più connessioni virtuali** distinte che forniscono **diverse quantità di risorse** a **diversi tipi** di traffico.

**Per realizzare** quantitativamente tale concetto, vengono impiegate **diverse tecniche**:
- **Funzioni di rete**: esprimono funzionalità di rete elementari che vengono utilizzate come "mattoni" per creare ogni fetta di rete.
- **Virtualizzazione**: fornisce una rappresentazione astratta delle risorse fisiche secondo uno schema unificato ed omogeneo. Inoltre, consente una distribuzione scalabile delle sezioni basata su NFV che consente il disaccoppiamento di ogni istanza della funzione di rete dall'hardware di rete su cui viene eseguita.
- **Orchestrazione**: è un processo che consente il coordinamento di tutte le diverse componenti di rete che sono coinvolte nel ciclo di vita di ciascuna fetta di rete. In questo contesto, SDN viene impiegato per consentire una configurazione delle sezioni dinamica e flessibile.

<img src="slicing.png" alt="alt text" width="1000">

Elenco di **prodotti opensource** utilizzabili per la gestione di una **RAN 5G**:

<img src="LF-Open-Source-Component-Project-for-5G.png" alt="alt text" width="1000">

**Sitografia:**

- https://www.u-blox.com/en/publication/white-paper/short-range-low-power-wireless-devices-and-internet-things-iot
- https://www.digikey.it/it/articles/how-to-quickly-start-low-power-wireless-iot-sensing
- https://www.researchgate.net/publication/333226782_Proposal_for_the_Design_of_Monitoring_and_Operating_Irrigation_Networks_Based_on_IoT_Cloud_Computing_and_Free_Hardware_Technologies
- https://www.electronicshub.org/wireless-sensor-networks-wsn/
- http://www.iosrjournals.org/iosr-jece/papers/Vol.%2011%20Issue%206/Version-1/H1106014856.pdf
- https://it.wikipedia.org/wiki/Low_Energy_Adaptive_Clustering_Hierarchy
- https://repository.vtc.edu.hk/cgi/viewcontent.cgi?article=1043&context=ive-eng-sp
- https://it.wikipedia.org/wiki/Ad-hoc_On-demand_Distance_Vector
- http://tesi.cab.unipd.it/28281/1/Tesi.pdf
- https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.736.8723&rep=rep1&type=pdf
- https://www.researchgate.net/publication/316563960_Comparative_study_and_simulation_of_flat_and_hierarchical_routing_protocols_for_wireless_sensor_network
- https://dpmrassociation.org/dPMR-Spectrum-Efficiency.html
- https://en.wikipedia.org/wiki/ALOHAnet
- https://research-information.bris.ac.uk/ws/portalfiles/portal/110218284/duquennoy17tsch.pdf
- https://hal.inria.fr/hal-01282597/file/iwipcps.pdf
- https://it.scribd.com/document/462861362/Dash7Surveyfinalreport-pdf
- https://openwsn.atlassian.net/wiki/spaces/OW/overview
- https://www.mdpi.com/2224-2708/2/2/235/htm
- https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6806188/
- https://en.wikipedia.org/wiki/BACnet
- https://it.wikipedia.org/wiki/KNX_(standard)
- https://optigo.net/blog/which-better-bacnet-lonworks-modbus-or-knx
- https://en.wikipedia.org/wiki/List_of_automation_protocols
- https://iot.eetimes.com/private-lte-networks-for-critical-iot-applications/
- https://www.samsung.com/global/business/networks/insights/blog/0503_how-is-a-private-5g-network-different-from-a-public-5g-network/
- https://www.grandmetric.com/2018/03/15/how-does-ofdm-work/
- https://www.asus.com/it/support/FAQ/1042759/
- https://www.gta.ufrj.br/ensino/eel879/trabalhos_vf_2014_2/rafaelreis/ofdma_scfdma.html
- https://m.eet.com/media/1072709/LTE_Chapter2_38to42.pdf
- https://www.wwt.com/article/wi-fi-6e-the-next-generation-of-wireless/
- https://www.netmanias.com/en/post/oneshot/14103/5g/5g-protocol-stack-user-plane-control-plane
- https://www.netmanias.com/en/post/blog/14500/5g-edge-kt-sk-telecom/7-deployment-scenarios-of-private-5g-networks
- https://www.sdxcentral.com/5g/definitions/5g-network-slicing/
- https://www.zte.com.cn/global/about/magazine/zte-technologies/2018/1/Special-Topic/5G-network-Slicing.html
- https://en.wikipedia.org/wiki/5G_network_slicing
- https://www.nokia.com/about-us/newsroom/articles/open-ran-explained/
- https://www.techplayon.com/o-ran-fornthual-c-u-sync-mgmt-planes-and-protocols-stack/
- https://www.techplayon.com/o-ran-fronthaul-spilt-option-7-2x/
- https://www.gruppotim.it/tit/it/notiziariotecnico/edizioni-2019/n-3-2019/N3-Open-RAN-dalle-specifiche-ai-trials/approfondimenti-1.html
- https://www.cnit.it/wp-content/uploads/2020/09/Spieghiamo-il-5G-200803.pdf
- https://www.mdpi.com/1999-5903/12/3/46/htm

>[Torna all'indice generale](index.md)
    




