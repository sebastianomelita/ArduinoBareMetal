>[Torna all'indice generale](index.md)

## **Reti ethernet da ufficio**

Le **reti di ufficio** sono realizzate con tecnologie ethernet a topologia fisica a stella o a stella gerachica (albero). Lo switch ha la funzione dI:
- **concentratore di dispositivi**. Un **collegamento punto punto** è dedicato ad ogni dispositivo che occupa esattamente una porta sul concentratore. Servono tante porte (e tanti cavi) quanti sono i dispositivi in prossimità del concentratore. L'uso esclusivo di un cavo da parte di un dispositivo è detto **microsegmentazione** e di fatto azzera il problema delle collisioni. Se la porta è condivisa da più dispositivi tramite un BUS o un HUB si dice che lo switch realizza una **macrosegmentazione**, situazione in cui le collisioni ci sono e sono arbitrate dal protocollo Ethernet (di tipo CSMA/CD).
- **commutazione** cioè smistamento dei dati lungo le dorsali della rete fino ai router di confine della stessa (gateway).

Lo **switch** è generalmente regolato dal **protocollo STP** che, secondo lo standard, limita i collegamenti a cascate a poche unità (profondità nominale di 3 dispositivi secondo standard EIA/TIA).
Il cablaggio può risultare **oneroso** in presenza di un elevato numero di dispositivi poichè richiederebbe l'impiego di un cavo a parte per ogni sensore. Per questo motivo dispositivi di commutazione e architettura **tradizionali** sono generalmente ritenuti poco adatti per la realizzazione delle ampie reti di sensori diffuse in ambito industriale.

## **Reti ethernet industriali**

Le **reti industriali o ferrotramviarie** che utilizzano la tecnologia ethernet sono spesso realizzate con **topologia fisica a BUS o ad anello**.
Un anello è composto da lunghe cascate di switch equipaggiati con protocollo STP modificato o con altri protocolli proprietari.
Il cablaggio è più economico in presenza di un cluster numeroso di dispositivi dato che con un unico cavo si possono collegare più switch.
Possibilità di topologie **ridondate a doppio anello** (treni, industria)

![industrialnet](industrialnet.jpg)

## **Reti di sensori e attuatori**

Spesso sono **reti miste** cioè composte da sottoreti eterogenee.

![sensor network](sensornet1.png)

La **rete principale** è ethernet con dorsali fisiche a stella cablate e collegamenti periferici cablati o wireless WiFi

Ad essa si collegano una o più **reti secondarie di accesso** per i dispositivi sensori o attuatori con **interfacce** di tipo:
- **Non ethernet** (cablate o wireless) spesso in topologia fisica a BUS:
    - Necessitano di un **gateway** di confine avente almeno una **interfaccia ethernet nella rete principale** con possibili funzioni di:
        - **Traduzione di formato** dei dati da rete a bus a rete ethernet
        - Interrogazione periodica (polling) dei dispositivi (master di una architettura master/slave)
        - Raccolta e memorizzazione delle informazioni per essere trasferite in un secondo momento al server di gestione

- **Ethernet**. Non è necessario alcun gateway di traduzione dato che uno o più sensori si collegano direttamente ad una porta di uno switch di accesso oppure ad un AP WiFi. 
    
## **Server di gestione**

In genere è localizzato all'intermo della **rete principale** con una collocazione **on-premise** all'interno del sistema.

Tendenze sempre più diffuse portano al **trasferimento crescente di funzioni** anche sul **cloud**. Una **soluzione estrema** è quella di spostare **tutte le funzioni** sul cloud tenendo ben presente che un **guasto della connessione ad internet** causerebbe una cessazione delle **funzioni** di regolazione e controllo che sono state progettate per essere eseguite **centralizzate sul server**. Le funzioni gestite in maniera autonoma e peer to peer **completamente a bordo** dei dispositivi non dovrebero risentire di particolari problemi.

In ogni caso è necessario un **server di gestione** con funzioni di:
- Processamento (elaborazione nuovo stato e comando attuatori)
- Memorizzazione (storage) ed estrazione (mining) delle informazioni
- Analisi dei dati per estrarre reportistica di aiuto alle decisioni (risparmio energetico)
- Pubblicazione in Internet delle informazioni su un un sito o su un WebService (opendata)
- Segnalazione anomalie
- Backup dei dati e gestione disaster recovery di dati e servizi
- Gestione dei FW dei vari dispositivi
- Gestione della sicurezza
     
##  **Considerazioni energetiche nelle reti di sensori**     
 
Le reti di **sensori wireless** hanno una estensione nello spazio variabile e la loro toplogia preferita è a stella o a maglia.

![hops](hops.png)

La rete principale ethernet può **estendersi nello spazio** secondo un'architettura (ad albero o ad anello) che può comprendere molti switch. Lo **smistamento completo** di una trama dati lungo la rete può prevedere una catena di smistamenti lungo gli switch (hop) più o meno lunga. Nel caso di una rete ethernet questo non è generalmente un problema eccessivo nè dal punto di vista dei ritardi nè rispetto a quello dei consumi (gli switch sono alimentati attraverso la rete elettrica).

La situazione può essere diversa nel caso delle **reti di sensori**. Queste sono spesso **wireless** e realizzate con dispositivi **alimentati a batteria** che si vorrebbe fosse sostituita idealmente  **non prima di un paio di anni**.

A seconda dello schema adoperato, dal **punto di vista energetico** non è indifferente considerare se **un comando** o **l'accesso in lettura ad un sensore** avvengono connettendosi direttamente **all'unico** dispositivo hub centrale (magari distante) o connettendosi **al più vicino** di una cascata di nodi:
- **Hop singolo**: ciascun nodo del sensore è collegato all'**unico gateway centrale** posto generalmente in **posizione baricentrica** rispetto a tutti i dispositivi. Sebbene sia possibile la trasmissione a lunga distanza, il consumo di energia sarà significativamente superiore a quella spesa per la misura del valore del sensore e per la sua elaborazione. 
- **Hop multiplo**: esiste un percorso verso i nodi terminali passando attraverso dei nodi intermedi. L’energia che un **singolo dispositivo** impiega per raggiungere **un nodo remoto** (ad esempio per accenderlo) si limita a quella per raggiungere il **nodo più prossimo**. Su reti mesh (magliate) di **grandi dimensioni** anche questa energia può essere non trascurabile. Su **reti indoor** è invece la soluzione decisamente **più conveniente** dato che spesso **il primo** dispositivo di smistamrnto raggiungibile (router o switch) spesso non è a più di un metro di distanza mentre quello in **posizione baricentrica** può essere anche a 10-50m di distanza.

Un'altra funzione **potenzialmente energivora** è il **polling dei sensori** ovvvero la loro lettura periodica con annessa trasmissione in remoto dei dati. In questo caso se il **primo nodo** di smistamento della catena è parecchio distante (il caso di tenologie outdoor come LoraWan o Sigfox) o sebbene indoor si adopera una **trasmissione** in una **tecnologia  energivora** (come è in modalità standard il WiFi) allora sono possibili almeno due **soluzioni operative**:
- **allungare l'intervallo di polling** facendolo passare dall'ordine dei secondi a quello dei minuti o delle ore.
- **memorizzare le misure in locale** sul dispositivo e, ad intervalli regolari adeguatamente lunghi, inviare dei **dati aggregati nel tempo** come **medie e varianze** o statistiche in genere.

##  **Canali di comunicazione principali**

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


>[Torna all'indice generale](index.md)
    




