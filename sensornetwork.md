**Reti ethernet di ufficio**

Le reti di ufficio sono realizzate con tecnologie ethernet a topologia fisica a stella o a stella gerachica (albero). Lo switch ha la funzione dI:
- **concentratore di dispositivi**. Un collegamento punto punto è dedicato ad ogni dispositivo che occupa esattamente una porta sul concentratore. Servono tante porte quanti sono i dispositiviin prossimità del concentratore.
- **commutazione** cioè smistamento dei dati lungo le dorsali della rete fino ai router di confine della stessa (gateway).

Lo switch è generalmente regolato dal protocollo STP che, secondo lo standard, limita i collegamenti a cascate a poche unità (profondità nominale di 3 dispositivi secondo standard EIA/TIA).
Il cablaggio può risultare oneroso in presenza di un elevato numero di dispositivi, ad esempio, un cavo per ogni sensore. Motivo per cui dispositivi ed architettura sono generalmente ritenute poco adatte per le ampie reti di sensori comuni in ambito industriale.

**Reti ethernet industriali**

Le **reti industriali o ferrotramviarie** che utilizzano la tecnologia ethernet sono spesso realizzate con **topologia fisica a BUS o ad anello**.
Un anello è composto da lunghe cascate di switch equipaggiati con protocollo STP modificato o con altri protocolli proprietari.
Il cablaggio è più economico in presenza di un cluster numeroso di dispositivi dato che con un unico cavo si possono collegare più switch.
Possibilità di topologie ridondate a doppio anello (treni, industria)

![industrialnet](industrialnet.jpg)



**Reti di sensori**

Spesso sono reti miste cioè composte da sottoreti eterogenee.

![sensor network](sensornet1.png)

La **rete principale** è ethernet con dorsali fisiche a stella cablate e collegamenti periferici cablati o wireless WiFi

Una o più sottoreti di sensori (cablate o wireless) con interfacce non ethernet spesso in topologia fisica a BUS:
- Necessario un **gateway** di confine avente una **interfaccia nella rete principale** con possibili funzioni di:
        - Traduzione di formato dei dati da rete a bus a rete ethernet
        - Interrogazione periodica (polling) dei dispositivi (master di una architettura master/slave)
        - Raccolta e memorizzazione delle informazioni per essere trasferite in un secondo momento al server di gestione

Sottoreti di sensori cablati o wireless con interfacce ethernet:
- Non è necessario alcun gateway di traduzione dato che i sensori si collegano direttamente ad una porta di uno switch oppure ad un AP WiFi
   

In ogni caso è necessario un **server di gestione** con funzioni di:
- Processamento (elaborazione nuovo stato e comando attuatori)
- Memorizzazione (storage) ed estrazione (mining) delle informazioni
- Analisi dei dati per estrarre reportistica di aiuto alle decisioni (risparmio energetico)
- Pubblicazione in Internet delle informazioni su un un sito o su un WebService (opendata)
- Segnalazione anomalie
- Backup dei dati e gestione disaster recovery di dati e servizi
     
Spesso i **sensori** o gli **attuatori** non sono dotati di interfaccia ethernet e sono organizzati in **sottoreti apposite (ad hoc)** cablate con interfacce industriali (Dallas, I2C, SPI, Modbus, Profibus, ecc.). Queste sono spesso caratterizzate dal fatto di possedere una **topologia fisica a BUS o ad anello** che possiede il vantaggio di interconnettere **molti dispositivi** (sensori o attuatori) sfruttando l'utilizzo di **un solo cavo**. 

E' comune anche una topologia di accesso a **bus wireless** in cui la contesa del mezzo è regolata da protocolli di arbitraggio del tipo **ALOHA** O **CSMA/CA**. Questo è il caso di reti di sensori wireless potenzialmente compostre da molti nodi aventi anche capacità di smistamento gestite in ogni aspetto da tecnologie complesse come Zigbee o BLE che definiscono sia i dettagli delle interfacce radio che i protocolli di gestione dei canali e delle risorse sui nodi.

Le reti di **sensori wireless** hanno una estensione nello spazio variabile e la loro toplogia preferita è a stella o a maglia.

![hops](hops.png)

A seconda dello schema adoperato è interessante dal **punto di vista energetico** considerare se un comando o l'accesso ad un sensore avvenga connettendosi direttamente ad un dispositivo hub centrale o attraverso una cascata di nodi:
- **Hop singolo**: ciascun nodo del sensore è collegato al gateway, sebbene sia possibile la trasmissione a lunga distanza, il consumo di energia per la comunicazione sarà significativamente superiore alla raccolta e al calcolo dei dati. 
- **Hop multiplo**: esiste un percorso verso i nodi terminali passando attraverso dei nodi intermedi. L’energia per raggiungere nodi distanti si limita a quella per raggiungere i nodi vicini. Su reti mesh di grandi dimensioni anche questa energia può essere non trascurabile. Su reti indoor è invece la soluzione decisamente più conveniente.

**Canali di comunicazione principali**

Sono necessari almeno due canali di comunicazione tra sensori e gestore delle informazioni:
- tra sensori e gateway verso la LAN:
    - A filo con accesso:
        - dedicato: un filo o un canale per sensore (SDM, TDM)
        - multiplo a mezzo condiviso (BUS) di tipo master slave (Modubus) o peer to peer (CanBUS, KNX, ecc) o misto (ProfiBUS). 
        - Spesso bidirezionale specie se in presenza di attuatori
        
    - Senza filo cioè wireless con accesso:
        - singolo dedicato: link punto-punto analogico digitalizzato con AX25 oppure digitale con un radio modem (Yarm ACME Systems, 6LoWPAN, LoRa) resi full duplex con FDM o TDM.
        - Multiplo e condiviso (BUS) di tipo half duplex reso bidirezionale (full duplex) tramite tecniche asincrone CSMA/CA (Zigbee, wifi, LoRa) o sincrone TDMA (Zigbee, Bluetooth).
        - 
- Tra gateway e gestore delle informazioni:
     - Tipicamente tramite LAN ethernet e architettura Client/Server
     - Interazioni di tipo PUSH o PULL
     - Paradigma Request/Response (HTTPS, COAP), Publish/Subscriber (MQTT) oppure canale persistente bidirezionale (BSD socket o WebSocket)







    




