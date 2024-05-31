>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)


## **Stack ISO/OSI**

### **Interoperabilità**

L'**ISO/OSI** ha catalogato e organizzato in una **architettura a 7 strati** tutti i **protocolli** concepiti nel tempo organizzandoli in base alle **categorie di problemi** di rete che essi risolvono. L'architettura è normalmente rappresentata sotto forma di **stack** (pila) e, per ogni elemento dello stack, identifica una **categoria di protocolli** che deve risolvere **un determinato insieme di problemi** della comunicazione in rete. L'architettura è **modulare** nel senso che uno strato può essere sostituito senza che quelli sopra di lui se ne accorgano permettendo una grande **flessibilità**. Questa flessibilità può essere utilizzata per **adattare** un protocollo di livello superiore a **vari mezzi fisici** o per ottenere su un certo mezzo fisico una determinata **qualità del servizio**. **Obiettivo finale** è garantire l'**interoperabilità universale tra i dispositivi** da collegare, a prescindere dalle possibili differenze che possono intercorrere rispetto a marca, modello e tecnologie adoperate. 

### **Architettura a strati**

Un **vincolo dell'architettura** è che uno strato può dialogare solo con quelli adiacenti, o sopra o sotto di lui.

<img src="iso-osi1.png" alt="alt text" width="1000">

| Livello | Nome                  |       Funzioni svolte                        |      Esempi di protocollo        |
|---------|-----------------------|-----------------------------|-----------------------------|
| 7       | Applicazione          |  Applicazione **Client/Server** che virtualizza una risorsa **remota** e **condivisa** facendola apparire come **locale** e **dedicata**, definisce il **tipo di utenti** e il **tipo di sicurezza** con cui dialogano| HTTP, DNS, DHCP, FTP, SFTP, scp, rsync, rclone |
| 6       | Presentazione         |  Traduce **documenti**, **flussi di dati**, **record**, **caratteri** da un formato all'altro (ad es. XML piuttosto che JSON oppure mp3 al posto di mp4), realizza la **compressione** dei dati e la loro **cifratura**, realizza la **serializzazione** degli oggetti (ad esempio struct)| MIME, XDR, HTML, XML, SSL, TLS, zip |
| 5       | Sessione              |   Apre, gestisce e termina conversazioni e scambio di dati (sessioni) tra due applicazioni stabilendone le fasi, gestisce il **tipo di comunicazione** (half duplex, full duplex) | SMB, NetBIOS  |                                                                                                   |
| 4       | Trasporto             | **Segmentazione** di ciascun pacchetto in più segmenti, **multiplazione** di più connessioni di livello 4 (uniscono processi diversi) in una stessa connessione di livello 3 (unisce due host), **controllo di errore** End to End (con ritrasmissione), **controllo di flusso** End to End, **riordino** dei pacchetti fuori sequenza, **controllo di congestione** |   TCP, UDP      |                                                                                                  |
| 3       | Rete                  | **Indirizzamento**, cioè individuazione del luogo fisico in cui si trova l'host destinatario, **sceglie** il percorso migliore per raggiungerlo (**routing**), esecuzione dell'**inoltro fisico** dei pacchetti da una porta di ingresso a quella di uscita che sta nel percorso scelto (**forwarding**), per ogni router incontrato lungo il percorso.  |   IP    |                                                                  |
| 2       | Collegamento dati     | **Segmenta** i messaggi lunghi aggiungendo loro l'indirizzo del mittente, esegue la **multiplazione** di più connessioni di livello 2 sulla stessa connsessione di livello 1, esegue il **controllo di errore** (senza ritrasmissione), esegue il **controllo di flusso** (velocità del mittente) | Ethernet, WiFI, BLE, Zigbee, LoRa|
| 1       | Fisico                | Esegue la **codifica/decodifica** dei bit, cioè traduce i bit in segnali elettrici in **trasmissione** mentre traduce i segnali elettrici in bit **in ricezione**. Definisce gli **standard** meccanici ed elettrici dei canali.| CCA, LBT|

