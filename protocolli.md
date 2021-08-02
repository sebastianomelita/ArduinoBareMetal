>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)

## **Protocolli**

I protocolli sono **regole di comunicazione** che tutti gli **interlocutori** devono seguire per **portare avanti** la **comunicazione** su di un **canale**.
Un protocollo **definisce** il **formato** e l’**ordine dei messaggi** scambiati tra due o più entità di rete, le **azioni in risposta** ai messaggi ricevuti o **altri eventi**.
Sono valide per un **determinato canale** e si realizzano in **maniera software** (o al limite con logica digitale cablata) tramite programmi che girano sui nodi della rete.

Realizzano **algoritmi distribuiti** che comprendono:
- funzioni sui nodi della rete
- scambio di messaggi lungo i canali della rete

### **Formato messaggi**

Definisce **cosa** va comunicato, **come** e **quando** va comunicato. Gli **elementi chiave** di un protocollo sono: s**intassi**, **semantica**, **temporizzazione**.
<img src="MACframeEthernet.png" alt="alt text" width="600">

Il **formato** è definito indicando:
- il **significato** dei vari campi. Ad esempio **DA** rappresenta il Destination Address mentre **SA** rappresenta il Sorce Address. Il campo **Information** rappresenta il payload, cioè il carico utile, ovvero le informazioni che il protocollo deve spostare dalla sorgente alla destinazione. Tutto ciò che **sta prima** del campo paysload è l'**Header**, ovvero l'**intestazione** del messaggio detta anche **PCI (Protocol Control Information)**.
- la **lunghezza** dei vari campi, può essere:
    - **fissa**. E' misurata in byte ed è indicata con dei numeretti posti sopra il nome del campo che misurano.
    - **variabile**. Normalmente viene indicata la lunghezza minima e quella massima. Talvolta un campo aggiuntivo (Length in figura) è deputato proprio a contenere la lunghezza del payload 

Per **accedere** (in lettura o in scrittura) ad un **determinato campo** basta sommare la lunghezza dei campi che precedono il campo da accedere per determinare il suo **spiazzamento** dall'inizio del messaggio.

### **Classificazione protocolli**

I protocolli si possono **classificare** in:
- connessi e non connessi
- confermati e non confermati
- con stato e senza stato

### **Protocolli connessi e non**

I **protocolli connessi** sono sempre composti da **tre fasi**: 
- **apertura** (o setup): 
    - viene iniziata la connessione da uno degli interlocutori
    - vengono negoziati i parametri della connessione
    - vengono allocate (assegnate o prenotate) le risorse trasmissive (sui nodi e sui canali)
- **Dialogo**: vengono scambiati i messaggi (PDU) seguendo le regole del protocollo
- **Chiusura**: viene chiusa la comunicazione e vengono rilasciate le risorse precedentemente allocate.

I **protocolli non connessi** sono sempre composti da **una sola fase**: 
- **Dialogo o trasmissione**: vengono scambiati i messaggi (PDU) seguendo le regole del protocollo

### **Protocolli confermati e non**

I **protocolli confermati** sono protocolli in cui il **mittente** possiede un timer detto **timer di ritrasmissione** impostato ad un valore massimo di conteggio detto **timeout**. Il timer viene **avviato** al momento esatto dell'invio di un messaggio e viene **resettato** al momento della ricezione di un messaggio di conferma  di corretto arrivo a destinazione.

Il messaggio di **conferma** viene detto **ack** (acknowledgement) e viene inviato **dal ricevente** in direzione del **mittente**. Un ack **è sempre inviato** dal **ricevente** di un **precedente** messaggio, **mai** dal **mittente** di quel messaggio.

Se il **timer di trasmissione** è resettato **prima** dello scadere del timeout la trasmissione è considerata **avvenuta con successo**. Se invece allo scadere del timeout ancora **non si ricevono ack** allora il messaggio viene **dato per perso** ed è, dal mittente, **ritrasmesso**.

### **Protocolli con stato e non**
con 
Un protocollo si dice stateful cioè con stato quando vale una dele due situazioni che rappresentano due facce diverse di una stessa medaglia:
- un messaggio scambiato in un certo momento è correlato con gli altri messaggi scambiati in precedenza
- all'arrivo di ogni messaggio viene aggiornato un elemento di memoria (variabile) che riassume la storia dell'evoluzione del protocollo nel tempo.

La storia del protocollo è detta anche sessione, per cui vale anche la definizione: un protocollo stateful è un protocollo di comunicazione in cui il destinatario può mantenere lo stato di sessione dalle richieste precedenti.

Mentre al contario: un protocollo stateless è un protocollo di comunicazione in cui il destinatario non deve mantenere lo stato della sessione dalle richieste precedenti. Il mittente trasferisce lo stato di sessione rilevante al destinatario in modo tale che ogni richiesta possa essere intesa isolatamente, ovvero senza riferimento allo stato di sessione dalle richieste precedenti conservate dal destinatario.









>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)
