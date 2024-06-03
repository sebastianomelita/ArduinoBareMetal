>[Torna a reti di sensori](sensornetworkshort.md#classificazione-delle-tecnologie-wsn-in-base-a-velocità-e-copertura)

## **Caso d'uso BLE** 

Date le particolarità della tecnologia, i casi d'uso per la rete di sensori sono quelli tipici applicazioni **IoT indoor** a **corto/medio raggio**, dove concorre con altre tecnologie di rete: WIFi, Zigbee e, sotto certe condizioni, LoRaWAN. Per la sensoristica Indoor ha praticamente gli stessi punti di forza di Zigbee con il quale è praticamente intercambiambile (ma non interoperabile).

<img src="img/BLEzone.png" alt="alt text" width="800">

Esiste però un ambito nel quale il BLE è attualmente **senza rivali** rispetto alle tecnologie Zigbee, WiFi e LoRaWAN: il **tracciamento indoor** degli asset aziendali e la **localizzazione indoor** degli utenti. Il meccanismo che consente queste funzioni si basa sulla trasmissione di particolari messaggi periodici detti **beacon**. 

La tecnologia dei **beacon** è **comune** a quasi tutti i protocolli wireless moderni, compresi Zigbee, WiFi e LoRaWAN, che quindi sono in parte capaci anche loro delle funzioni di localizzazne suddette, ma in maniera molto **meno precisa** e versatile.

## **Architettura di una rete di reti** 

Di seguito è riportata l'architettura generale di una **rete di reti** di sensori. Essa è composta, a **livello fisico**, essenzialmente di una **rete di accesso** ai sensori e da una **rete di distribuzione** che fa da collante di ciascuna rete di sensori.

<img src="sensornet1.png" alt="alt text" width="800">

I **gateway** utilizzano la **rete internet** e/o una **LAN** per realizzare un collegamento verso il **broker MQTT**, per cui, in definitiva, la topologia risultante è, **fisicamente**, quella di più **reti di accesso** con tecnologia e topologia differente (a maglia nel caso di zigbee) tenute insieme da una **rete di distribuzione** qualsiasi purchè sia di tipo TCP/IP (LAN o Internet).

Avere a disposizione una **rete di distribuzione IP** per i comandi e le letture è utile perchè permette di creare interfacce web o applicazioni per smartphone o tablet per:
- eseguire, in un'unica interfaccia (form), comandi verso attuatori posti su reti con tecnologia differente.
- riassumere in un'unica interfaccia (report) letture di sensori provenienti da reti eterogenee per tecnologia e topologia

### **Ruolo del broker MQTT** 

Il **broker MQTT** è solo una delle tante soluzioni possibili per:
- inoltro dei comandi da un **sensore di comando** su una rete di tipo A (ad es. LoRaWAN) ad un attuatore su una rete diversa di tipo B (ad es. Zigbee)
- inoltro di una **misura** da un **sensore ambientale** su una rete di tipo A (ad es. LoRaWAN) ad un **pannello di controllo** su una rete diversa di tipo B (tipicamente IP)
- inoltro di una **misura** da un **sensore ambientale** su una rete di tipo A (ad es. LoRaWAN) ad un **server di gestione** su una rete diversa di tipo B (tipicamente IP) per:
-  elaborazioni a **breve termine** quali la generazione di **statistiche** per la determinazione di **soglie** o **predizioni** per:
    - realizzazione da remoto della **logica di comando** (processo dei comandi) degli **attuatori**
    - **report** per l'assistenza alle decisioni
    - generazioni di **allarmi**
    - realizzazione di **ottimizzazioni** della gestione o del consumo di risorse, energia o materie prime
    - contabilizzazione dei consumi (**smart metering**)
    - controllo e sorveglianza in tempo reale dello **stato** di impianti o macchinari
    - segnalazione dei **guasti** o loro **analisi predittiva** prima che accadano
    - **consapevolezza situazionale** di ambienti remoti, difficili, pericolosi o ostili (https://it.wikipedia.org/wiki/Situational_awareness)
-  elaborazioni a **lungo termine** quali:
    - analisi dei dati per la realizzazione di studi scientifici
    - elaborazione di nuovi modelli statistici o fisici o biologici dell'ambiente misurato
      
Il **vantaggio** del **broker MQTT** è quello di poter gestire in modo semplice e **standardizzato** lo **smistamento** (inoltro) delle **misure** e dei **comandi** tra i vari portatori di interesse (stakeholder) di un **cluster** di reti di sensori, siano essi utenti umani, interfacce grafiche, server applicativi diversi o altri dispositivi IoT.

Esistono molte altre soluzioni che magari sono più semplici e graficamente accattivanti ma che passano per portali proprietari o per servizi cloud a pagamento e nulla aggiungono di didatticamente rilevante ai nostri discorsi.


## **Gateway**

### **Ruolo del gateway** 

**Riassumendo**, alla **rete di distribuzione IP** si collegano, quindi, una o più **reti secondarie** che servono da **rete di accesso** per i dispositivi sensori o attuatori con **interfacce** spesso di tipo **non ethernet** che necessitano di un **gateway** di confine con possibili funzioni di:     
  - **Inoltro**, cioè smistamento dei messaggi da un tipo di rete all'altro di tipo L3 (**routing**) o di tipo L2 (**bridging**). L'inoltro del messaggio di un sensore può essere:
       - **diretto** nella rete di distribuzione tramite link fisico verso il dispositivo di smistamento (router o switch) più vicino.
           - Nel caso di una LAN il gateway possiede un indirizzo IP privato.
           - Nel caso di una WAN il gateway possiede un indirizzo IP pubblico.
       - **indiretto** tramite una dorsale virtuale, cioè un **tunnel**, verso il network server o verso un router di una WAN privata, realizzato, ad esempio, in maniera cifrata tramite un **client di VPN**, oppure in maniera non cifrata tramite un client di tunnel generico **GRE**. In questo caso il **gateway** possiede:
         - un indirizzo **IP pubblico** nell'interfaccia in **Internet**. 
         - un **IP privato** nell'interfaccia sul **tunnel cifrato**. Il tunnel cifrato, per VPN L3, è gestito con una subnet IP **separata** per ogni tunnel.
  - **Traduzione di formato** dei messaggi da rete a bus a rete ethernet con eventuale realizzazione del **bridge** L4 tra il livello applicativo in uso nella rete di sensori e quello in uso nella rete di distribuzione.
  - **Interrogazione periodica** (polling) dei dispositivi nella rete di sensori (master di una architettura master/slave)
  - **Raccolta e memorizzazione** delle informazioni per essere trasferite in un **secondo momento** al server di gestione
  - **Protezione della rete di sensori**, cioè di firewall, soprattutto quando questa, tramite il gateway, si connette direttamente alla rete **Internet** mediante un **IP pubblico**.

Il gateway è uno **snodo nevralgico** dei messaggi, per cui la sua posizione dovrebbe essere **ben nota** e accuratamente **riportata in planimetria** per permettere una sua rapida manutenzione/sostituzione.


### **Traduzione della semantica applicativa** 

**Ble2mqtt** è un software open-source progettato per permettere ai dispositivi BLE di comunicare direttamente con un server MQTT (Message Queuing Telemetry Transport) senza la necessità di un hub proprietario.  [Ble2mqtt](https://github.com/devbis/ble2mqtt)


BLE è uno standard di comunicazione wireless utilizzato anche per il controllo e l'automazione domestica, mentre MQTT è un protocollo di messaggistica leggero utilizzato per il trasferimento di dati tra dispositivi. Utilizzando BLE2mqtt, gli utenti possono integrare facilmente dispositivi BLE di diversi produttori in un sistema di automazione domestica basato su MQTT, offrendo maggiore flessibilità e controllo.

**Ble2mqtt** opera a livello di **applicazione** della pila OSI in quanto **traduce** un **payload BLE** in un **payload JSON MQTT**  (gateway = router applicativo). Si tratta di un software che consente di integrare dispositivi BLE in un'infrastruttura di domotica basata su MQTT (Message Queuing Telemetry Transport). Ble2mqtt funge da **ponte** tra la rete BLE e il broker MQTT, consentendo agli utenti di interagire con i dispositivi BLE tramite messaggi MQTT.

## **Documentazione logica della rete (albero degli apparati attivi)** 

### **Federazione di reti BLE in Internet** 

L'albero degli **apparati attivi** di una rete di sensori + rete di distribuzione **in Internet** + server di gestione e controllo che potrebbe rappresentare **tre edifici** distanti domotizzati tramite **BLE** e federati tramite **Internet**: 

<img src="img/integratedGW-BLE.png" alt="alt text" width="900">

Il **bridge BLE** (in realtà è un **gateway** e quindi pure un router) è normalmente anche il **master o centrale** della rete di sensori. 

Il **broker MQTT** può essere installato in cloud, in una Virtual Private network, oppure On Premise direttamente nel centro di gestione e controllo. 

Il **gateway**, quando collegato direttamente ad **Internet**, è normalmente anche un **firewall** (con funzioni di NAT se si adopera IPv4), mentre se collegato alla **LAN** (attraverso uno SW o un HUB wiereless) ha solamente la **funzioni** di:
- **router applicativo** che **traduce** i messaggi da una rete IP (la LAN) ad una non IP (la rete di sensori).
- **client MQTT** con funzione di **publisher** (sul topic di stato e traduce **da** i dispositivi) e di **subscriber** (sui topic di comando e configurazione e traduce **verso** i dispositivi).

### **Federazione di reti BLE su LAN** 

#### **Partizionamento e ridondanza** 

Per quanto riguarda il **numero dei gateway** in una stessa **LAN**, il numero minimo necessario perchè la rete zigbee funzioni è **uno**. Un gateway avente anche funzione di **master** nelle rete di sensori. Però, data la **criticità** di eventuali **guasti** su questo dispositivo (la rete di sensori diventa nel suo complesso **inaccessibile**), potrebbe essere opportuno prevedere:
- localmente la **ridondanza dei gateway**. Almeno 2 gateway per ogni rete di sensori. Uno master attivo di default, e uno slave che entra in azione quando sente che il proprio master è non raggiungibile.
- globalmente un **partizionamento della rete** di sensori in più settori con frequenze diverse e gestiti da coordinatori diversi inseriti in **più gateway sparsi** in **zone diverse** dell'impianto.
  
La **partizione** di una rete BLE può essere utile in determinate situazioni, specialmente se si hanno un **gran numero** di dispositivi o se si vuole **separare** i dispositivi **per zone** o **per scopi** diversi. Ecco alcune **situazioni** in cui potrebbe essere vantaggioso partizionare una rete Zigbee:

- **Servizi e profili**: Nelle reti BLE, i dispositivi comunicano attraverso **servizi** e **profili** Bluetooth standardizzati. Puoi organizzare i dispositivi in **gruppi** basati su **servizi simili** o scopi simili. Ad esempio, potresti avere un gruppo di dispositivi BLE che forniscono dati di monitoraggio della salute e un altro gruppo di dispositivi che controllano i dispositivi domestici intelligenti.

- **Partizionamento fisico**: Si possono dividere fisicamente le reti BLE in base alla loro **posizione** o alla loro **area di copertura**. Ad esempio, potresti avere un insieme di dispositivi BLE all'interno di un'abitazione e un altro insieme di dispositivi all'esterno. In questo caso, potresti usare **più gateway** o controller per gestire le diverse parti della rete.

- **Rilevanza dei dati**: Si possono configurare i  dispositivi BLE per trasmettere solo i dati rilevanti per una particolare area o scopo. Ad esempio, in un grande magazzino, si potrebbe voler raccogliere solo i dati dai sensori nelle aree attive durante un determinato momento anziché da tutti i dispositivi nell'intero magazzino.

- **Gestione del traffico**: Si potrebbe pianificare la distribuzione dei dispositivi BLE in modo da evitare congestioni di traffico e interferenze. Ad esempio, si potrebbe evitare di sovraccaricare una specifica area con troppi dispositivi BLE che trasmettono contemporaneamente.

Il partizionamento delle reti BLE può essere utile per migliorare l'efficienza, la sicurezza e la gestibilità di una infrastruttura IoT. Per **partizionare** una rete BLE, si potrebbero creare **più centrali** BLE, cioè più **gateway**, ciascuno con la propria rete di sensori da gestire, e utilizzare una **LAN** (compoasta da switch) per collegare le reti tra loro. 

<img src="img/integratedGW-BLE-LAN.png" alt="alt text" width="900">

## **Reti BLE per tracciamento e localizzazione** 

Si tratta di un utilizzo diverso da quello di una normale rete di sensori poichè l'obiettivo finale non è creare una rete di dispositivi domotici composta da sensori e attuatori ma creare una rete di dispositivi per la **localizzazione** e il **tracciamento**, nel **tempo** e nello **spazio**, della posizione di **altri dispositivi**.

L'**iBeacon di Apple** è stata la prima tecnologia BLE Beacon a essere pubblicata, quindi la maggior parte dei beacon si ispira al formato dati iBeacon. Gli iBeacon sono abilitati in molti SDK di Apple e possono essere letti e trasmessi da qualsiasi iDevice abilitato per BLE. IBeacon è uno standard proprietario e chiuso. 
I beacon trasmettono quattro informazioni:
- Un UUID che identifica il beacon.
- Un numero maggiore che identifica un sottoinsieme di beacon all'interno di un grande gruppo.
- Un numero minore che identifica un beacon specifico.
- Un livello di potenza TX in complemento di 2, che indica la potenza del segnale a un metro dal dispositivo. Questo numero deve essere calibrato per ciascun dispositivo dall'utente o dal produttore.
  
<img src="img/ibeaconframe.png" alt="alt text" width="600">

Un'applicazione di scansione legge l'UUID, il numero maggiore e il numero minore e li usa come riferimento per ottenere informazioni sul beacon da un database; 
il beacon stesso non porta informazioni descrittive, richiede che questo database esterno sia raggiungibile. 
Il campo di potenza TX viene utilizzato con l'intensità del segnale misurata per determinare la distanza del dispositivo beacon dallo smartphone. Si noti che TxPower deve essere calibrato dall'utente per raggiungere una buona precisione.

### **Scopo iBeacon** 
Questo innovativo uso della tecnologia fa principalmente leva su due concetti chiave: 
- **Micro-location**: è noto che uno dei punti di forza dei Beacon che sfruttano Bluetooth LE è la possibilità di una localizzazione a corto raggio precisa e rapida (complementare al GPS):
     - **Posizionamento assoluto**, i GPS forniscono una posizione assoluta in termini di latitudine e longitudine ma ha difficoltà a raccogliere i segnali dei satelliti GPS all’interno degli edifici, soprattutto a più piani. Per questo il sistema di GPS è inadatto per fornire una localizzazione precisa in un raggio di 15-20 metri,
     - **Posizionamento relativo**. La Micro-location lavora semplicemente con la distanza relativa del device dai punti noti dove sono posizionati i Beacon. La geolocalizzazione basata su Ibeacon è ad un alto livello di granularità ed è appunto definita convenzionalmente con il termine «Micro-location».
     - **Possibilità di tracciamento**. Gli iBeacons sono traccianti, quando un device lascia l'area da loro coperta, sono capaci di monitorare la sua posizione nel tempo. Una rete di beacon può **tracciare** gli **spostamenti** di **persone** e **manufatti** in un intero edificio o **sito aziendale**.
- **Interaction/Engagement/Context**:
     - I segnali degli iBeacons possono interagire con le Apps, inviando notifiche (sia al sistema che all’utente) e facendo eseguire a queste ultime **azioni specifiche** in un **tempo** ed una **locazione specifica**. 
     - Sostanzialmente, il servizio fornito dai Beacons è quello di fornire un **contesto esterno** all'**applicazione**, che può ora conoscere **cosa** realmente **circonda l'utilizzatore** del device. Per questo motivo viene aperta la possibilità di far **interagire** le applicazioni mobili con il **mondo fisico** circostante senza alcuno sforzo (in maniera **trasparente** all’utente), in modo che l'utente finale possa davvero percepire il collegamento tra la realtà ed il device che tiene tra le mani. 

Importante sottolineare alcune **proprietà** peculiari del BLE: 
- **Trasparenza dell’utente**. Ogni device con una connessione Bluetooth attiva e l'App adeguata **reagirà automaticamente**, in tempo reale, ai Beacon circostanti, senza la necessità che l'**utente intervenga**, per esempio estraendo lo smartphone dalla tasca. E' come se l'app avesse dei **sensi** che le permettono di reagire **autonomamente** in maniera adeguata al **contesto** in cui si trova. L’attivazione del consenso all'utilizzo dei beacon da parte di una app dovrebbe essere eseguita esplicitamente dall’utente in maniera tale da evitare di autorizzare **inconsapevolmente** azioni pericolose per la privacy.
- **Tracciamento spaziale e temporale**. Il negoziante inoltre potrà monitorare le **aree** di maggior **interesse** dei propri clienti ancora prima che questi acquistino qualcosa, in quanto i Beacon possono tracciare ogni spostamento all'interno del negozio, permettendo al sistema di determinare gli scaffali davanti ai quali i clienti si fermano di più. Queste informazioni di **navigazione** degli **scaffali** di un **negozio** sono analoghe a quelle fornite dai cookie durante la **navigazione** delle **pagine** di un **browser** e, pertanto, sono informazioni altrettanto utili in termini di **marketing**. Ma i **beacon** possono anche essere usati per fornire all'utente **contenuti associati** al **luogo** in cui l'**app** si trova, come ad esempio la **descrizione** di un'opera d'arte in un museo o la **visualizzazione di una interfaccia** di comando o configurazione di un impianto tecnologico.
- **Interfacce di comando universale**. Gli hardware iBeacon saranno sempre più piccoli e ed economici: possono essere programmati non solo per essere degli Advertiser ma anche degli Scanner, cosìcché un semplice Smartphone, che è a tutti gli effetti un Beacon, possa inviare comandi ai dispositivi installati per esempio in casa, per accendere luci o altri elettrodomestici a distanza. 
- **Struttura autonoma**. Un altro punto fondamentale è che non è mai richiesta una connessione ad Internet: anche se poter accedere a risorse su cloud tramite la rete potrebbe ampliare ancora di più il range di attività possibili con BLE.

**In definitiva**, gli iBeacon utilizzano Bluetooth Low Energy per creare un'infrastruttura smart, orientata alla localizzazione, che i dispositivi mobili possono utilizzare per ricavare **informazioni contestuali** basate sull'**ambiente stesso** in cui si **muovono**, in **tempo reale**. 

Le applicazioni possono ora sapere esattamente dove si trovano e cosa le circonda, aprendo la strada ad un nuovo livello di interazione col mondo, senza bisogno di una connessione ad Internet.


### **Schema di cablagggio a beacon fisso** 

È l’approccio più comune. I beacon sono posizionati in **posizioni fisse** e note, rispetto a una **mappa interna**.  

I dispositivi mobili (**listener**) abilitati Bluetooth **riconoscono** i beacon quando questi si trovano all'interno del **raggio** della loro **portata** (variabile dal metro alle decine di metri) e determinano la **posizione assoluta** (latitudine e longitudine) del dispositivo sulla mappa, stimata grazie a misure di **distanza**:
- con la **distanza** stimata da un **1 beacon** si stabilisce solo il **raggio di presenza** del beacon
- con la **distanza** stimata da almeno **3 beacon** si stabilisce la **posizione puntuale** del beacon nel piano, è il meccanismo della **trilaterazione**
- con la **distanza** stimata da almeno **4 beacon** si stabilisce la **posizione puntuale** del beacon nello spazio (viene introdotta l'altezza), è il meccanismo della **trilaterazione** + altezza

I **dati di tracciamento** raccolti da ciascun **dispositivo mobile** possono quindi essere inviati, via wifi o modem UMTS, a un **sistema centralizzato** a scopo analitico e ad altri servizi come la **mappatura** delle presenze in tempo reale, o delle **localizzazioni** in tempo reale.

Si noti che in questo **approccio** i **dispositivi da localizzare** hanno il ruolo di listener e devono essere essere **collegati alla rete**, mentre i **dispositivi fissi** no.

La particolarità di questo approccio è:
- un **numero elevato** di beacon a basso costo installati sulle zone da presidiare
- l'**assenza** di una infrastruttura di **rete** per i beacon che sono, a tutti gli effetti, **dispositivi isolati**.
- la **responsabilità** dell'elaborazione e del collegamento al server **delegata** al **listener** che si **sposta**.

<img src="img/fixedbeacon.png" alt="alt text" width="900">

**Esempio**: tour digitale in un museo, in cui ogni stanza o attrazione potrebbe avere un beacon fisso che emette un tag BLE specifico. Se una persona sceglie di installare l'app mobile del museo, il suo telefono, ogni qualvolta cammina **vicino a un beacon**, legge il **tag** e le informazioni pertinenti e mirate sull'esposizione potrebbero essere recuperate da ciascun dispositivo tramite il tag, consentendo un'esperienza più istruttiva e coinvolgente per i visitatori.

**Esempio ambulatorio.**

<img src="img/ambulatorio.gif" alt="alt text" width="900">

**Esempio shopping.**

<img src="img/shopping.jpg" alt="alt text" width="900">

**Esempio ospedale.**

<img src="img/ospedale.png" alt="alt text" width="900">


### **Schema di cablagggio a scanner fisso** 

È un’approccio sempre più diffuso. Questi "**ascoltatori**" fissi raccolgono gli **identificativi** di tutti i beacon Bluetooth alla loro **portata** e sono loro stessi che **trasmettono** le informazioni raccolte a un **sistema centralizzato** per l'analisi, invece che un **dispositivo mobile** collegato alla rete.

Il **sistema centrale** applicherà alcuni filtri digitali e, in base alla posizione degli ascoltatori fissi nota sistema, determinerà la posizione dei beacon mediante la **trilaterazione**. 

Utilizzando **questo approccio**, invece di tracciare o localizzare  un **dispositivo mobile** costoso (smartphone) , si effettua il tracciamento dei **singoli beacon** che, grazie al **costo irrisorio** e alle **ridodittissime dimensioni**, consentono una serie di casi d'uso nuovi e innovativi.

Si noti che in questo **approccio** i **dispositivi fissi** hanno il ruolo di listener e devono essere essere **collegati alla rete**, mentre i **dispositivi da localizzare** no.

Questo **approccio** è però molto più **oneroso** per l'infrastruttura perchè adesso serve una **rete di distribuzione** delle informazioni di tracciamento raccolte dai **listener** che deve essere **estesa** almeno quanto tutta l'**area da presidiare**.

<img src="img/fixedscanner.png" alt="alt text" width="900">

**Esempi**:
- **Tracciamento** di **risorse** in una **fabbrica** (materie prime o prodotti) o in un **cantiere** (utensili, macchinari spostabili, attrezzature in genere) identificate da un **tag univoco**. **Beacon** solidali alle risorse e **listener** installati lungo la linea di produzione forniscono **dati di tracciamento** in **tempo reale** non solo rispetto alla **posizione** ma anche a **quantità di tempo** (ad esempio, il tempo in cui alcuni pezzi rimangono in un reparto di una fabbrica).
- **Tracciamento** partecipanti di una **fiera** a cui sono stati dati dei piccoli beacon BLE grandi come una moneta da portare con sé, che possono interagire con gli **"ascoltatori"** BLE posizionati presso gli stand dimostrativi e nelle sale riunioni dell'evento. A seconda dell'occasione, del caso d'uso e dell'obiettivo aziendale, tale sistema è in grado di:
    - raccogliere solo **metriche anonime** e **aggregate**, quali numero di presenze e analisi di eventi in tempo reale 
    - gestire **code e appuntamenti**, potrebbe anche essere utile ai partecipanti (i gestori di beacon) che, una volta **in prossimità** di un evento, potrebbero tramite un'**app** interrogare il sistema centrale per **prenotare** i posti e **accodarsi** virtualmente in tempo reale senza mettersi **fisicamente** in fila.
    - raccogliere **informazioni puntuali** e personali. Con l’esplicito accordo degli utenti, e solo nei limiti di dove sono stati collocati gli "ascoltatori", si potrebbe tracciare in tempo reale la posizione di **specifiche persone**.

Nell'architettura a **scanner fissi** i dispositivi BLE sono analoghi ai gateway dell'infrastruttura BLE Mesh solo che adesso lo scopo è radicalmente diverso, non servono a connettere l'intera rete di sensori alla LAN, ma a mandare ad un server una **informazione di tracciamento** da parte del listener che la ha raccolta. Poichè il **beacon si muove** nello spazio, potenzialmente in tutti gli ambienti, è necessario installare **molti scanner** lungo i percorsi che si desiderano tracciare e **non pochi gateway** soltanto.

#### **Posizionamento listener (scanner)**

Nell'architettura a **scanner fissi** i dispositivi BLE **non** possono restare **isolati** ma devono comunicare le informazioni sui beacon di passaggio nelle vicinanze ad un **server centrale** e, per far questo, hanno necessità di una rete da utilizzare come infrastruttura di **comunicazione**. Esistono alcune **alternative**:
- utilizzare la **rete LAN cablata** a cui i gateway BLE si collegano fisicamente mediante una presa Ethernet (RJ45). Il **prerequisito** di questa soluzione è un **ambiente cablato** in maniera uniforme e capillare.
- utilizzare una **rete di AP WiFi** a cui collegare i gateway come client. **Presuppone** una rete wifi con una copertura adeguata.
- utilizzare una **rete BLE mesh** realizzata dagli scanner e da altri dispositivi eventualmente presenti nell'ambiente con altri scopi (sensori o attuatori). Il **prerequisito** è avere una rete BLE mesh distribuita in maniera **sufficientemente capillare** negli ambienti dove si intende realizzare un servizio di **tracciamento** o **localizzazione indoor**.

**Esempio tracking assets industriali**

<img src="img/industrialtracking.jpg" alt="alt text" width="900">

### **Rete di sensori BLE** 

<img src="img/piconet.png" alt="alt text" width="1000">

E' una architettura a stella gerarchica (albero). E' realizzata da un solo dispositivo master. Un master può essere contemporaneamente pure slave di un'altra piconet.

## **Topologie di connessione**

### **Beacon**

I beacon sono delle **sequenze di sincronizzazione** (dette preambolo) in grado sia di sincronizzare gli **orologi** dei dispositivi (Tx e Rx) che si accingono ad iniziare una comunicazione, ma anche di **indentificare** in maniera univoca i dispositivi che li emettono. Per dei dettagli vedi [preambolo di sincronizzazione](protocolli.md#preambolo-di-sincronizzazione).

La trama dati compresa tra **due beacon** consecutivi viene detta **supertrama** (superframe) ed è generalmente divisa in due zone con **politiche di accesso** al canale diverse:  
- una **deterministica** al riparo dalle collisioni detta **CFP** (Contention Free Period) e regolata dalla multiplazione statica TDMA, che viene usata per trasmettere i dati delle comunicazioni **unicast**.
- una **probabilistica** a contesa, in cui i tentativi di accesso dei dispositivi sono soggetti al **rischio di collisione** perchè regolata da un protocollo di tipo **CSMA/CA**, che invece serve per trasmettere delle particolari informazioni **broadcast** dette **advertisement**.

Nel contesto di BLE, un **centrale** può assumere un ruolo di **coordinamento**, simile a quello svolto dal PCF in una rete Wi-Fi, gestendo l'accesso al canale in modo master/slave in cui il centrale ha il ruolo di **master** che stabilisce **quale** stazione deve parlare, **quando** e **per quanto** tempo usando una politica di **turnazione** delle stazioni (**polling**).

Tutti i **dispositivi BLE** emettono beacon per cui il nome di beacon alla fine è finito per identificare anche un **generico dispositivo BLE**.

Le **superframe BLE** sono divise principalmente in tre zone:

- **Advertising Zone** (Zona di pubblicità): In questa zona, i dispositivi BLE trasmettono pacchetti di advertising per annunciare la propria presenza e offrire servizi ai dispositivi circostanti. Questa zona è utilizzata per l'inizializzazione delle connessioni e per il broadcasting di informazioni.

- **Connection Establishment Zone** (Zona di stabilità della connessione): Quando un dispositivo BLE desidera stabilire una connessione con un altro dispositivo, utilizza questa zona per scambiare pacchetti di connessione. Una volta stabilita la connessione, si passa alla terza zona.

- **Connection Events Zone** (Zona degli eventi di connessione): In questa zona avvengono gli eventi di trasmissione e ricezione di dati tra i **dispositivi connessi**. La struttura temporale di questa zona può variare in base alle necessità di trasmissione dei dati e al risparmio energetico.

Queste zone sono organizzate in un **ciclo temporale** (la supertrama) che si ripete periodicamente per consentire la comunicazione tra i dispositivi BLE in modo efficiente e sincronizzato.

### **Topologia broadcast**

Il **beacon non collegabile** è un dispositivo Bluetooth (broadcaster) a bassa energia in modalità di trasmissione. Trasmette semplicemente informazioni **statiche** archiviate internamente **senza ricevere** alcunchè da un eventuale observer. Dato che la trasmissione è **non connettibile** non attiva alcuna risorsa HW di ricezione, per cui ha il **minor consumo** energetico possibile. Il dispositivo deve semplicemente svegliarsi, trasmettere (pochi) dati e tornare a dormire (modalità radiofaro).  Ciò comporta l'inconveniente che gli unici dati **dinamici** sono limitati a ciò che è noto al dispositivo o a ciò che è disponibile tramite altri canali di cui è dotato il dispositivo quali interfacce seriali RS232 (UART), periferiche 4-wire (SPI), bus seriale universale (USB) e così via.

Gli **attori** di questa modalità sono quindi **due**:
- il **Broadcaster**: Invia periodicamente pacchetti di **advertise** non connettibili a chiunque sia disposto a riceverli. 
- l'**Osservatore** (observer): Esegue ripetutamente la **scansione** delle frequenze predefinite per ricevere eventuali pacchetti di advertisement non connettibili attualmente trasmessi (**scanning passivo**).

<img src="img/nolinkbeacon0.png" alt="alt text" width="1100">

I pacchetti di **advertisement** sono periodici e sono messaggi di beacon trasmessi in broadcast da dispositivi broadcaster detti, per l’appunto, essi stessi beacon. Da questi l’osservatore ricava informazioni minimali (tag). 

### **Topologia connessa**
Il **beacon collegabile** (o periferica) è un dispositivo Bluetooth a bassa energia in **modalità periferica**, il che significa che può non solo trasmettere, ma anche **ricevere** e quindi potrebbe anche essere **interrogato periodicamente** per interagire con i **servizi** implementati sul dispositivo beacon effettuando:
- **letture (R)**. Il beacon potrebbe essere interrogato per effettuare, ad esempio, il polling di alcuni sensori, o per conoscere lo stato delle sue batterie.
- **notifiche (N)**. Il master (centrale o Client) potrebbe **registrarsi** per ottenere la notifica di qualche evento gestito dal beacon, ad esempio un segnale quando un valore di interesse scende sotto una certa soglia.
- **scritture (W)**. I servizi forniscono una o più caratteristiche che potrebbero essere modificate da un dispositivo peer. Un esempio di queste caratteristiche potrebbe essere la stringa di dati che rappresenta le informazioni trasmesse dal beacon. In questo modo è possibile avere un **beacon configurabile** che può essere facilmente aggiornato via etere proprio attraverso il bluetooth.

Gli **attori** di questa modalità sono sempre **due**:
- **Dispositivo master o centrale**: Esegue periodicamente la scansione delle frequenze predefinite alla ricerca di pacchetti pubblicitari connettibili e, se ne trova uno adatto, avvia una connessione (scanning attivo). Una volta stabilita la connessione, il **dispositivo centrale** gestisce i tempi e avvia gli scambi periodici di dati, diventa, cioè, il **master** della comunicazione.
- **Dispositivo slave o periferica**: un dispositivo che invia periodicamente pacchetti pubblicitari (advertisement beacon) connettibili in broadcast e **accetta** connessioni in entrata. Una volta iniziata una connessione, la periferica segue i tempi del master centrale e scambia regolarmente dati con esso, quindi, dopo la connessione, assume il ruolo di **slave** della comunicazione.

<img src="img/linkbeacon.png" alt="alt text" width="1100">

La differenza tra la scansione dei beacon effettuata da un dispositivo **centrale** e quella effettuata da un semplice **observer** sta nel fatto che la prima è una ricerca che è abilitata ad instaurare una connessione **bidirezionale** con i dispositivi beacon periferici, mentre la seconda è una scansione che permette l’attivazione di connessioni di **sola ricezione**. 

### **Topologia mista**

Stanno iniziando a comparire dispositivi dual-mode e single-mode più avanzati, dispositivi in ​​grado di combinare più ruoli contemporaneamente. Ciò consente loro di partecipare a più connessioni contemporaneamente, mentre usano gli advertisement per trasmettere informazioni in broadcast.

<img src="img/blemisto.png" alt="alt text" width="600">

## **GATT**

I dispositivi BLE, dal punto di vista SW, si dividono in dispositivi **Client** e in dispositivi **Server**:

- Un **BLE client** è un dispositivo che inizia una connessione con un BLE server e richiede informazioni o servizi da quest'ultimo. Il client invia richieste specifiche al server e riceve le risposte pertinenti. Il client è anche uno scanner, cioè **scansiona** i dispositivi nelle vicinanze per scoprire i server BLE disponibili. Una volta trovato un server desiderato, il client stabilisce una **connessione** e invia **richieste** di lettura, scrittura o notifica per i dati offerti dal server. **In sostanza**, se **connesso** il BLE client è un **centrale/master**, se **non connesso** è un **observer**.
- Un **BLE server** è un dispositivo che contiene dati e servizi a cui i client possono accedere. Il server fornisce le risposte alle richieste del client e può anche inviare notifiche ai client registrati. Il server **pubblicizza** la sua **presenza** e le sue **capacità** attraverso pacchetti di **advertising**. I client nelle vicinanze possono quindi scoprire questi pacchetti, stabilire una connessione e interagire con il server per accedere ai servizi offerti. **In sostanza**, se **connesso** il BLE server è una **periferica/slave**, se **non connesso** è un **broadcaster**.

**Esempi**:
- **BLE Client**. Gli **smartphone**, i **tablet** e i **computer** sono comunemente utilizzati come BLE client, in quanto tendono a essere i dispositivi che **richiedono** informazioni da sensori, dispositivi indossabili o altri dispositivi periferici.
- **BLE Server**. **Sensori** intelligenti o termostati agiscono come server, offrendo informazioni sullo stato della casa o accettando comandi per modificare le impostazioni, ricevuti da un'applicazione mobile client.

I **servizi BLE** sono stati nel tempo **standardizzati** e sono raggruppati per **profili**, cioè per categorie di servizi, all'interno dei quali i **tipi** di servizio hanno **comandi**, **stato** e **configurazioni** standardizzati. Questo **approccio** fa si che un **comando** inviato da un dispositivo di una marca X possa essere recepito dall'**attuatore** di una marca Y con la garanzia che venga **intepretati correttamente**. In sostanza viene garantita la piena **interoperabilità** tra dispositivi **analoghi** anche se di marca diversa.

Nel caso particolare dei **beacon**, è però prassi comune introdurre dei **servizi custom** che possono essere diversi da un costruttore all'altro a patto, però, che il loro identificativo globale, l'**UIID** sia unico e diverso da quello già assegnato ad altri servizi.

**GATT** sta per Generic Attributes e definisce una **struttura dati** gerarchica esposta ai dispositivi BLE collegati. Ciò significa che GATT definisce il modo in cui due dispositivi BLE inviano e ricevono messaggi standard. 

<img src="img/GATT-ESP32-BLE-Server-Client-Example.webp" alt="alt text" width="700">

- **Profilo**: BLE utilizza profili standardizzati per specifiche applicazioni, come il profilo di monitoraggio della salute, il profilo di controllo della luce, e molti altri. Questi profili definiscono i servizi e le caratteristiche specifiche che i dispositivi devono supportare per garantire l'interoperabilità.
- **Servizio**: raccolta di informazioni correlate al servizio, come letture dei sensori, livello della batteria, frequenza cardiaca, ecc.;
- **Caratteristica**: è dove vengono salvati i dati effettivi nella gerarchia (valore);
- **Descrittore**: metadati sui dati;
- **Proprietà**: descrivono **come** è possibile interagire con il valore caratteristico. Ad esempio: **leggere**, **scrivere**, **notificare**, **trasmettere**, **indicare**, ecc.

### **UUID**

Ogni servizio, caratteristica e descrittore ha un UUID (Universaly Unique Identifier). Un UUID è un numero univoco a 128 bit (16 byte). Per esempio:

```C++
55072829-bc9e-4c53-938a-74a6d4c78776
```

Esistono UUID abbreviati per tutti i tipi, servizi e profili specificati nel SIG [Bluetooth Special Interest Group](https://www.bluetooth.com/specifications/assigned-numbers/). 

Ma se la tua applicazione necessita di un proprio UUID, è possibile generarlo utilizzando questo sito Web di generatore di UUID [UUID generator website](https://www.uuidgenerator.net/).

In sintesi, l'UUID viene utilizzato per identificare in modo univoco le informazioni. Ad esempio, può identificare un particolare servizio fornito da un dispositivo Bluetooth.

## **Messaggi MQTT**

### **Messaggi confermati**

La **conferma** dei messaggi inviati da parte del ricevente normalmente non è necessaria nel caso dei **sensori**. Infatti, se un invio da parte di un sensore non andasse a buon fine, è inutile richiedere la ritrasmissione di un dato che comunque a breve arriva con una misura più aggiornata. 

La conferma, invece, è prevista per funzioni di **comando** o **configurazione**.  Ad esempio  nel caso di pulsanti, rilevatori di transito o allarmi in cui l'invio del messaggiò avviene sporadicamente e in maniera del tutto **asincrona** (cioè non prevedibile dal ricevitore), potrebbe essere auspicabile avere un feedback da parte del protocollo mediante un meccanismo di conferma basato su **ack**. Ma non sempre ciò è possibile.

La **conferma**, però, potrebbe pure essere gestita soltanto dal **livello applicativo** (non dal protocollo) utilizzando un **topic di feeedback** (o stato) per inviare il valore dello stato corrente subito dopo che questo viene interessato da un comando in ingresso sul dispositivo. 

### **Definizione di topic e payload**

Sovente, nella rete di distribuzione IP è presente un server col ruolo di **broker MQTT** a cui sono associati:
- su un **topic di misura o attuazione (comando)**:
    - il dispositivo **sensore** è registrato sul broker col ruolo di **publisher** perchè vuole usare questo canale di output per **inviare il comando** verso l'attuatore 
    - il dispositivo **attuatore** è registrato sul broker con il ruolo di **subscriber** perchè è interessato a ricevere, su un canale di input, eventuali comandi di attuazione (motori, cancelli). 
-  su un **topic di feedback (stato)** (dal dispositivo terminale, verso il broker), utile al server applicativo per ricevere la conferma dell'avvenuto cambio di stato dell'attuatore ma anche utile all'utente per conoscere il nuovo stato:
    - il dispositivo **attuatore** è registrato sul broker con il ruolo di **publisher** perchè intende adoperare questo canale di output per **inviare il feedback** con il proprio stato ad un **display** associato al sensore di comando.
    - il dispositivo **sensore**, ma meglio dire il dispositivo **display** associato al dispositivo sensore (un led o uno schermo), è registrato sul broker con il ruolo di **subscriber** perchè è interessato a ricevere, su un canale di input, eventuali  **feedback** sullo stato dell'attuatore per **mostrarli** all'utente. In questo caso è demandato all'utente, e non al protocollo, **decidere** se e quante volte ripetere il comando, nel caso che lo stato del dispositivo non sia ancora quello voluto.
-  su un **topic di configurazione** dove può pubblicare solamente il server applicativo mentre tutti gli altri dispositivi IoT sono dei subscriber:
    - sia i dispositivi **sensori** che i dispositivi **attuatori** si registrano sul broker con il ruolo di **subscriber** perchè intendono adoperare questo canale di **input** per ricevere **comandi di configurazione** quali, per esempio, attivazione/disattivazione, frequenza di una misura, durata dello stand by, aggiornamenti del firmware via wirelesss (modo OTA), ecc.
    - il **server applicativo** è responsabile della definizione delle impostazioni di configurazione e decide **quali** mandare e a **chi**.

**In realtà**, il topic di configurazione, pur essendo teoricamente appropriato, potrebbe anche essere incorporato nel topic di comando, magari prevedendo un livello più alto di autorizzazione rispetto ai comandi relativi alle funzioni ordinarie.

Il **canale applicativo** su cui vengono inviati i messaggi sono quindi i **topic**. Su un certo **topic** il dispositivo con il ruolo di **output** agisce come un **publisher**, mentre quello con il ruolo di **input** agisce come un **subscriber**.

l'**ID MQTT** è un identificativo che permette di individuare un dispositivo ma non è un indirizzo di livello 3, non individua la macchina host in base al suo IP, piuttosto è un indirizzo di livello applicativo noto solo ad un server centrale, cioè il broker. Un dispositivo IoT non è tenuto a conoscere l'IP di tutti gli altri dispositivi ma solamente quello del broker. Il broker soltanto sa gli indirizzi di tutti i dispositivi, conoscenza che acquisisce durante la fase di **connessione** di un client al broker, momento in cui avviene anche il recupero del'**socket remoto** del client.

Il **broker**, dal canto suo, **associa** ogni **topic** con tutti gli **ID** che sono registrati presso di esso come **subscriber**. Questa associazione è utilizzata per smistare tutti i messaggi che arrivano con un certo topic verso tutti gli ID che ad esso sono associati. Il topic diventa così un **indirizzo di gruppo**. La particolarità di questo indirizzo è che è **gerarchico** secondo una struttura ad **albero**, cioè gruppi di dispositivi possono essere suddivisi in **sottogruppi** più piccoli estendendo il nome del path con un **ulteriore prefisso**, diverso per ciascun sottogruppo. L'operazione può essere ripetuta ulteriormente in maniera **ricorsiva**.

**Ad esempio**, posso individuare le lampade della casa con il path ```luci``` e accenderle e spegnerle tutte insieme, ma posso sezionarle ulteriormente con il path ```luci/soggiorno``` con il quale accendere o spegnere solo quelle del soggiorno oppure con il path ```luci/soggiorno/piantane``` con il quale fare la stessa cosa ma solo con le piantane del soggiorno.

### **Gestione dei topic di comando**

Potremmo a questo punto inserire il comando delle luci nel topic più generale delle misure ed attuazioni che chiameremo ```comandi``` e registrare i pulsanti del soggiorno al topic ```luci/soggiorno/comandi``` come pubblisher, mentre potremmo registrare le attuazioni delle lampade allo stesso topic come subscriber. Il comando potrebbe essere il JSON  ```{"toggle":"true"}```, per cui alla fine tutto intero il path diventerebbe ```luci/soggiorno/comandi/{"toggle":"true"}```. Se volessimo selezionare un solo dispositivo sono possibili due strade alternative:
- inserire il **prefisso mqtt** del dispositivo direttamente **nel path** ```luci/soggiorno/comandi/mydevice1-98F4ABF298AD/{"toggle":"true"}```
- inserire un **id** del dispositivo **nel JSON** ```luci/soggiorno/comandi/{"deviceid":"01", "toggle":"true"}```, dove con ```01``` ci indica un indirizzo univoco solamente all'interno del sottogruppo ```luci/soggiorno```. Con questa soluzione il dispositivo deve saper gestire un secondo livello di indirizzi indipendente dal meccanismo del path dei topic.

### **Gestione dei topic di stato**

Questo canale viene utilizzato per inviare lo **stato** di un dispositivo a tutti coloro che ne sono interessati. L'interesse potrebbe nascere per più motivi:
- **Conferma** dell'avvenuta **attuazione**. Alla **ricezione** di un comando (ad esempio "on":"true"), l'**attuatore** potrebbe essere tenuto a **notificare** (in modalità PUSH), al **display** associato al sensore (o al **server di processo**) trasmittente, il proprio **stato attuale**, in modo che l'**utente** (o il server di processo) possa verificare l'effettiva **efficacia** dell'ultimo comando di attuazione.
- **Sincronizzazione PULL** del **server di processo**. Il server di processo potrebbe **prelevare** sul topic di stato, tramite un **comando di richiesta** inviato al dispositivo terminale sul topic comandi, lo **stato** degli attuatori per aggiornare un pannello generale di comando o eseguire delle statistiche o per recuperare gli input di un algoritmo che deve eseguire.
- **Sincronizzazione PULL** di un **pannello di controllo**. Un **quadro di controllo web** potrebbe **prelevare** sul topic di stato, tramite un **comando di richiesta** inviato al dispositivo terminale sul topic comandi, lo **stato** degli attuatori:
    -  una **sola volta**, all'inizio, quando la pagina è stata **caricata/ricaricata** dall'utente
    -  **periodicamente**, per essere certi di avere sempre lo **stato più aggiornato**, anche a fronte di una eventuale **disconnessione** di rete che abbia impedito la registrazione dell'ultimo feedback da parte dell'attuatore.
- **Sincronizzazione PUSH**. Lo stesso attuatore potrebbe prendere l'iniziativa di **spedire periodicamente** il proprio stato a tutti coloro che ne sono interessati (server di processo o tutti i display web che lo comandano), senza che nessuno invii richieste esplicite sul topic di comando. E' un'**alternativa PUSH** alla sincronizzazione PULL periodica.

Un esempio di **canale MQTT di stato** potrebbe essere: 
- nel caso di **identificazione univoca** del dispositivo via  **path MQTT**: ```luci/soggiorno/stato/mydevice1-98F4ABF298AD/{"state":"on"}```
- nel caso di **identificazione univoca** del dispositivo nel **payload JSON**: ```luci/soggiorno/stato/{"deviceid":"01", "state":"on"}```

### **Gestione dei topic di configurazione**

Questo canale viene utilizzato per inviare **comandi di configurazione** al dispositivo da parte del server di processo. L'interesse potrebbe nascere per più motivi:
- effettuare un aggiornamento del FW di bordo via wireless.
- impostare qualche caratteristica nella definizione delle sue funzioni come, ad esempio, comportarsi come un apricancello o come comando per luci.
- impostare la frequenza di una misura, o l'intervallo di scatto di un allarme, ecc.
- cambiare la sintassi dei JSON di payload o quella di un path MQTT

Un esempio di **canale MQTT di configurazione** per, ad esempio, impostare il periodo di pubblicazione automatica dello stato potrebbe essere: 
- nel caso di **identificazione univoca** del dispositivo via  **path MQTT**: ```luci/soggiorno/config/mydevice1-98F4ABF298AD/{"stateperiod":"3000"}```
- nel caso di **identificazione univoca** del dispositivo nel **payload JSON**: ```luci/soggiorno/config/{"deviceid":"01", "stateperiod":"3000"}```

## **Protocolli di accesso al canale**

La situazione può essere riassunta nel seguente modo:

<img src="img/bleaccess.png" alt="alt text" width="1000">

Ogni **piconet** ha due canali fisici: un canale **peer to peer** ad accesso  **probabilistico CSMA** detto  Canale di **Advertisement** ed un canale ad accesso **deterministico TDMA** detto **Canale Dati** regolato da un dispositivo **master**:
- Nel **canale dati**, dedicato alle comunicazioni **unicast**, i tempi sono stabiliti dal master.  Ogni comunicazione occupa slot temporali differenti da quelli di un'altra, e ciascuna con una propria periodicità detta **connInterval** che è un tempo  multiplo di 1.25 ms e sempre compreso tra 7.5ms e 4s.
- Nel **canale di advertising**, dedicato alle comunicazioni **broadcast**, i tempi sono stabiliti dagli advertiser ma questi possono essere in tanti a voler parlare contemporaneamente per cui l’**accesso** è a contesa e, per limitare le collisioni, si inizia a parlare dopo una sorta di backoff semicasuale ```T_advEvent = advInterval + advDelay``` dove advInterval è casuale compreso tra 0 e 10 ms mentre advDelay è un intero multiplo di 0.625 ms compreso tra 20 ms e 10.24 s (periodo di beacon). I devices che **trasmettono** pacchetti di advertising nei canali fisici sono detti **advertisers**. I devices che invece **ricevono** questi pacchetti, senza l'intenzione di aprire una connessione sono detti **scanners**.

<img src="img/blefasiconn.png" alt="alt text" width="400">

Una **connessione** può essere stabilita solo tra un dispositivo **advertiser** ed un dispositivo **initiator** e questi dispositivi diventeranno rispettivamente **slave** e **master** della piconet e, appena questa è formata, comunicheranno sul canale dati, terminando così l'**Advertising Event** ed iniziando un **Connection Event**.

**Connection Interval**: tempo tra connection events. Stabilisce un appuntamento regolare tra dispositive master e slave. Se non ci sono dati dell'applicazione da inviare o ricevere, i due dispositivi scambiano pacchetti di controllo per mantenere la connessione.
**Slave Latency**: il numero di eventi connection che lo slave può «saltare» cioè nei quali lo slave non è obbligato ad “ascoltare” il master e quindi può restare nello stato standby.
**Supervision Timeout**: tempo massimo tra due pacchetti di dati validi ricevuti prima che una connessione venga considerata "persa".


## **API di connessione** 

<img src="img/blenode.png" alt="alt text" width="800">

Il **livello GAP** dello stack BLE è il responsabile della funzione di connessione. Questo livello gestisce le modalità e le procedure di accesso del dispositivo, inclusi:
- **procedure di accesso**:
    - rilevamento del dispositivo
    - creazione del collegamento
    - interruzione del collegamento
    - avvio delle funzioni di sicurezza e configurazione del dispositivo
- **Standby**: è lo stato initiale dopo un reset.
- **Advertiser**: il dispositivo fa advetising con dati specifici che consentono a tutti i dispositivi initiator di sapere se è un dispositivo connettibile (contiene l'indirizzo del dispositivo e può contenere alcuni dati aggiuntivi come il nome del dispositivo).
- **Scanner**: quando riceve l’advertisement, il dispositivo scanner invia una scan request. L’advertiser risponde con una scan response. Questo processo si chiama discovery del dispositivo. Il dispositivo scanner è a conoscenza del dispositivo advertiser e può avviare una connessione con esso.
- **Initiator**: all'avvio, deve specificare l’indirizzo del dispositivo peer a cui vuole connettersi. Se viene ricevuto un advertisement corrispondente a quell’indirizzo, il dispositivo invia quindi una richiesta per stabilire una connessione con il dispositivo advertiser definendo i parametri della connessione (Connection Interval, Slave Latency, Supervision Timeout).
- **Slave/Master**: quando viene stabilita una connessione, il dispositivo funziona come uno slave se è l’advertiser, come un master se è l’initiator.

Si può accedere al livello GAP tramite **chiamate dirette** o tramite la API **GAPRole Task**. E’ consigliato utilizzare il metodo **GAPRole Task** anziché le **chiamate dirette** quando possibile.

La **configurazione diretta** del livello GAP descrive le funzioni e i parametri che non sono gestiti o configurati tramite le API GAPRole Task e che pertanto devono essere modificati direttamente tramite il livello GAP. GAPRole Task è un'attività separata che scarica l'applicazione della gestione della maggior parte delle funzionalità del livello GAP. Questa attività è abilitata e configurata dall'applicazione durante l'inizializzazione. Sulla base di questa configurazione, molti eventi dello stack BLE vengono gestiti direttamente dal GAPRole Task e non vengono mai passati all'applicazione. Esistono funzioni di callback che l'applicazione può registrare con l'attività GAPRole in modo che l'attività dell'applicazione possa essere notificata per determinati eventi e procedere di conseguenza.

<img src="img/blestate.png" alt="alt text" width="300">

- **Standby** è lo stato di default in cui non ci sono scambi di pacchetti.
- Un dispositivo in stato advertising può avviare una ricerca tramite l’invio di pacchetti che saranno ricevuti da altri dispositivi in stato initiating o scanning. 
    - Dispositivo in stato advertising: è disponibile ad effettuare una connessione e quindi passare allo stato connection prendendo il ruolo di slaves 
    - Dispositivo in stato initiating: passerà allo stato connection con il ruolo di master della piconet.
- Dispositivo in stato **initiating**: passerà allo stato connection con il ruolo di master della piconet. I dispositivi in stato di advertising, scanning e initiating utilizzano i cosiddetti canali advertising per la loro comunicazione mentre i dispositivi in stato connection utilizzano i canali data.
- La possibilità di passare dallo stato di connessione attiva (**connection**) allo stato standby, permette allo slave di risparmiare energia durante gli intervalli di tempo tra una trasmissione e quella successiva. 
Dalla figura si vede che  questo passaggio possa avvenire però solo attraverso gli stati advertising e initiating; cioè la fase di connessione deve essere sempre preceduta da una fase di ricerca.

## **Modulazione**

BLE adopera una forma di FDM in cui trasmettitore e ricevitore non usano in una connessione sempre la stessa frequenza ma saltano, ad istanti prefissati, lungo 37 canali secondo uno schema reso noto ad entrambi in fase di setup. Ogni connessione avrà uno schema di salti che, istante per istante, non si sovrappone a quello delle altre connessioni.
In una connessione dati, viene utilizzato un algoritmo di salto di frequenza per scorrere i 37 canali di dati: 

```C++
fn+1=(fn+hop) mod 37
```

Dove fn+1 è la frequenza (canale) da utilizzare al prossimo evento di connessione e hop è un valore che può variare da 5-16 e viene impostato al momento del setup della connessione. Il meccanismo di salto è dinamico e può variare per adattarsi a sopraggiunte condizioni di interferenza con altri dispositivi.

Supponiamo, ad esempio, che un dispositivo BLE si trovi a coesistere con reti Wi-Fi sui canali 1, 6 e 11. Il dispositivo BLE contrassegna i canali 0-8, 11-20 e 24-32 come canali non buoni. 
Ciò significa che mentre i due dispositivi comunicano, rimapperanno i salti in maniera tale da evitare i canali con interferenza [Link FHSS](accessoradio.md#fhss).

<img src="img/blefhss.png" alt="alt text" width="600">

**Sitografia**:

- (https://amslaurea.unibo.it/6599/1/fantini_enrico_tesi.pdf
- https://delgenio.jimdofree.com/app/download/6302381463/Tesi+F.DelGenio+Definitiva+30-11-2015.pdf?t=1466548571
- https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8355905
- https://www.silabs.com/documents/public/user-guides/ug103-14-fundamentals-ble.pdf
- https://www.oreilly.com/library/view/getting-started-with/9781491900550/ch01.html
- https://www.comarch.it/iot-ecosystem/asset-tracking/
- https://microchipdeveloper.com/wireless:ble-link-layer-connections
- https://microchipdeveloper.com/wireless:ble-link-layer-connections#toc0
- https://os.mbed.com/blog/entry/BLE-Beacons-URIBeacon-AltBeacons-iBeacon/
- https://nl.mathworks.com/help/comm/examples/modeling-of-ble-devices-with-heart-rate-profile.html
- https://www.google.com/imgres?imgurl=https%3A%2F%2Fwww.researchgate.net%2Fprofile%2FJonathan_Ruiz-de-Garibay%2Fpublication%2F231212227%2Ffigure%2Ftbl1%2FAS%3A669582813581312%401536652391081%2FRFID-vs-NFC-vs-Bluetooth.png&imgrefurl=https%3A%2F%2Fwww.researchgate.net%2Ffigure%2FRFID-vs-NFC-vs-Bluetooth_tbl1_231212227&tbnid=lhIjM06RjY13PM&vet=12ahUKEwiul4etl7zpAhURMuwKHSRcCIgQMygFegUIARDcAQ..i&docid=1LDoXFeY8V2bKM&w=850&h=267&q=ble%20vs%20rfid&safe=active&ved=2ahUKEwiul4etl7zpAhURMuwKHSRcCIgQMygFegUIARDcAQ
- https://www.assetinfinity.com/blog/asset-tracking-technologies
- https://nl.mathworks.com/help/comm/ug/what-is-bluetooth.html
- https://devzone.nordicsemi.com/nordic/short-range-guides/b/bluetooth-low-energy/posts/ble-characteristics-a-beginners-tutorial
- https://stackoverflow.com/questions/23735307/understanding-the-gatt-protocol-in-ble
- http://software-dl.ti.com/lprf/simplelink_cc2640r2_latest/docs/blestack/ble_user_guide/html/ble-stack-3.x/gaprole.html
- https://apagiaro.it/bluetooth-low-energy-presentation/
- http://www.lucadentella.it/2018/02/09/esp32-31-ble-gap/
- https://www.todaysoftmag.com/article/2225/bluetooth-beacon-tracking-it-works-both-ways
- https://github.com/KMuthumala/ESP32-BLE-Beaconing
- https://randomnerdtutorials.com/esp32-ble-server-client/

>[Torna a reti di sensori](sensornetworkshort.md#classificazione-delle-tecnologie-wsn-in-base-a-velocità-e-copertura)