### **Canali virtuali**

#### **Definizione**

I **canali virtuali** sono dei **canali logici** che **mimano** un collegamento **diretto** tra entità pari dello **stesso livello**, attraverso il quale vengono scambiate le **PDU** di quel livello con le regole del **protocollo** che in esso sono in uso. I **canali virtuali** vengono **creati** grazie alla tecnica dell'**imbustamento multiplo** e rappresentano il **punto di vista** che un **generico livello** ha dell'unico canale effettivamente esistente, cioè il canale reale di livello 1 (fisico). 

E' un punto di vista **parziale** perchè non si basa su quello che una comunicazione realmente è nella sua **complessità** ed estensione (a livello fisico e lungo i vari nodi), ma per come, ad un **certo livello**, essa appare, semplicemente osservando il modo (protocollo) con cui si **comporta** lo scambio dei **dati** all'interno di quel **livello**. Infatti, **ad ogni livello**:
- molte **proprietà** del canale vengono date **per scontate**, non percependo affatto che vengono realizzate **da altri** livelli.
- lo **schema** della rete appare **semplificato**, non percependo che la sua **struttura** è molto più **articolata**, anche se è stata **riassunta** in un **unico link**.

I **canali logici** hanno delle funzioni in più, di cui **non** godono i **canali fisici** sottostanti, che permettono di risovere quei problemi che si incontrano per realizzare una comunicazione **universale** tra dispositivi **eterogenei**.

Ogni **canale logico** serve a collegare tra loro le **due entità**, moduli SW con un certo **ruolo** (analoghi a certi impiegati tra i tanti di una grande azienda), che sono deputate a risolvere solamente **certi problemi**, tutti e soli quelli che rientrano all'interno delle **mansioni** che competono a quelle entità. 

#### **Messaggi di controllo**

La **risoluzione dei problemi** di rete è sempre **distribuita**, nel senso che non può avvenire senza lo **scambio di messaggi di servizio** che servono a coordinare il lavoro tra le **entità pari**, cioè quelle dello **stesso livello**. I messaggi di servizio (detti **messaggi di controllo**) sono quelli legati al **ruolo** e alle **mansioni** dei due interlocutori del livello corrente e trascurano gli **altri** messaggi di servizio relativi ad **altre mansioni** che, essendo sotto la responsabilità di **altri ruoli** appartenenti ad **altri livelli**, vengono, dalle entità del livello attuale, completamente **ignorate**.

I **canali logici**, però, pur agendo in **contemporanea**, non sono affatto una replica di **7 messaggi diversi** che giungono a destinazione percorrendo **7 canali fisici paralleli**. Il canale fisico è sempre **uno solo** e il messaggio fisicamente è sempre la **stessa sequenza** di bit ai capi delle due interfacce omologhe di **livello 1** (fisico). Quello che accade è che, intorno allo stesso messaggio di partenza, le informazioni di **controllo**, necessarie per svolgere le funzioni di rete, vengono **modificate** passando da un livello all'altro:
- In **trasmissione** vengono **aggiunte** ad ogni livello scendendo lungo la pila.
- In **ricezione** vengono **rimosse** salendo di un livello alla  volta lungo la pila.   

Quello che **cambia**, da un livello all'altro, è:
- la **visione** che le varie entità hanno della **comunicazione** che, scendendo di livello, diventa via via più densa di **dettagli fisici**.
- l'insieme dei **messaggi di controllo** rappresentato dall'**header** del messaggio ad un certo livello che è funzionale allo svolgimento dei compiti assegnati a quel livello.

#### **Imbustamento multiplo**

E' una **tecnica** il cui obiettivo è la **creazione** dei **canali virtuali**.

