>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)


# **Stack ISO/OSI**

## **Architettura**

L'**ISO/OSI** ha catalogato e organizzato in una **architettura a 7 strati** tutte le funzioni di rete in base alle **categorie di problemi** di cui esse si occupano. Le **funzioni**, normalmente **distribuite** tra più nodi, si basano su **scambi di messaggi** e quindi sono gestite da **protocolli di comunicazione**. 

L'**architettura** è normalmente rappresentata sotto forma di **stack** (pila) e, per ogni elemento dello stack, identifica una **categoria di protocolli** che deve risolvere un determinato **tipo** di problemi della comunicazione in rete. 

<img src="img/pilaosi.png" alt="alt text" width="900">

Esiste anche un'architettura a **5 strati** che **non prevede** le funzioni di **sessione** e **presentazione**. Se delle funzioni di comunicazione non sono incluse nell’architettura però non vuol dire che queste siano inutili ma, semplicemente, che sono svolte altrove. La figura suggerisce che le funzioni degli **strati mancanti** siano tutte **delegate** all'**applicazione**.

L'architettura è **modulare**. Uno strato può essere sostituito senza che quelli sopra di lui se ne accorgano. Ciò permette una grande **flessibilità**. Questa flessibilità può essere utilizzata per **adattare** un protocollo di livello superiore a **vari mezzi fisici** o per ottenere su uno stesso mezzo fisico un ventaglio di diverse **qualità del servizio**. 

### **Interoperabilità**

**Obiettivo finale** è garantire l'**interoperabilità universale tra i dispositivi** da collegare, a prescindere dalle possibili differenze che possono intercorrere rispetto a marca, modello e tecnologie adoperate. 

Avendo l'**interoperabilità** spinta alla base, l'**architettura ISO/OSI** è naturalmente predisposta per un altro obiettivo ambizioso, realizzare una rete **geografica**, cioè una rete dati con estensione davvero molto ampia, detta in gergo **WAN** (Wide Area Network). Una tipica **WAN** deve essere operativa in **ambito globale**, perciò deve essere tale da permettere di individuare una **macchina** (e un **servizio**) in rete ovunque nel mondo.

### **Nodi**

L'architettura ISO/OSI modella le funzioni di una tipica rete magliata composta da nodi intermedi o **IS** (Intermediate Systems) e da nodi terminale o **ES** (End Systems). 
- i **nodi IS** sono i nodi di transito **interni** alla rete, quelli che forniscono il servizio di consegna dei messaggi utilizzato dagli ES per comunicare tra di loro. Svolgono funzioni di **smistamento** di pacchetti al fine di selezionare il percorso ottimale tra un nodo ES **mittente** e un nodo ES **destinatario**, tra i tanti che è possibile scegliere all'interno di una rete **parzialmente magliata**.
     - I nodi IS sono per ISO/OSI solamente i **router**.
     - Switch e HUB (cavo o wireless), pur offrendo anch'essi servizi di smistamento analoghi a quelli di un router, per ISO/OSI **non esistono** perchè i loro servizi, essendo **incorporati** all'interno di un collegamento di linea, sono **trasparenti**, cioè nascosti allo stack OSI.
     - I nodi IS devono implementare obbligatoriamente solo i **primi 3 livelli** della pila ISO/OSI (fisico, linea e rete).
- i **nodi ES** sono i nodi di sorgente e destinazione dei messaggi della rete, sono nodi **esterni** alla rete e normalmente sono di proprietà degli utenti della rete.
     - Devono implementare obbligaroriamente **tutti i livelli** della pila ISO/OSI (dal fisico fino al livello di applicazione)
     - Sono i nodi che usufruiscono dei servizi di consegna offerti dalle **risorse** della rete che tipicamente sono:
          - canali di comunicazione fisici (cavi metallici, fibra, mezzo radio)
          - nodi di smistamento (detti anche di commutazione), tipicamente i **router**
          - protocolli di comunicazione
          - servizi di gestione e controllo


### **La rete di reti Internet**

La realizzazione tecnologica di una **WAN mondiale** non è stata inizialmente ottenuta con un'**unica rete WAN** ma piuttosto, grazie ad una soluzione tecnica di compromesso, con una **federazione** di reti **WAN** e **LAN** eterogenee, la **Internet** propriamente detta.

Internet, nata nel ristretto ambiente della ricerca negli anni 70', ha poi riscosso il successo commerciale a cui siamo abituati solo all'indomani dell'invenzione del **WWW** (World Wide Web), avvenuta nel 1990. 

Lo **stack TCP/IP**, una particolare e fortunata implementazione dei livelli 4 e 3 della pila OSI che stanno alla base della rete Internet, ha consentito di tenere insieme, non tanto un'**unica WAN**, quanto una **inter-network** di numerose **reti eterogenee** regionali. 

Per ottenere ciò, Internet ha utilizzato anche a livello 2 quei servizi di inoltro End To End che tipicamento si trovano al livello 3 della pila ISO/OSI. Di fatto, ha utilizzato, a livello dello strato di linea (data link ISO/OSI), i servizi di smistamento dei dati offerti da reti realizzate con tecnologie diverse dal TCP/IP.

<img src="img/Internet.png" alt="alt text" width="700">

In sostanza, **reti proprietarie**, con propri schemi di indirizzamento (spesso non IP o con IP privati), possono essere adoperate per realizzare dei **link logici** tra router IP di una **Internet mondiale** composta da nodi ed host aventi indirizzi **IP pubblici**. 

Una **Internet IP** è quindi una rete di **router IP** collegati tra loro da **link punto punto** che possono essere **fisici** oppure **virtuali**:
- ogni **link fisico** è tipicamente basato su conduttori metallici, fibre ottiche o mezzi radio.
- Ogni **link virtuale** è spesso materialmente realizzato utilizzando, a livello 2 (**Data Link**) della pila ISO/OSI, i servizi applicativi di **livello 7** di una **rete regionale** con tecnologia arbitraria (MPLS, ATM, Ethernet).

