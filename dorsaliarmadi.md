>>[Torna reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 




## **Albero degli apparati passivi** 

- È una rappresentazione logica di armadi,  dorsali e TO che ha il vantaggio di riassumere sinteticamente la struttura generale del cablaggio.
- La topologia è ad albero avente per radice l’armadio CD, per nodi gli armadi BD e FD, per foglie le prese TO. 
- I rami dell’albero sono le dorsali intese come singolo cavo di molteplicità base (4 per il doppino, 2 per la fibra). 
- L’architettura ha una profondità massima di tre livelli (campus, edificio, piano).
- Vanno riportati ed etichettati tutti gli armadi. Per quanto riguarda i TO vanno riportati ed etichettati il primo e l’ultimo.
- Ogni nodo DEVE diramare almeno verso altri due nodi di livello inferiore (se non è un ramo cioè una semplice giunzione tra due cavi)

<img src="img/passivi.png" alt="alt text" width="1000">

## **Tabella delle dorsali** 

Serve stabilire qualità e quantità dei cavi permanenti (installati nell’edificio) impiegati nel cablaggio.

Deve definire perlomeno le seguenti proprietà:
- Sigla del cavo, tipicamente un struttura X-Y dove X è la sigla dell’armadio di livello superiore a cui è collegato un capo del cavo, mentre Y è la sigla dell’armadio di livello inferiore (o del TO) a cui è collegato l’altro capo.
- Tipo di cavo. MMF o SMF per la fibra, UTP, FTP o STP per il doppino
- Categoria. OM1, OM2 , OM3 , OM4 , OM5 per la fibra MMF. Oppure OS1, OS2 per SMF. Cat 5e, cat 6, cat 6A, cat 7, cat 7A, cat 8 per il doppino.
- Molteplicità. Il numero di sottocavi inseriti in divisione di spazio (SDM) dentro il cavo di dorsale. In numero pari, tipicamente 2, 4, 6, 8, 12 per la fibra. Sempre 4 doppini trecciati per un cavo UTP, FTP o STP.
- Lunghezza. Misurata in metri e desumibile ad occhio dal percorso del cavo per come appare in planimetria. Stimata individualmente per ogni dorsale se questa è di primo o secondo livello (campus e piano), tipicamente stabilita in maniera forfettaria moltiplicando il numero di TO per una lunghezza media che può essere desunta approssimativamente dalla planimetria oppure stimata secondo un valore medio valido per l’Italia pari a 30m.
- Ruolo. Scopo per il quale è previsto quella dorsale. 

### **Cablaggio di campus** 

<img src="img/tabcampus.png" alt="alt text" width="700">

Nelle ultime due righe della tabella apparentemente ci sta un collegamento tra due armadi di pari livello BD, soluzione normalmente non permessa dallo standard. In realtà, si tratta di una semplice giunzione tra una delle coppie fibre che arrivano ad es su BD2 con una coppia di fibre che va verso BD9 allo scopo di prolungarla verso quell’armadio. La coppia di fibre in questione non viene permutata su BD2 ma solo su BD9. E’ una soluzione tecnica per riutilizzare su BD9 una fibra altrimenti sprecata sull’armadio BD2.

### **Cablaggio di edificio**

<img src="img/tabedificio.png" alt="alt text" width="700">

Si noti che gli AP sono stati considerati afferenti agli armadi di edificio piuttosto che a quelli di piano. Questa soluzione è particolarmente giustificata se gli AP realizzano una copertura ad alta densità/capacità verso client che consumano una grande mole di dati (bit rate elevata)

In questo caso un AP ha attualmente (WiFi6 e WiFi AC) una collegamento ad 1Gbps. Una costellazione ad alta densità potrebbe aggregarne anche una decina, per cui l’armadio che li serve dovrebbe avere una dorsale di almeno 10Gbps.

### **Cablaggio di piano**

<img src="img/tabepiano.png" alt="alt text" width="700">

>[Torna reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