Nelle **reti a pacchetto**, in fase di **trasmissione**, si usa una tecnica, detta **imbustamento multiplo**, con la quale si **incapsulano** i messaggi di un livello, le **PDU** (Protol Data Unit),  nel campo dati, detto **SDU** (Service Data Unit), del livello **immediatamente inferiore**. Questa operazione parte dal livello applicativo ed è **ripetuta** in tutti i livelli, escluso il fisico. Il **carico utile** o **payload** è il **messaggio** che la rete ha avuto in consegna, da parte dell'utente, per essere **spostato** fino alla destinazione. Ogni livello **aggiunge** al messaggio utente informazioni di controllo propriamente sue nel campo **header**. Il **livello fisico**, a seguito di tutti gli annidamenti, ha il **messaggio più lungo** perché è composto dal messaggio utente a dalla **somma di tutti gli header** aggiunti nei singoli livelli, detta, in gergo tecnico, **overhead**. 

All'**imbustamento multiplo** in **trasmissione** corrisponde lo **sbustamento multiplo in ricezione**, un'operazione analoga ma **inversa**, dove tutti i messaggi ricevuti a **livello fisico** incapsulati uno dentro l'altro, vengono, salendo da un livello all'altro, **sbustati** eliminando l'intestazione del livello immediatamente inferiore. Arrivati al **livello applicativo**, come risultato, si ottiene il **messaggio utente**. Quindi, l'**ordine** di **inserimento** delle **intestazioni** in trasmissione è esattamente l'inverso dell'ordine di **rimozione** delle stesse in ricezione, secondo una tipica **politica LIFO**.

**L'imbustamento multiplo** permette la creazione dei cosiddetti **canali virtuali** cioè dei collegamenti **apparenti** e **diretti** tra **strati corrispondenti** di dispositivi **remoti**. Un nodo può essere **logicamente** suddiviso in una serie di strati detti **"entità"**, ciascuna ha uno specifico **ruolo** nella comunicazione, caratterizzato da specifici **compiti** e **funzioni** (oltre quello della mera **consegna del payload** all'entità pari all'altro capo del canale virtuale) che hanno la particolarità di dover essere svolti in **maniera distribuita** nei vari nodi, attraverso lo scambio di opportuni **messaggi di controllo**. Ogni entità lascia, quindi, una traccia nel messaggio finale, dato che in esso è sempre possibile isolare sia il messaggio utente trasmesso (**payload**) che le **informazioni di controllo** necessarie per realizzare le funzioni di quell'entità (**header**). 

Un **protocollo** del **livello generico N**:
- nel **processo di imbustamento**, vede come ultimo header, solo gli **header del suo livello** perchè gli header dei **livelli sottostanti** non sono ancora stati inseriti.
- Nel **processo di sbustamento** accade invece che un livello N vede solo l'**ultimo header affiorante**, cioè il proprio, perchè quelli dei **livelli sottostanti** sono stati **rimossi** al momento del loro sbustamento.

In ogni caso, anche un livello vedesse gli **header di livelli diversi** dal proprio, non saprebbe cosa farsene perchè rappresentano **informazioni di servizio** di un **protocollo diverso** dal proprio (con obiettivi diversi) che non sarebbe in grado di interpretare.

Il meccanismo dell'imbustamento/sbustamento fa si che, al momento in cui un pacchetto raggiunge un certo **livello N**, questo esibisca come **header più esterno** sempre un **N-PCI**, l'unico header che il protocollo di **livello N** è in grado di **interpretare**. Inoltre, il livello N sa che tutto quello che viene dopo un N-PCI è la **N-SDU (payload)** del proprio livello e va **consegnata** al livello superiore (o a quello inferiore) senza modifiche.

#### **Trasparenza dei livelli**

La **trasparenza** dei livelli nel modello OSI (Open Systems Interconnection) si riferisce al principio secondo cui ciascun livello del modello fornisce servizi al livello immediatamente superiore **senza** che quest'ultimo debba conoscere i **dettagli specifici** dell'**implementazione** del livello inferiore.