Col tempo il quadro è via via mutato. Nella parte **core** di Internet, IP è diventata la tecnologia dominante e, nel suo complesso, la rete **Internet** è sempre più simile ad un agglomerato di **sottoreti IP indipendenti** e annidate l'una dentro l'altra dette **AS** (Autonomous Systems). 

Gli **AS** sono collegati da router di frontiera detti **border gateway**. La **dimensione** complessiva delle sottoreti scala da **continentale** a **nazionale**, e da nazionale a **regionale**. 

Le **reti regionali**, le MAN (Metropolitan Area Network), sono comunque ancora realizzate con tecnologie abbastanza varie e adoperano un **inoltro** (smistamento) dei pacchetti che non sempre è basato solamente su IP, sono molto comuni tecnologie alternative quali MPLS e Frame Relay.



- Per dettagli sull'architettura a strati vedi [architettura a strati](archstrati.md)

- Per delle domande riassuntive vedi [domande osi](domandeosi.md)

## **Modello ISO/OSI**

E’ composto da 7 livelli ordinati secondo una pila (stack) di protocolli dove gli **elementi attivi** di ogni livello, cioè quelli che svolgono le funzioni, sono le **entità**.

Entità appartenenti allo **stesso livello** N, su sistemi diversi, sono dette **entità pari** (peer entities) e la comunicazione fra due peer ad un certo livello avviene: 
- per mezzo di un insieme di regole e convenzioni chiamate **protocollo del livello N**
- tramite unità informative (pacchetti) dette **Protocol Data Unit** di livello N (N-PDU) 

La pila di protocolli è un “contenitore” di funzioni di rete che sta all’interno di un nodo di rete (IS o ES). In **ogni nodo**, esiste una pila di protocolli per ogni **scheda di rete** (interfaccia esterna) da esso posseduta.


| Livello | Nome                  |       Funzioni svolte                        |      Esempi di protocollo        |
|---------|-----------------------|-----------------------------|-----------------------------|
| 7       | Applicazione          |  Applicazione **Client/Server** che virtualizza una risorsa **remota** e **condivisa** facendola apparire come **locale** e **dedicata**, definisce il **tipo di utenti** e il **tipo di sicurezza** con cui dialogano| HTTP, DNS, DHCP, FTP, SFTP, scp, rsync, rclone |
| 6       | Presentazione         |  Traduce **documenti**, **flussi di dati**, **record**, **caratteri** da un formato all'altro (ad es. XML piuttosto che JSON oppure mp3 al posto di mp4), realizza la **compressione** dei dati e la loro **cifratura**, realizza la **serializzazione** degli oggetti (ad esempio struct)| MIME, XDR, HTML, XML, SSL, TLS, zip |
| 5       | Sessione              |   Apre, gestisce e termina conversazioni e scambio di dati (sessioni) tra due applicazioni stabilendone le **fasi**, gestisce il **tipo di comunicazione** (half duplex, full duplex) | SMB, NetBIOS  |                                                                                                   |
| 4       | Trasporto             | **Segmentazione** di ciascun pacchetto in più segmenti, **multiplazione** di più connessioni di livello 4 (uniscono processi remoti) in una stessa connessione di livello 3 (unisce due host remoti), **controllo di errore** End to End (con ritrasmissione), **controllo di flusso** End to End, **riordino** dei pacchetti fuori sequenza, **controllo di congestione** |   TCP, UDP      |                                                                                                  |
| 3       | Rete                  | **Indirizzamento**, cioè individuazione del luogo fisico in cui si trova l'host destinatario, esegue la **scelta** del percorso migliore per raggiungerlo (**routing**), esecuzione dell'**inoltro fisico** (forwarding) dei pacchetti, da una porta di ingresso a quella di uscita che, per ogni router incontrato lungo il percorso, stanno nel percorso scelto.  |   IP    |                                                                  |
| 2       | Collegamento dati     | **Segmenta** i messaggi lunghi aggiungendo loro l'indirizzo del mittente, esegue la **multiplazione** di più connessioni di livello 3 (uniscono coppie di host remoti) sulla stessa connsessione di livello 2 (unisce due nodi adiacenti), esegue il **controllo di errore** (senza ritrasmissione), esegue il **controllo di flusso** (velocità del mittente) | MAC/LLC, HDLC, PPP, EAP |
| 1       | Fisico                | Esegue la **codifica/decodifica** dei bit, cioè traduce i bit in segnali elettrici in **trasmissione** mentre traduce i segnali elettrici in bit **in ricezione**. Definisce gli **standard** meccanici ed elettrici dei canali.| Ethernet, WiFI, BLE, Zigbee, LoRa|

Si noti il ruolo della **segmentazione** ai livelli 2 e 4 che, frammentando lunghi stream di dati in unità informative più piccole, permette la **multiplazione** del canale, cioè la **condivisione** dello stesso tra pacchetti di **sorgenti diverse** che, a livello superiore, posseggono **indirizzi diversi**:
- a **livello 2**, il canale L2 diretto tra due **IS adiacenti** può essere condiviso da pacchetti L3 aventi indirizzi differenti. Permette sostanzialmente la **condivisione** dei **link** tra i vari router tra **host differenti** (individuati da un indirizzo IP pubblico).
- a **livello 4**, il canale virtuale L3 diretto tra due **ES** può essere condiviso dai segmenti L4 aventi indirizzi differenti. Permette sostanzialmente la **condivisione** dei **link virtuali End to End** tra i vari host tra **processi differenti** (individuati da un numero di porta).

## **Multiplazione**

La **tecnologia** con cui si realizza la **multiplazione** nelle moderne reti a pacchetto è il **TDM statistico** per via della sua elevata efficienza ed è, nella **visione OSI**, implementato e gestito dai **router** (Per dettagli vedi [TDM statistico](tdmstatistico.md)). 

La **multiplazione**, secondo il modello OSI, è una **funzione di livello 2** che permette a **pacchetti di livello 3** (pacchetti IP), aventi indirizzi di destinazione diversi, di **condividere** uno **stesso link** di transito di livello 2 che collega due **router adiacenti**.

