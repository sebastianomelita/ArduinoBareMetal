>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)


## **Stack ISO/OSI**

### **Interoperabilità**

L'**ISO/OSI** ha catalogato e organizzato in una **architettura a 7 strati** tutti i **protocolli** concepiti nel tempo organizzandoli in base alle **categorie di problemi** di rete che essi risolvono. L'architettura è normalmente rappresentata sotto forma di **stack** (pila) e, per ogni elemento dello stack, identifica una **categoria di protocolli** che deve risolvere **un determinato insieme di problemi** della comunicazione in rete. L'architettura è **modulare** nel senso che uno strato può essere sostituito senza che quelli sopra di lui se ne accorgano permettendo una grande **flessibilità**. Questa flessibilità può essere utilizzata per **adattare** un protocollo di livello superiore a **vari mezzi fisici** o per ottenere su un certo mezzo fisico una determinata **qualità del servizio**. **Obiettivo finale** è garantire l'**interoperabilità universale tra i dispositivi** da collegare a prescindere dalle possibili differenze che possono intercorrere rispetto a marca, modello e tecnologie adoperate. 

### **Architettura a strati**

Un **vincolo dell'architettura** è che uno strato può dialogare solo con quelli adiacenti, o sopra o sotto di lui.

<img src="iso-osi1.png" alt="alt text" width="1000">

### **Canali virtuali**

I canali virtuali sono dei **canali logici** che vengono creati grazie alla tecnica dell'**imbustamento multiplo**. I **canali logici** hanno delle funzioni in più che **non** sono presenti sui **canali fisici** che permettono di risovere alcuni dei problemi che si incontrano per realizzare una comunicazione **universale** tra dispositivi **eterogenei**.

#### **Imbustamento multiplo**

Nelle **reti a pacchetto**, in fase di **trasmissione**, si usa una tecnica, detta **imbustamento multiplo**, con la quale si **incapsulano** i messaggi di un livello, le **PDU** (Protol Data Unit),  nel campo dati, le **SDU** (Service Data Unit), del livello **immediatamente inferiore**. Questa operazione parte dal livello applicativo ed è **ripetuta** in tutti i livelli, escluso il fisico. Il **carico utile** o **payload** è il **messaggio** che la rete ha avuto in consegna, da parte dell'utente, per essere **spostato** fino alla destinazione. Ogni livello **aggiunge** al messaggio utente informazioni di controllo propriamente sue nel campo **header**. Il **livello fisico**, a seguito di tutti gli annidamenti, ha il **messaggio più lungo** perché è composto dal messaggio utente a dalla **somma di tutti gli header** aggiunti nei singoli livelli, detta, in gergo tecnico, **overhead**. 

All'**imbustamento multiplo** in **trasmissione** corrisponde lo **sbustamento multiplo in ricezione**, un'operazione analoga ma **inversa**, dove tutti i messaggi ricevuti a **livello fisico** incapsulati uno dentro l'altro, vengono, salendo da un livello all'altro, **sbustati** eliminando l'intestazione del livello immediatamente inferiore. Arrivati al **livello applicativo**, come risultato, si ottiene il **messaggio utente**. Quindi, l'**ordine** di **inserimento** delle **intestazioni** in trasmissione è esattamente l'inverso dell'ordine di **rimozione** delle stesse in ricezione, secondo una tipica **politica LIFO**.

**L'imbustamento multiplo** permette la creazione dei cosiddetti **canali virtuali** cioè dei collegamenti **apparenti** e **diretti** tra **strati corrispondenti** di dispositivi **remoti**. Un nodo può essere **logicamente** suddiviso in una serie di strati detti **"entità"**, ciascuna ha uno specifico **ruolo** nella comunicazione, caratterizzato da specifici compiti e funzioni che hanno la particolarità di dover essere svolti in maniera distribuita nei vari nodi attraverso lo scambio di opportuni messaggi. Ogni entità lascia una traccia nel messaggio finale, dato che in esso è sempre possibile isolare sia il dato trasmesso da quella entità (payload) che le informazioni di controllo necessarie per realizzare le funzioni di quell'entità (header). 

#### **Astrazione funzionale**

Ogni funzione di un livello **"astrae"**, cioè riassume in un'unica funzione di alto livello **generica**, molte funzioni dei livelli sottostanti legate a dettagli HW specifici. Per cui, la particolarità di questi collegamenti è quella di realizzare una **virtualizzazione della rete** e dei suoi dispositivi che, man mano che si sale dal livello fisico a quello applicativo, si fa sempre più spinta, **nascondendo** i dettagli implementativi degli strati inferiori e rendendo **impercettibili** le loro **differenze** ai livelli sopra di essi. Nel caso di una rete IP il **livello TCP** di trasporto già non **"vede"** più le differenze tecnologiche tra reti realizzate con un HUB WiFi piuttosto che con uno switch o piuttosto con più router in cascata, essendo stati questi dispositivi tutti **"astratti"**, cioè riassunti in un **unico collegamento** di tipo **TCP** tra **due host terminali**.

