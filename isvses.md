>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)

## **End Systems ES**

Gli **utenti** della rete sono le entità mittenti o destinatarie dei dati, possono essere utenti umani o agenti software cioè programmi in esecuzione (detti processi)
Gli utenti della rete stanno su **macchine** (hosts) dette **End Systems o ES** (sistemi finali o terminali) perché sono queste le macchine che stanno **ai capi** di una **comunicazione**.
Gli End System possiedono la proprietà fondamentale di essere **individuabili univocamente** all’interno della rete tramite un **indirizzo**.

Gli End Systems possono essere:
- **Computers**
- Periferiche intelligenti (telecamere, stampanti di rete, sensori, ecc) : 
    - dotate della logica necessaria a svolgere i protocolli di comunicazione 
    - dotate di indirizzo.

## **Intermediate Systems IS**

Sono i **dispositivi interni** della rete e sono in genere **nodi di commutazione**
I **nodi di commutazione** sono dispositivi di **smistamento** dove l’informazione entra da una **porta di ingress** ed esce da una **porta di uscita**.
Le **porte** di uscita e di ingresso **tra un nodo e l’altro** sono collegate da **mezzi trasmissivi** detti **collegamenti o link**
La **cascata di nodi e link** tra un ES **sorgente** e il corrispondente ES di **destinazione** viene detto **percorso del messaggio o path**.
L’operazione di **smistamento** da una porta di ingresso ad una di uscita viene detto **inoltro o forwarding**.
Il processo di **scelta del percorso migliore** tra ES sorgente e ES di destinazione viene detto **instradamento o routing**.


<img src="isvses.png" alt="alt text" width="700">

La **comunicazione diretta ES-ES** è un collegamento **logico (o virtuale)** che appare **dedicato** ai due ES coinvolti nel dialogo.
Il collegamento logico tra ES si scompone in **più collegamenti fisici** tenuti insieme da nodi IS
I **collegamenti tra nodi IS** sono **risorse comuni** a molte comunicazioni.

I nodi di IS di commutazione (switch o router) partecipano attivamente nel realizzare la **privacy** dell'utente perchè, nell'ultimo smistamento, quello verso un ES, inoltrano i dati solo a quel nodo ES a cui sono destinati, questo perchè il link tra un nodo e il suo IS è sempre **dedicato** (punto-punto). La stessa cosa non accade quando il collegamento tra un nodo IS e un ES avviene con **mezzi broadcast** come i **BUS** dove un unico filo collega molti ES.


>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)