Però esistono anche analoghe multiplazioni che, essendo incorporate come **proprietà** del canale **fisico** o del canale **datalink**, non sono prese in considerazione dal modello ISO/OSI:

- la funzione di multiplazione svolta dai protocolli di accesso multiplo **sui mezzi a BUS**. Viene svolta in HW a **livello 1** (fisico). Permette a **pacchetti di livello 2** (trame MAC), aventi indirizzi di destinazione diversi, di **condividere** uno **stesso mezzo a BUS** di transito di livello 1 che collega **tutti gli host**.

- la funzione di multiplazione svolta dagli **switch**. Viene svolta in HW a **livello 1** (fisico). Permette a **pacchetti di livello 2** (trame MAC), aventi indirizzi di destinazione diversi, di **condividere** uno uno **stesso link** di transito di livello 1 che collega due **switch adiacenti**.


<img src="img/zigbee-osi.png" alt="alt text" width="500">

## **Middleware**

Se delle funzioni di comunicazione non sono incluse nell’architettura, allora non vuol dire che queste siano inutili ma, semplicemente, che non è da questa architettura che esse verranno svolte.

Sarà responsabilità di «qualcun altro» svolgerle e tipicamente, sessione e presentazione in un nodo sono in alternativa realizzate dalla:
- Applicazione, oppure da un
- Middleware, cioè librerie di funzioni di terze parti

<img src="img/osicompleta.png" alt="alt text" width="1000">

Le funzioni di **trasporto** (L4) e di **rete** (L3) sono svolte dal **SO** e sono accessibili dai programmi tramite delle API standard (primitive socket)

Le funzioni di **linea** (L2) sono svolte dai **driver** delle schede di rete

## **PDU**

<img src="img/pdu.png" alt="alt text" width="400">

L’informazione da trasferire è organizzata in **unità dati** (PDU o Protocol Data Unit) che comprendono due **componenti**:
- informazione di utente (**SDU** o Service Data Unit):
    - possono essere forniti ad una entità direttamente dall'applicazione oppure dall'entità di livello immediatamente superiore.
    - L’obiettivo di un protocollo di livello N è proprio inviare le sue SDU lungo il canale con una prefissata qualità di servizio.
    - Le SDU sono la «**stiva**» all’interno della quale **allocare** le **PDU** provenienti dal **livello superiore**. 
    - Vengono chiamate anche **payload** (carico utile)
- informazione di controllo (**PCI** o Protocol Control Information) dette anche **header** (intestazione):
    - Sono informazioni di **servizio** usate dalle **entità** pari per **portare avanti** il **protocollo** attivo sul **canale virtuale** che le collega direttamente.
    - Il **protocollo** sull'entità trasmittente **aggiunge** l'**intestazione** ai dati contenuti nella SDU (payload), lo stesso protocollo, stavolta attivo sull'entità ricevente, la **rimuove** restituendo esattamente i **dati** che erano stati consegnati in partenza su quel **canale virtuale**.
    - Sono tipicamente **indirizzo** di **sorgente** e di **destinazione** più altre informazioni necessarie per **realizzare le funzioni** di quel livello (contatori, numeri di sequenza, checksum, ecc.)


Le  **PDU** in genere sono di **due tipi**:
- **Dati**: sono composte da intestazione (PCI) più campo dati (payload), sono le più generiche e svolgono entrambe le funzioni di:
    - trasporto dei **messaggi dati** da scambiare tra entità corrispondenti in un canale virtuale
    - trasporto dei **messaggi di servizio** che, pur non essendo utili per l'utente del servizio stesso, sono necessari per il suo corretto compimento, dato che il **protocollo** in uso sul canale li prevede.
    
- **Controllo**: rispetto alle PDU normali, sono **privi del campo dati** cioè non possiedono il campo SDU (o payload), ovvero, sono composti solamente di una **intestazione** (PCI) che contiene **campi di controllo** che:
    - **non** trasportano contenuti informativi utili per gli utenti del canale (le entità corrispondenti).
    - trasportano solamente **messaggi di servizio** che, pur non essendo utili per l'utente del servizio stesso, sono **necessari** per lo svolgimento del **protocollo** in uso su quel **canale virtuale**.
    
La **risoluzione dei problemi** di rete è sempre **distribuita**, nel senso che non può avvenire senza lo **scambio di messaggi di servizio** che servono a coordinare il lavoro tra le **entità pari**, cioè quelle dello **stesso livello**.

## **Canali virtuali**

### **Definizione**

Un **canale virtuale** è un **canale logico** che **emula** un **collegamento diretto** tra **entità pari**, percepito da esse come un percorso che trasporta **solamente** le **PDU** di quel livello e che segue esclusivamente le regole del **protocollo** in uso a quel livello. I **canali virtuali** vengono **creati** grazie alla tecnica dell'**imbustamento multiplo**, e rappresentano il **punto di vista** che un **generico livello** ha dell'unico canale effettivamente esistente, cioè il **canale reale** di livello 1 (fisico). 

### **Tre tipi di astrazione**

Il **punto di vista** di una entità di un certo livello è pur sempre una interpretazione **parziale**, perchè non si basa su quello che una comunicazione realmente è **globalmente** (a livello fisico e lungo i vari nodi), ma per come, ad un **certo livello**, essa appare,  osservando il modo (protocollo) con cui si **comporta** lo scambio dei **dati** all'interno di quel **livello**. Infatti, in un generico **livello N**, vengono realizzate almeno tre **tipi di virtualizzazione**:
1. **Funzionale**. Alcune **proprietà** e alcune **funzioni** del livello corrente vengono date **per scontate**, come se questo fosse naturalmente in grado di fornirle, non percependo affatto che sono il frutto della **collaborazione** con  tutti gli **altri livelli** sottostanti.
2. **Topologica**. Al livello corrente lo **schema** della rete potrebbe apparire **semplificato**, fino ad essere stato **riassunto** in un **unico link**. In particolare, ciò che al **livello N** generico potrebbe apparire come un **collegamento virtuale diretto** tra due host, potrebbe essere, ai livelli sottostanti, fisicamente **spezzato** in più collegamenti tra **nodi intermedi** di **transito**.
3. **Strutturale**. Il collegamento tra **due strati omologhi**, cioè le **entità** di due nodi adiacenti, potrebbe apparire **diretto** lungo un canale **orizzontale dedicato** solo a loro. La comunicazione, invece, è sempre più articolata e comprende la comunicazione **verticale** tra **uno strato e l'altro**, fino al **livello fisico**.