La **trasparenza** dei livelli aggiunge proprietà di **flessibilità** all'architettura perchè, grazie a questa, è possibile **modificare** o **sostituire** uno o più livelli inferiori senza la necessità di modificare tutti quelli superiori, promuovendo così il **riuso** dei livelli più astratti. Ciò è sempre possibile a patto che l'**interfaccia** tra di essi rimanga **invariata**. Le interfacce standardizzano i servizi forniti da un livello all'altro e permettono:
- **Modularità**: ogni livello può essere sviluppato e aggiornato indipendentemente, facilitando la manutenzione e l'innovazione tecnologica.
- **Interoperabilità**: la trasparenza dei livelli permette a dispositivi e protocolli diversi di lavorare insieme, a patto che rispettino le interfacce standardizzate.
- **Semplicità di progettazione**: gli sviluppatori possono concentrarsi sulle funzionalità specifiche di un livello senza dover comprendere l'intera stack di protocolli.
- **Risoluzione dei problemi**: la suddivisione delle funzionalità in livelli separati rende più facile l'identificazione e la risoluzione dei problemi, poiché i problemi possono essere isolati a un livello specifico.

#### **Astrazione**

Vale il principio di **astrazione**, che vuol dire che, man mano che **si sale**, ogni **livello** ha una visione del problema iniziale via via più **semplice e generale** che si occupa sempre meno dei **dettagli fisici** e, viceversa, sempre di più di problematiche concettuali legate alla **natura** dei messaggi, del loro **significato** più che del **modo** con cui essi vengono **scambiati**. La **soluzione astratta** di un problema ha il decisivo **vantaggio** di poter essere utilizzata uguale anche all'interno di **contesti differenti**, lì dove i **dettagli** per la sua applicazione vengono declinati in **maniera diversa**. 

In genere, grossomodo accade che la soluzione astratta viene **adattata**, mentre quella di dettaglio viene **sostituita**.

Ogni funzione di un livello **"astrae"**, cioè riassume in un'unica funzione di alto livello **generica**, molte funzioni dei livelli sottostanti legate a dettagli HW specifici. Per cui, la particolarità di questi collegamenti è quella di realizzare una **virtualizzazione della rete** e dei suoi dispositivi che, man mano che si sale dal livello fisico a quello applicativo, si fa sempre più spinta, **nascondendo** i dettagli implementativi degli strati inferiori e rendendo **ininfluenti** le loro **differenze** per i livelli sopra di essi. 

#### **Astrazione dei livelli OSI**

Un **processo** è **individuabile univocamente** all'interno di una stessa macchina host tramite un **numero di porta**, riusabile uguale su macchine diverse. Un **processo** è **individuabile univocamente** nel mondo tramite la **coppia** (indirizzo **IP**, numero di **porta**) detta talvolta **socket**.

La **ricostruzione sottostante** mira a rappresentare una rete OSI come una **infrastruttura** volta a realizzare un **canale** (apparentemente) **dedicato** ad una particolare coppia di **processi remoti**, tra i tanti che girano in due **host remoti**, a loro volta scelti tra i tanti collegati ad **Internet**.

