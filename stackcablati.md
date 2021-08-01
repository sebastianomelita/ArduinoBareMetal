>[Torna a reti di sensori](sensornetworkshort.md)

## **Stack cablati specifici per domotica e sensoristica**

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

>[Torna a reti di sensori](sensornetworkshort.md)