### **Proprietà di un canale virtuale**

Al **livello N**, al fine di realizzare i **servizi** per il livello superiore **N+1**:
- La **comunicazione** tra **entità pari** apparentemente si svolge come se avvenisse lungo un **canale diretto** che le unisce in **orizzontale** detto **canale virtuale** di **livello N** perchè avviene: 
    - **scambiandosi** PDU di livello N **(N-PDU**)
    - in base ad un **protocollo** di **livello N** 
- Ciascun canale virtuale aggiunge **nuovi servizi** al canale virtuale sottostante. I **servizi** di un livello N, possono essere richiamati da moduli SW mediante delle **API** (Application Programming Interface) di programmazione standardizzate che, in quanto tali, vengono dette **primitive di servizio** di livello N. tutte le implementazioni di un protocollo di quel livello devono impegnarsi a realizzarle con quella segnatura (insieme di nome + parametri).

### **Tipi di canale virtuale**

**Riassumendo**, i **canali virtuali non esistono fisicamente** ma sono **ugualmente reali** perchè, dal punto di vista dei **messaggi scambiati**, le **entità** pari (peer entity), cioè gli interlocutori corrispondenti nei vari livelli, si **comportano** come se essi ci fossero davvero e attraverso questi **effettivamente dialogassero**. Sono una **visione semplificata** con cui, ad un **livello superiore**, si possono **osservare** le funzioni svolte e i messaggi scambiati dai livelli inferiori. Semplificata, ma comunque aderente alla realtà, nel senso che è una maniera in grado di spiegarne compiutamente il comportamento.

<img src="isvses.png" alt="alt text" width="600">

**I canali virtuali** si dividono in:
- link **end to end**, chiamati così perché collegano livelli che sono presenti **soltanto** negli **host** cioè i **dispositivi terminali**, quelli su cui si **interfaccia l'utente**. I nodi **vicini** in questo collegamento virtuale sono i nodi **fisicamente più distanti**, in quanto la pila OSI realizza, a livello di servizio, un collegamento **virtuale **dedicato** e diretto** proprio tra nodi terminali.
- **link IS-IS** (collegamento tra  Intermediate systems), stanno più in basso (sui **primi 3** livelli) e sono chiamati così perché collegano tra loro i **dispositivi di rete**, cioè quelli che creano la rete. Anche gli host posseggono questi livelli e pertanto sono, a tutti gli effetti, dispositivi di rete anch'essi (anche se privi di alcune funzioni importanti). I nodi **vicini** in questo collegamento virtuale soni i nodi della rete **fisicamente più prossimi**, in quanto la pila OSI realizza, a livello di servizio, un collegamento **virtuale **dedicato** e diretto** tra nodi vicini.

In figura la lettera H sta per **header** cioè intestazione (ad es. AH=Application header).

<img src="img/iso-osi.png" alt="alt text" width="1100">

## **Imbustamento multiplo**

È la **tecnica** utilizzata per realizzare più **canali virtuali** su l’unico canale fisico (**canale reale**) che collega due macchine (**hosts**). 

Il canale virtuale è solamente **emulato** perchè nelle **reti a pacchetto**, in fase di **trasmissione**, si usa una tecnica, detta **imbustamento multiplo**, con la quale si **incapsulano** i messaggi di un livello generico N, le **N-PDU**, all'interno del campo dati, del livello **immediatamente inferiore**, detto **(N-1)-SDU**, lungo un punto di accesso verticale detto **SAP** (Service Access Port). Questo processo di **incapsulamento** parte dal livello applicativo ed è **ripetuto** su tutti i livelli, escluso il fisico. 

<img src="img/imbustamento.png" alt="alt text" width="1100">

L'**imbustamento multiplo** è realizzato in trasmissione, lungo il **canale reale** che scende in verticale attraverso un **SAP**:
- I messaggi inviati sul **canale virtuale** di **livello N** sono detti **N-PDU** (Protocol Data Unit) 
- I **SAP** sono le **porte** delle **interfacce** attraverso le quali si realizza il **canale reale** di livello N.
- Le **N-PDU** inviate sul canale reale da un protocollo di livello N, diventano, attraversata la N-SAP, **(N-1)-SDU** (imbustamento multiplo), cioè il **payload** del protocollo del **livello inferiore**. Il livello inferiore non è in grado di interpretare le PCI del livello superiore e pertanto le tratta alla stregua di normali dati, cioè **(N-1)-SDU**, da trasferire secondo le regole del proprio protocollo.
- Ogni livello **aggiunge** alle **SDU**, alcune informazioni di controllo, dette **header** (intestazione) o anche (**PCI**: Protocol Control Information) 
- Gli **header** sono **diversi** per ogni livello e contengono le **informazioni di servizio** che consentono al **protocollo** di quel livello di funzionare

<img src="img/osipdusdu.png" alt="alt text" width="500">

### **Overhead**

Poichè ogni livello **aggiunge** sempre alle SDU un proprio campo **header**, accade che il **livello fisico**, a seguito di tutti gli annidamenti, produce il **messaggio più lungo** perché è composto dal messaggio utente a dalla **somma di tutti gli header** aggiunti nei singoli livelli. Questa somma è detta, in gergo tecnico, **overhead**. Il rapporto tra la lunghezza del campo dati originale e l'overhead complessivo al livelllo fisico rappresenta una stima dell'**efficienza** del protocollo nello spedire i messaggi che, in generale, è **tanto maggiore**:
- quanto l'**overhead è minore**
- quanto il **messaggio dati** di partenza è **più grande**