| Livello | Nome                  | Astrazione fornita al livello superiore       | Funzioni svolte (riassunto incompleto)                        |
|---------|-----------------------|-------------------------------------------------------------------------------------------------------------------------|-----------------------------|
| 7       | Applicazione          | Fornisce all'applicazione un canale **dedicato** (prenotato) a 2 **host** remoti e a 2 **processi**,  steso tra due **porte**, a basso BER, FIFO e immune dalla perdita di pacchetti per congestione della rete e sovraccarico del ricevente e immune dalla perdita di pacchetti per mancato accordo sulle fasi della trasmissione e con un formato comune del payload                                                                                                                 | Non deve fare nulla, se non fornire l'indirizzo del destinatario |
| 6       | Presentazione         | Fornisce al livello 7 un canale **dedicato** (prenotato) a 2 **host** remoti e a 2 **processi**, steso tra due **porte**,  a basso BER, FIFO e immune dalla perdita di pacchetti per congestione della rete e sovraccarico del ricevente e immune dalla perdita di pacchetti per mancato accordo sulle fasi della trasmissione e con un formato comune del payload                                                                                                                 | Realizza una negoziazione dei formati del payload tra quelli disponibili ad entrambi gli interlocutori| 
| 5       | Sessione              | Fornisce al livello 6 un canale **dedicato** (prenotato) a 2 **host** remoti e a 2 **processi** e steso tra due **porte**, a basso BER, FIFO e immune dalla perdita di pacchetti per congestione della rete e sovraccarico del ricevente e immune dalla perdita di pacchetti per mancato accordo sulle fasi della trasmissione | La trasmissione viene suddivisa in fasi su cui esiste un accordo su quanto durino e come si raggiungano |                                                                                                            |
| 4       | Trasporto             | Fornisce al livello 5 un canale **dedicato** (prenotato) a 2 **host** remoti e a 2 **processi**, steso tra due **porte**, a basso BER, FIFO e immune dalla perdita di pacchetti per congestione della rete e per sovraccarico del ricevente | Esegue, tramite multiplazione TDM,  la condivisione del canale IP tra più segmenti provenienti da processi diversi, abbassa ulteriormente la BER, limita la velocità del mittente a beneficio del ricevitore e della rete intera, esegue il riordino dei segmenti fuori ordine|                                                                                                          |
| 3       | Rete                  | Fornisce al livello 4 un canale **dedicato** (prenotato) a 2 **host** fisicamente lontani (remoti), a basso BER, immune dal sovraccarico del ricevente e **steso** tra i due **host** remoti scegliendo autonomamente i link che fanno parte del percorso|      Realizza la localizzazione fisica del destinatario, il calcolo del percorso per raggiungerlo attraverso i nodi della rete e l'effettivo inoltro dei pacchetti IP da una porta di ingresso ad una di uscita di ciascun nodo lungo il percorso scelto.|                                                                         |
| 2       | Collegamento dati     | Fornisce al livello 3 un canale **dedicato** (prenotato) a 2 **host** fisicamente lontani (remoti) e **steso** tra 2 **IS vicini**, e con bassa BER e immune dal sovraccarico del ricevente  | Realizza, tramite multiplazione dinamica TDM, una condivisione del canale tra più nodi e una rilevazione e correzione di errore tramite CRC e una limitazione di velocità del trasmittente |
| 1       | Fisico                | Fornisce al livello 2 un canale di bit **dedicato** tra due **nodi IS** fisicamente vicini                                | Esegue una codifica/decodifica dei bit nei segnali fisici (elettrici, radio o ottici) adatti ad un certo mezzo fisico|

#### **Astrazioni topologiche**

Astrazioni interessanti sono quelle, di tipo **topologico**, che traducono una rete con molti **link fisici** in un unico **link logico** che li riassume tutti (astrazione). Questa cosa avviene tipicamente a:
- **livello 4**, dove una **intera rete IP**, composta da **link tra router** collegati, in genere, a maglia, viene astratta in un **link tra host** diretto (**Link End-to-End** virtuale come servizio di livello 4).
- In tutti gli **altri livelli** della pila OSI, canali punto-punto vengono astratti in altri canali **punto-punto** posti:
    - tra due **router vicini** (adiacenti) nei primi due livelli
    - tra i dispositivi **End to End** (host) nei livelli 5, 6 e 7.
  