Astrazioni interessanti sono quelle, di tipo **topologico**, che traducono una rete con molti **link fisici** in un unico **link logico** che li riassume tutti (astrazione). Questa cosa avviene tipicamente a:
- **livello 4**, dove una **intera rete IP**, composta da **link tra router** collegati, in genere, a maglia, viene astratta in un **link tra host** diretto.
- In tutti gli **altri livelli** della pila OSI vengono astratti solo canali **punto-punto** posti:
    - tra due **router vicini** (adiacenti) nei primi due livelli
    - tra i dispositivi **End to End** (host) nei livelli 5, 6 e 7.
  
Una eccezione notevole fanno le **LAN**, dove una **intera rete** LAN, composta di **link tra SW** collegati, in genere, ad albero, viene astratta dal protocollo IP (di livello 3) in tanti **link tra host** diretti. A livello logico, ne è possibile realizzare uno per ogni possibile collegamento tra coppie di host (**maglia completa** come servizio di livello 3). 


#### **Canale reale**

Il **canale reale** è il **mezzo trasmissivo** che unisce il **livello fisico** dei due interlocutori. Si sviluppa in **orizzontale** lungo il mezzo trasmissivo ma anche in **verticale** lungo le interfacce tra i vari strati, **a scendere** in trasmissione, **a salire** in ricezione.
I messaggi dei vari livelli, **incapsulati** uno dentro l'altro, vengono trasmessi in un **blocco unico** lungo il **canale reale**. Ma per comodità, all'**interno dei dispositivi**, ogni **messaggio applicativo**  può essere visto **destrutturato** in una serie di messaggi **separati** lungo i 7 **canali virtuali** dei singoli livelli.  Il **messaggio parziale**, trasmesso e ricevuto lungo un livello, è inviato con il **formato** e con le **regole** di trasmissione (**protocollo**) propri di quel livello. **In sostanza**, i messaggi che sono stati **generati** e **trasmessi separatamente** dai 7 livelli lungo i 7 **canali virtuali**, diventano un **unico messaggio matrioska** di 7 messaggi, **annidati** uno dentro l'altro, una volta che sono trasmessi lungo il **canale reale**. 

### **Nodi ES ed IS**

**Riassumendo**, i **canali virtuali non esistono fisicamente** ma sono **ugualmente reali** perchè, dal punto di vista dei **messaggi scambiati**, le **entità** pari (peer entity), cioè gli interlocutori corrispondenti nei vari livelli, si comportano come se essi ci fossero davvero e attraverso questi **effettivamente dialogassero**. Sono una **visione semplificata** con cui, ad un **livello superiore**, si possono **osservare** le funzioni svolte e i messaggi scambiati dai livelli inferiori. Semplificata, ma comunque reale.

**I canali virtuali** si dividono in:
- link **end to end**, chiamati così perché collegano livelli che sono presenti **soltanto** negli **host** cioè i **dispositivi terminali**, quelli su cui si **interfaccia l'utente**
- Più in basso (sui **primi 3** livelli), stanno i **link IS-IS** (collegamento tra  Intemediate systems), chiamati così perché collegano tra loro i **dispositivi di rete**, cioè quelli che creano la rete. Anche gli host posseggono questi livelli e pertanto sono, a tutti gli effetti, dispositivi di rete anch'essi (anche se privi di alcune funzioni importanti).

In figura la lettera H sta per **header** cioè intestazione (ad es. AH=Application header).
<img src="iso-osi.png" alt="alt text" width="1000">

### **Architettura di riferimento**

Lo **stack di protocolli OSI** è un **modello architetturale** di riferimento. Per **ogni strato** sono stati studiati nel tempo un gran numero di protocolli, ciascuno con i propri **pregi e difetti**. Un'**architettura reale**, quella che poi verrà standardizzata ed implementata in un dispositivo commerciale, per ogni strato della propria pila, sceglierà, tra i tanti disponibili in letteratura tecnica, un certo **tipo di protocollo** del quale realizzerà e standardizzerà la propria **particolare versione**. Attualmente, per **accedere ad Internet** e all'interno della maggior parte delle **reti locali LAN**, si adopera la cosiddetta suite **TCP/IP**. In ogni caso, molte **reti di sensori**, per funzionare localmente al loro ambiente di lavoro, **non sempre usano** la suite TCP/IP. Inoltre, anche le reti di sensori che l'adoperano, ai livelli inferiori come **i livelli data link e il livello fisico**, spesso utilizzano protocolli **diversi** da quelli che sono stati standardizzati per le LAN. 

>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)