### **Sbustamento multiplo**

All'**imbustamento multiplo** in **trasmissione** corrisponde lo **sbustamento multiplo in ricezione**, un'operazione analoga ma **inversa**, dove tutti i messaggi ricevuti a **livello fisico**, incapsulati uno dentro l'altro, vengono, salendo da un livello all'altro, **sbustati** rimuovendo l'intestazione del livello immediatamente inferiore. 

Arrivati al **livello applicativo**, come risultato, si ottiene il **messaggio utente** privo di qualunque intestazione. Quindi, l'**ordine** di **inserimento** delle **intestazioni** in trasmissione è esattamente l'inverso dell'ordine di **rimozione** delle stesse in ricezione, secondo una tipica **politica LIFO** (Last In - First Out).

### **In sintesi**

**L'imbustamento multiplo** permette la creazione dei cosiddetti **canali virtuali**, cioè dei collegamenti **apparenti** e **diretti** tra **strati corrispondenti** (entità) di dispositivi **remoti**. 

Un nodo può essere **logicamente** suddiviso in una serie di strati detti **"entità"**, ciascuna ha un suo proprio **ruolo** nella comunicazione, caratterizzato da specifici **compiti** e **funzioni** che hanno l'obiettivo della **consegna del payload** con una certa **QoS** (Quality Of Service). Queste **funzioni** hanno la particolarità di dover essere svolte in **maniera distribuita** nei vari nodi, mediante lo **scambio** di opportuni **messaggi di controllo**. Ogni entità lascia, quindi, una **traccia** nel messaggio finale, dato che in esso è sempre possibile isolare sia il messaggio utente trasmesso (**payload**) che le **informazioni di controllo** necessarie per realizzare le funzioni di quell'entità (**header**). 

Il meccanismo dell'imbustamento/sbustamento fa sì che, al momento in cui un pacchetto raggiunge un certo **livello N**, questo esibisca come **header più esterno** sempre un **N-PCI**, l'unico header che il protocollo di **livello N** è in grado di **interpretare**. Inoltre, il livello N sa che tutto quello che viene dopo un N-PCI è la **N-SDU (payload)** del proprio livello e che questa va **consegnata** al livello superiore (o a quello inferiore) senza modifica alcuna. 