Una **eccezione** notevole fanno le **LAN**, dove una **intera rete** LAN, composta di **link tra SW** collegati, in genere, ad albero, viene considerata come un link di livello 2 **diretto tra due router**. Ciò accade per una anomalia delle LAN in cui vi sono due **livelli di routing** complementari, di cui solo quello esterno **tra router** è, in realtà, **considerato** dall'architettura ISO/OSI:
- uno **esterno alla LAN** che si occupa della consegna dei pacchetti **tra i router di confine** delle LAN sorgenti e destinazione. Realizza un **inoltro** di **pacchetti IP** tra **router**.
- uno **interno alla LAN** che si occupa della consegna dei pacchetti tra l'**host sorgente** e il **router di confine** delle LAN **sorgente** e della consegna tra il **router di confine** della LAN di **destinazione** e l'**host** di destinazione. Realizza un **inoltro** di **trame MAC** tra **SW**.
  
#### **Astrazione di una rete a circuito**

Una rete a **commutazione di circuito** è realizzata soltanto con **multiplatori TDM statici** ed è assimilabile a una **cascata di slot** prenotati su **diverse tratte** (link) ed allocati ad una **sola comunicazione**.  

Dalla **tabella precedente**, si può osservare facilmente come alla fine, il **servizio offerto** dal livello applicativo della pila ISO/OSI, oltre a garantire l'**interoperabilità universale** tra i dispositivi, oltre ad implementare un **sistema aperto**, ha la possibilità di poter realizzare, a **livello logico** (cioè percepito), anche una **qualità del servizio** analoga a quella (a **prenotazione** statica) offerta da una **rete commutata** (rete a commutazione di circuito).  

La **percezione** di trovarsi in una rete a commutazione di circuito è solo **approssimata** ed è tanto più realistica quanto più grande è il numero dei nodi allocati e quanto maggiore è la banda installata sui canali, in modo che la **variabilità dei ritardi** (jitter) sulle **code** dei multiplatori statistici si mantenga **minima**. 

I **vantaggi**, incomparabilmente **maggiori**, che una rete a commutazione di pacchetto offre rispetto ad una a circuito (con multiplatori TDM statici) sono:  la **scalabilità** (ottenuta facilmente aggiungendo **nuovi nodi** e **nuovi link**) e l'**efficienza** (ottenuta grazie alla multiplazione statistica). 

Per contro, i canali realizzati con **multiplazione statica** mantengono ancora un'**affidabilità** e precisione temporale nella consegna (**jitter**) tutt'ora inegualiate dalle reti a pacchetto. Nonostante ciò, la **multiplazione statica** è oramai preferibile solo in applicazioni **di nicchia** in cui il **traffico prenotato** è sempre **costante** e mai **sprecato**. Ciò accade, ad esempio, nel mondo IoT quando è necessaria l'interrogazione periodica (**polling**) dei sensori, come quella effettuata, ad esempio, all'interno della zona CFP delle **supertrame** wireless.


#### **Canale reale**

Il **canale reale** è il **mezzo trasmissivo** che unisce il **livello fisico** dei due interlocutori. Si sviluppa in **orizzontale** lungo il mezzo trasmissivo ma anche in **verticale** lungo le interfacce tra i vari strati, **a scendere** in trasmissione, **a salire** in ricezione.
I messaggi dei vari livelli, **incapsulati** uno dentro l'altro, vengono trasmessi in un **blocco unico** lungo il **canale reale**. Ma per comodità, all'**interno dei dispositivi**, ogni **messaggio applicativo**  può essere visto **destrutturato** in una serie di messaggi **separati** lungo i 7 **canali virtuali** dei singoli livelli.  Il **messaggio parziale**, trasmesso e ricevuto lungo un livello, è inviato con il **formato** e con le **regole** di trasmissione (**protocollo**) propri di quel livello. 

Ogni volta che un canale virtuale invia una PDU  direttamente in **orrizontale** ad una entità pari attraverso un **canale virtuale**, in realtà la stessa PDU passa in **verticale** attraverso un SAP (service Access Point) al livello adiacente dove viene da questo presa in carico per creare i **servizi** che il livello superiore utilizza per portare avanti il suo protocollo.