In pratica, un livello, **ad esempio il 3**, considera come proprio payload (3-SDU) il **payload utente** (l'unico vero payload, cioè il **7-SDU**) più tutti gli **header** accumulati negli imbustamenti dal livello 7 al livello 3 (**7-6-5-4-PCI**).

## **Canale reale**

Il **canale reale** è il **mezzo trasmissivo** che unisce il **livello fisico** dei due interlocutori. Si sviluppa:
1. in **verticale** lungo le interfacce tra i vari strati **a scendere**, in **trasmissione**
2. in **orizzontale** lungo il **mezzo trasmissivo**, codificato sotto forma di segnali
3. in verticale **a salire**, in **ricezione**.
  
<img src="img/The 7 Layers of OSI.png" alt="alt text" width="1000">

I messaggi dei vari livelli, **incapsulati** uno dentro l'altro, vengono trasmessi in un **blocco unico** lungo il **canale reale**:
- Fino a che non viene raggiunto il livello 1 (canale fisico), nessun dato è realmente trasferito direttamente dal livello N del Tx al livello N del Rx. L’unico canale reale è quello fisico!
- le **entità pari** in realtà comunicano in **verticale**, trasferendo i dati **attraverso i SAP**, cioè le **porte** sulle interfacce di separazione tra un livello e l'altro, prima a scendere in trasmissione e poi a salire in ricezione. L'unica comunicazione **orizzontale** è lungo il **canale fisico** realizzato dal **mezzo trasmissivo** (fibra ottica, conduttore metallico, mezzo radio).
- in un router (IS), esiste uno **stack separato** per **ogni porta**, di cui esso è dotato, per cui pacchetti vengono:
     - **sbustati** sulla porta di **ingresso**, da cui viaggiano in **verticale salendo** dal livello 1 fino al livello 3, dove vengono recuperati i **pacchetti IP**
     - i **pacchetti IP** vengono **smistati** dalla porta di ingresso su una delle porte di uscita (processo di forwarding o inoltro), viaggiando in **orizzontale**, attraversando quindi **stack diversi** dello **stesso livello**
     - **imbustati** sulla porta di **uscita**, da cui viaggiano in **verticale scendendo** dal livello 3 fino al livello 1, dove vengono spediti sotto forma di **bit** sul canale fisico.
- tutti gli **involucri**, cioè le **intestazioni** di livello inferiore al 3 (MAC), vengono scartati in fase di **sbustamento** e poi vengono ricostruiti d'accapo, con **nuovi indirizzi**, in fase di **reimbustamento**.
- i pacchetti di **livello 3**, cioè i **pacchetti IP**, sono per i router come dei payload (non avendo i router livelli superiori al 3), per cui vengono sempre da questi **inoltrati senza modifica** alcuna. Rimangono immutati sia i payload (segmento di livello 4) che le intestazioni (indirizzi sorgente e destinazione IP).

### **In sintesi**
- la comunicazione **reale** tra le **entità pari** dei livelli è **indiretta** e avviene:
    - In senso **verticale** attraverso le interfacce della pila di protocolli
    - In senso **orizzontale** lungo il canale fisico al livello 1
- La comunicazione **virtuale** tra le **entità pari** dei livelli è **diretta** e avviene:
    - in senso orizzontale tra entità adiacenti dello stesso livello.
    - lungo un **canale virtuale** per ciascun livello.
    - utilizzando un **protocollo** e una **PDU** dedicati per ciascun livello.

Per comodità, all'**interno dei dispositivi**, ogni **messaggio applicativo**  può essere visto **destrutturato** in una serie di messaggi **separati** lungo i 7 **canali virtuali** dei singoli livelli.  Il **messaggio parziale**, trasmesso e ricevuto lungo un livello, è inviato con il **formato** e con le **regole** di trasmissione (**protocollo**) proprie di quel livello. 
 
**In realtà**, i messaggi che sono stati **generati** e **trasmessi separatamente** dai 7 livelli lungo i 7 **canali virtuali**, diventano un **unico messaggio matrioska** di 7 messaggi, **annidati** uno dentro l'altro, una volta che sono trasmessi lungo il **canale reale**. 

## **Astrazione**

Vale il principio di **astrazione**, che vuol dire che, man mano che **si sale**, ogni **livello** ha una visione del problema iniziale via via più **semplice e generale** che si occupa sempre meno dei **dettagli fisici** e, viceversa, sempre di più di problematiche concettuali legate alla **natura** dei messaggi, del loro **significato** più che del **modo** con cui essi vengono **scambiati**. La **soluzione astratta** di un problema ha il decisivo **vantaggio** di poter essere utilizzata senza modifiche anche all'interno di **contesti diversi**, lì dove i **dettagli** per la sua applicazione vengono declinati in **maniera differente**. 

In linea generale, grossomodo accade che la soluzione astratta viene **adattata**, mentre quella di dettaglio viene **sostituita**.

Ogni funzione di un livello **"astrae"**, cioè riassume in un'unica funzione di alto livello **generica**, molte funzioni dei livelli sottostanti legate a dettagli HW **specifici**. Per cui, la particolarità di questi collegamenti è quella di realizzare una **virtualizzazione della rete** e dei suoi dispositivi che, salendo lungo la pila OSI, si fa sempre più spinta. Ogni nuova astrazione **nasconde** i dettagli implementativi degli strati inferiori e rende **ininfluenti** le loro **differenze** per le funzioni svolte dai livelli posti sopra. 

### **Astrazione dei canali virtuali OSI**

Un **processo** è **individuabile univocamente** all'interno di una stessa macchina host tramite un **numero di porta**, riusabile uguale solo su interfacce diverse o su macchine diverse. Un **processo** è **individuabile univocamente** nel mondo tramite la **coppia** (indirizzo **IP pubblico**, numero di **porta**) detta talvolta **socket**. Esistono pure IP privati **riusabili localmente** in una rete LAN, ma non permettono di individuare un host in una rete WAN di estensione globale.

La **ricostruzione sottostante** mira a rappresentare una rete OSI come una **infrastruttura** volta a realizzare un **canale** (apparentemente) **dedicato** ad una particolare coppia di **processi remoti**, tra i tanti che girano in due **host remoti**, a loro volta scelti tra i tanti collegati ad **Internet**.

| Livello | Nome                  | Astrazione fornita al livello superiore       | Funzioni svolte (riassunto incompleto)                        |
|---------|-----------------------|-------------------------------------------------------------------------------------------------------------------------|-----------------------------|
| 7       | Applicazione          | Fornisce all'applicazione un canale **dedicato** (prenotato) a 2 **host** remoti e a 2 **processi**,  steso tra due **porte**, a basso BER, FIFO e immune dalla perdita di pacchetti per congestione della rete e sovraccarico del ricevente e immune dalla perdita di pacchetti per mancato accordo sulle fasi della trasmissione e con un formato comune del payload                                                                                                                 | Non deve fare nulla, se non fornire l'indirizzo del destinatario |
| 6       | Presentazione         | Fornisce al livello 7 un canale **dedicato** (prenotato) a 2 **host** remoti e a 2 **processi**, steso tra due **porte**,  a basso BER, FIFO e immune dalla perdita di pacchetti per congestione della rete e sovraccarico del ricevente e immune dalla perdita di pacchetti per mancato accordo sulle fasi della trasmissione e con un formato comune del payload                                                                                                                 | Realizza una negoziazione dei formati del payload tra quelli disponibili ad entrambi gli interlocutori| 
| 5       | Sessione              | Fornisce al livello 6 un canale **dedicato** (prenotato) a 2 **host** remoti e a 2 **processi** e steso tra due **porte**, a basso BER, FIFO e immune dalla perdita di pacchetti per congestione della rete e sovraccarico del ricevente e immune dalla perdita di pacchetti per mancato accordo sulle fasi della trasmissione | La trasmissione viene suddivisa in fasi su cui esiste un accordo su quanto durino e come si raggiungano |                                                                                                            |
| 4       | Trasporto             | Fornisce al livello 5 un canale **dedicato** (prenotato) a 2 **host** remoti e a 2 **processi**, steso tra due **porte**, a basso BER, FIFO e immune dalla perdita di pacchetti per congestione della rete e per sovraccarico del ricevente | Esegue, tramite multiplazione TDM statistica,  la condivisione del canale IP tra più segmenti provenienti da processi diversi, abbassa ulteriormente la BER, limita la velocità del mittente a beneficio del ricevitore e della rete intera, esegue il riordino dei segmenti fuori ordine|                                                                                                          |
| 3       | Rete                  | Fornisce al livello 4 un canale **dedicato** (prenotato) a 2 **host** fisicamente lontani (remoti), a basso BER, immune dal sovraccarico del ricevente e **steso** tra i due **host** remoti scegliendo autonomamente i link che fanno parte del percorso|      Realizza la localizzazione fisica del destinatario, il calcolo del percorso per raggiungerlo attraverso i nodi della rete e l'effettivo inoltro dei pacchetti IP da una porta di ingresso ad una di uscita di ciascun nodo lungo il percorso scelto.|                                                                         |
| 2       | Collegamento dati     | Fornisce al livello 3 un canale **dedicato** (prenotato) a 2 **host** fisicamente lontani (remoti) e **steso** tra 2 **IS vicini**, e con bassa BER e immune dal sovraccarico del ricevente  | Realizza, tramite multiplazione TDM statistica, una condivisione del canale tra più host e una rilevazione e correzione di errore tramite CRC e una limitazione di velocità del trasmittente |
| 1       | Fisico                | Fornisce al livello 2 un canale di bit **dedicato** tra due **nodi IS** fisicamente vicini                                | Esegue una codifica/decodifica dei bit nei segnali fisici (elettrici, radio o ottici) adatti ad un certo mezzo fisico|

### **Astrazioni topologiche**

#### **Rete L3 riassunta in un unico link L4**

Astrazioni interessanti sono quelle, di tipo **topologico**, che traducono una rete con molti **link fisici** in un unico **link logico** che li riassume tutti (astrazione). Questa cosa avviene tipicamente a:
- **livello 4**, dove una **intera rete IP**, magliata e composta da molti **router** e da molti link, viene astratta in un unico **link diretto tra host** senza router (**Link End-to-End** virtuale fornito come servizio di rete di livello 4).
- In tutti gli **altri livelli** della pila OSI, canali punto-punto vengono astratti in altri canali **punto-punto** posti:
    - tra due **router vicini** (adiacenti) nei primi due livelli
    - tra i dispositivi **End to End** (host) nei livelli 5, 6 e 7.
 
<img src="img/subnet.png" alt="alt text" width="1000">  

#### **Rete LAN riassunta in un unico link L2**

Una **eccezione** notevole fanno le **LAN** cioè le reti locali composta da switch. Una **intera rete** LAN, composta da molti switch collegati insieme con dei link di livello 1, viene considerata come un unico link (collegamento) di livello 2 **diretto tra due router**. Ciò accade per una anomalia introdotta storicamente dall'impiego delle LAN come parti delle reti IP (dette subnet) che sfocia nell'utilizzo combinato di router e switch. Questa usanza ha, di fatto, introdotto due **livelli di routing complementari**, di cui solo quello esterno **tra i router** è, in realtà, preso in considerazione dell'architettura ISO/OSI. Riassumendo, abbiamo **due livelli di smistamento** (inoltro):
- uno **esterno alla LAN** che si occupa della consegna dei pacchetti **tra i router di confine** delle LAN sorgenti e destinazione. Realizza un **inoltro** di **pacchetti IP**:
     - è un processo che **smista** i pacchetti da una porta di ingresso ad una di uscita di un router.
     - Ogni porta di uscita rappresenta un **percorso** tra i router differente e viene **scelto** in base all'**indirizzo di destinazione** del pacchetto di livello 3 (IP).
     - è il livello di smistamento dei pacchetti negli ISP (Internet Service Provider) che compongono Internet. E' l'unico routing preso in considerazione da ISO/OSI perchè smista in base all'indirizzo IP di destinazione delle PDU.
- uno **interno alla LAN** che si occupa della consegna dei pacchetti tra l'**host sorgente** e il **router di confine** delle LAN **sorgente** oppure della consegna tra il **router di confine** della LAN di **destinazione** e l'**host** di destinazione. Realizza un **inoltro** di **trame MAC** tra **SW**:
     - è un processo che **smista** i pacchetti da una porta di ingresso ad una di uscita di uno switch.
     - Switch e Hub non modificano gli indirizzi di livello 3 (IP) durante l'attraversamento
     - Ogni porta di uscita rappresenta un **percorso** tra gli switch differente e viene **scelto** in base all'**indirizzo di destinazione** del pacchetto di livello 2 (MAC).
     - Dal **livello 3**, questo percorso viene **percepito** come un **link diretto** tra due qualunque **coppie di host** della LAN, cioè come una connessione di linea **punto punto**.
     - ISO/OSI lo assimila ad una consegna di PDU **senza smistamento** perchè per lei avverrebbe lungo il canale punto-punto che unisce due router adiacenti (o un router e un host). Questo canale in realtà è composto da una rete di switch che, pur non smistando pacchetti IP direttamente, smista trame MAC, ciascuna col proprio indirizzo di livello 2 e ciascuna contenente un pacchetto IP come payload. Non essendo svolto in base ad un indirizzo di livello 3, questo routing, per ISO/OSI, non esiste.

Le LAN possono però essere realizzate anche con i **mezzi multipunto** di tipo broadcast, cioè con i **BUS**. I dispositivi commerciali, a filo e radio, che li realizzano sono chiamati genericamente **HUB**. Un HUB collega **molte stazioni** con lo **stesso canale** fisico multipunto che, per evitare il fenomeno delle collisioni, è regolato da opportuni **protocolli di arbitraggio**. 

I protocolli di arbitraggio realizzano, al pari degli switch, una **LAN**. Più mezzi a bus collegati tra loro sono sempre la medesima LAN che termina solamente a ridosso del **link con un router**. Anche in questo caso la LAN è una vera e propria **rete** che offre al **livello 2** servizi di connessione tra **coppie di host**. Dal **livello 2**, questo percorso viene **percepito** come un **link fisico diretto** tra due qualunque **coppie di host** della LAN, cioè come una connessione fisica **punto punto**.

### **Astrazione di una rete a circuito**

Una rete a **commutazione di circuito** è realizzata soltanto con **multiplatori TDM statici** ed è assimilabile a una **cascata di slot** prenotati su **diverse tratte** (link) ed allocati ad una **sola comunicazione**.  

Dalla **tabella precedente**, si può osservare facilmente come alla fine, il **servizio offerto** dal livello applicativo della pila ISO/OSI, oltre a garantire l'**interoperabilità universale** tra i dispositivi, oltre ad implementare un **sistema aperto**, ha la possibilità di poter realizzare, a **livello logico** (cioè percepito), anche una **qualità del servizio** analoga a quella offerta da una **rete statica commutata** (rete a commutazione di circuito).  

La **percezione** di trovarsi in una rete a commutazione di circuito è solo **approssimata** ed è tanto più realistica quanto più grande è il numero dei nodi allocati e quanto maggiore è la banda installata sui canali, in modo che la **variabilità dei ritardi** (jitter) sulle **code** dei multiplatori statistici si mantenga **minima**. 

I **vantaggi**, incomparabilmente **maggiori**, che una rete a commutazione di pacchetto offre rispetto ad una a circuito (con multiplatori TDM statici) sono:  la **scalabilità** (ottenuta facilmente aggiungendo **nuovi nodi** e **nuovi link**) e l'**efficienza** (ottenuta grazie alla multiplazione statistica). 

Per contro, i canali realizzati con **multiplazione statica** mantengono un'**affidabilità** e precisione temporale nella consegna (**jitter**) tutt'ora ineguagliate dalle reti a pacchetto. Nonostante ciò, la **multiplazione statica** è oramai preferibile solo in applicazioni **di nicchia** in cui il **traffico prenotato** è sempre **costante** e mai **sprecato**. Ciò accade, ad esempio, nel mondo IoT quando è necessaria l'interrogazione periodica (**polling**) dei sensori, come quella effettuata, ad esempio, all'interno della zona CFP delle **supertrame** wireless.


## **Implementazione dei protocolli**

La responsabilità della implementazione dei protocolli OSI è ripartita tra vari moduli SW e da un livello HW:

| Livello |Protocolli                |      Chi li implementa   | Dove girano | Chi collegano  |
|---------|-----------------------|-----------------------------|-----------------------------|-----------------------------| 
| 7       | Applicazione          |  Applicazione| Host | Processi |
| 6       | Presentazione         |  Middleware|  Host  | Processi |
| 5       | Sessione              |  Middleware|   Host     |   Processi  |                                                                                           
| 4       | Trasporto             | Sistema operativo|   Host     |   Processi   |                                                                                              
| 3       | Rete                  |Sistema operativo|    Router, host      |  Host  |                                                           
| 2       | Collegamento dati     | Driver di scheda di rete| Switch, bridge, router, host| NIC |
| 1       | Fisico                |HW della scheda di rete| Hub, switch, bridge, router, host | Prese  |

Quando **non esiste** un **middleware installato** sul sistema che realizza i protocolli di **sessione e presentazione**, allora queste funzioni, anche se nella maniera più parziale possibile, **vengono svolte** direttamente **dall'applicazione**.

## **Indirizzi e PDU**

Ogni livello ha la sua PDU col proprio nome e un header con la sua coppia di indirizzi sorgente/destinazione che servono ad identificare univocamente una entità di quel livello all'interno dello stesso:

| Livello |Protocolli                |      Nome PDU     |    Indirizzo |
|---------|-----------------------|-----------------------------|--------------------------------|
| 7       | Applicazione          |  Pacchetto applicativo |      Credenziali utente, certificato di sicurezza     |
| 6       | Presentazione         |  Pacchetto applicativo |      URL web         |
| 5       | Sessione              | Pacchetto applicativo |      Id di sessione       |                                                                                        
| 4       | Trasporto             | Segmento |       Numero di porta TCP o UDP        |                                                                                                
| 3       | Rete                  | Pacchetto |   Indirizzo IP           |                                                          
| 2       | Collegamento dati     | Trama |     Indirizzo MAC, numero di porta di inoltro, credenziali L2 (PPP o EAP)       |
| 1       | Fisico                | Bit |       Etichetta della presa        |

Valgono le seguenti **proprietà**:
- Gli indirizzi IP e URL sono **mappati 1:1** uno sull'altro. L'utente conosce solo l'URL che poi viene tradotto nel corrispondente IP dal servizio DNS (di livello applicativo).
- un **indirizzo IP** identifica univocamente **un host** globalmente nel mondo (Internet).
- un **num. di porta L4** identifica univocamente **un processo** localmente su una macchina.
- un **indirizzo IP + num. di porta L4** (socket) identifica univocamente **un processo** globalmente nel mondo (Internet).
- una **credenziale utente** o un **certificato** di sicurezza identificano univocamente **un utente** all'interno di un **dominio** (organizzazione).

<img src="img/mappeindirizzi.png" alt="alt text" width="1000">

<img src="img/osiall.jpg" alt="alt text" width="1000">

## **OSI come architettura di riferimento**

Lo **stack di protocolli OSI** è un **modello architetturale** di riferimento. Per **ogni strato** sono stati studiati nel tempo un gran numero di protocolli, ciascuno con i propri **pregi e difetti**. Un'**architettura reale**, quella che poi verrà standardizzata ed implementata in un dispositivo commerciale, per ogni strato della propria pila, sceglierà, tra i tanti disponibili in letteratura tecnica, un certo **tipo di protocollo** del quale realizzerà e standardizzerà la propria **particolare versione**. 

Attualmente, per **accedere ad Internet** e all'interno della maggior parte delle **reti locali LAN**, si adopera la cosiddetta suite **TCP/IP**. In ogni caso, molte **reti di sensori**, per funzionare localmente al loro ambiente di lavoro, **non sempre usano** la suite TCP/IP. Inoltre, anche le reti di sensori che l'adoperano, ai livelli inferiori come **i livelli data link e il livello fisico**, spesso utilizzano protocolli **diversi** da quelli che sono stati standardizzati per le LAN. 

Per delle domande riassuntive vedi [domande osi](domandeosi.md)

[Dettaglio ES/IS](isvses.md)

[Dettaglio protocolli](protocolli.md)

[Dettaglio multiplazioni statiche](multiplazioni.md)

[Dettaglio TDM statico su mezzi punto-punto](tdmstatico.md) 

[Dettaglio tecnologie di accesso al mezzo radio](accessoradio.md)

[Dettaglio TDM statistico su mezzi punto-punto](tdmstatistico.md) 

Sitografia:
- https://www.edutecnica.it/informatica/osi/osi.htm
- https://insights.profitap.com/osi-7-layers-explained-the-easy-way
- https://it.wikipedia.org/wiki/Internet
- http://www.tcpipguide.com/free/t_DataEncapsulationProtocolDataUnitsPDUsandServiceDa-2.htm
- https://vahid.blog/post/2020-12-24-how-the-internet-works-part-iii-reliability-and-security/

>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)