I **servizi** di un livello N, esattamente come le sue N-PDU, sono standardizzati e vengono chiamati **primitive di servizio** di livello N. tutte le implementazioni di un protocollo di quel livello devono impegnarsi a realizzarle.

**In sostanza**, i messaggi che sono stati **generati** e **trasmessi separatamente** dai 7 livelli lungo i 7 **canali virtuali**, diventano un **unico messaggio matrioska** di 7 messaggi, **annidati** uno dentro l'altro, una volta che sono trasmessi lungo il **canale reale**. 

#### **Tipi di canale virtuale**

**Riassumendo**, i **canali virtuali non esistono fisicamente** ma sono **ugualmente reali** perchè, dal punto di vista dei **messaggi scambiati**, le **entità** pari (peer entity), cioè gli interlocutori corrispondenti nei vari livelli, si comportano come se essi ci fossero davvero e attraverso questi **effettivamente dialogassero**. Sono una **visione semplificata** con cui, ad un **livello superiore**, si possono **osservare** le funzioni svolte e i messaggi scambiati dai livelli inferiori. Semplificata, ma comunque reale.

<img src="isvses.png" alt="alt text" width="500">

**I canali virtuali** si dividono in:
- link **end to end**, chiamati così perché collegano livelli che sono presenti **soltanto** negli **host** cioè i **dispositivi terminali**, quelli su cui si **interfaccia l'utente**. I nodi **vicini** in questo collegamento virtuale soni i nodi **fisicamente più distanti**, in quanto la pila OSI realizza, a livello di servizio, un collegamento **virtuale **dedicato** e diretto** proprio tra nodi terminali.
- Più in basso (sui **primi 3** livelli), stanno i **link IS-IS** (collegamento tra  Intemediate systems), chiamati così perché collegano tra loro i **dispositivi di rete**, cioè quelli che creano la rete. Anche gli host posseggono questi livelli e pertanto sono, a tutti gli effetti, dispositivi di rete anch'essi (anche se privi di alcune funzioni importanti). I nodi **vicini** in questo collegamento virtuale soni i nodi della rete **fisicamente più prossimi**, in quanto la pila OSI realizza, a livello di servizio, un collegamento **virtuale **dedicato** e diretto** tra nodi vicini.

In figura la lettera H sta per **header** cioè intestazione (ad es. AH=Application header).

<img src="iso-osi.png" alt="alt text" width="1100">

### **Implementazione dei protocolli**

La responsabilità della implementazione dei protocolli OSI è ripartità tra vari moduli SW e da un livello HW:

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

### **Indirizzi e PDU**

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

### **Architettura di riferimento**

Lo **stack di protocolli OSI** è un **modello architetturale** di riferimento. Per **ogni strato** sono stati studiati nel tempo un gran numero di protocolli, ciascuno con i propri **pregi e difetti**. Un'**architettura reale**, quella che poi verrà standardizzata ed implementata in un dispositivo commerciale, per ogni strato della propria pila, sceglierà, tra i tanti disponibili in letteratura tecnica, un certo **tipo di protocollo** del quale realizzerà e standardizzerà la propria **particolare versione**. 

Attualmente, per **accedere ad Internet** e all'interno della maggior parte delle **reti locali LAN**, si adopera la cosiddetta suite **TCP/IP**. In ogni caso, molte **reti di sensori**, per funzionare localmente al loro ambiente di lavoro, **non sempre usano** la suite TCP/IP. Inoltre, anche le reti di sensori che l'adoperano, ai livelli inferiori come **i livelli data link e il livello fisico**, spesso utilizzano protocolli **diversi** da quelli che sono stati standardizzati per le LAN. 

[Dettaglio multiplazioni statiche](multiplazioni.md)

[Dettaglio TDM statico su mezzi punto-punto](tdmstatico.md) 

[Dettaglio tecnologie di accesso al mezzo radio](accessoradio.md)

[Dettaglio TDM statistico su mezzi punto-punto](tdmstatistico.md) 

>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)



