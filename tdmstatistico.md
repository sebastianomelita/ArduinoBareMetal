>[Torna a multiplazione TDM](mezzoradio.md#Multiplazione-TDM)


## **Commutazione di pacchetto**

**Idea di base**

La commutazione di circuito prevede di allocare rigidamente delle risorse ad una comunicazione su base richiesta effettuata in fase di set-up (apertura) del canale: l’efficienza può essere bassa dato che impegno il canale anche quando non dico niente.

Idea per migliorare la situazione:
- Spezzo l’informazione in più segmenti
- Trasmetto un segmento, impegnando le risorse, solo quando questo è “pieno”

La commutazione di pacchetto prevede di allocare dinamicamente delle risorse a diverse comunicazioni, sempre su base richiesta, ma effettuata in fase di trasmissione sul canale: efficienza molto elevata perchè impegno il canale solo quando devo dire qualcosa.

**TDM dinamico**

E' un **TDM**, per cui i messaggi di comunicazioni diverse:
- arrivano in **ingresso** tutte nello stesso tempo ma su **porte diverse** (SDM)
- vengono trasmessi in **uscita** sulla stessa porta ma in **tempi diversi** (TDM)

L’**allocazione** delle comunicazioni nel tempo è:
- **non prestabilita**, cioè può cambiare all’interno della medesima comunicazione (TDM dinamico)
- **aperiodica**, le comunicazioni, nel flusso dati, non hanno una posizione regolare loro assegnata nel tempo
- **non esclusiva**. Il canale è di una certa sorgente per tutto il tempo (slot) in cui è ad essa assegnato ma la durata di questa assegnazione è limitata alla durata della trasmissione del **singolo pacchetto**

Il **momento** della trasmissione di un messaggio non è prevedibile a priori perchè dipende dal tempo di arrivo del messaggio stesso e da quanti messaggi ci stanno prima di lui nella coda di trasmissione. In generale, il tempo di coda è un evento prevedibile solo in termini statistici.

### **Struttura**

Ogni **messaggio** lungo (stream o file) è diviso **dalla sorgente** in unità dati più corte dette **pacchetti** (packets) mediante un processo detto **segmentazione**. La **lunghezza** in bit dei pacchetti è **variabile** (entro un valore massimo) e non è nota in anticipo al multiplatore.

Un **multiplatore** è realizzato da una **coda** che accumula i pacchetti che **attendono la trasmissione** sul canale.

Il **tempo di riempimento** delle code di ingresso non è istantaneo ma dipende dalla **velocità della sorgente** che è al multiplatore statistico del tutto **ignota**. Potrebbero arrivare **contemporaneamente** o **separatamente** o anche insieme ma **parzialmente sovrapposte** cioè sfalsate temporalmente l'una dall'atra cosìchè il caricamento di una per un po' si sovrappone con quello di un'altra.

I pacchetti in arrivo vengono ospitati sulle code **di ingresso**.
Non appena un pacchetto completa il suo arrivo viene trasferito dalla sua coda di ingresso su quella di uscita per risolvere il problema della contesa del canale.

La **coda di ingresso** contiene esattamente un pacchetto, mentre **la coda di uscita** può ospitare molti pacchetti. L'**accumulo** dei pacchetti sulla coda di uscita può essere dovuto:
- all'**arrivo simultaneo** di un pacchetto per ogni porta di ingresso
- **picco di velocità** su una porta di **ingresso** che, essendo eccessivo, **satura** la velocità di smaltimento della porta di uscita che pertanto è costretta ad accumulare pacchetti sulla coda antistante ad essa. La coda in questo caso fa da ammortizzatore che assorbe il picco momentaneo in attesa che questo si estingua nei momenti in cui arriveranno meno pacchetti.

l’ordine di trasmissione è uguale all’ordine di arrivo nella coda
Una volta arrivato il loro turno, i pacchetti vengono spediti alla massima velocità consentita dal canale
Vincolo operativo: la velocità di trasmissione sul canale di un singolo pacchetto deve essere (a regime) almeno la somma delle velocità di tutte le sorgenti



La  contesa del canale si risolve mediante accodamento dei pacchetti delle varie sorgenti al suo ingresso.
le sorgenti impegnano il canale solamente quando hanno dei pacchetti pronti per la trasmissione (allocazione dinamica) 
Non esiste una trama dato che l’ordine di trasmissione non identifica la sorgente: necessità di una  etichetta
Le sorgenti negli istanti in cui non trasmettono non impegnano il canale.
Ciascuna sorgente trasmette il suo pacchetto con una velocità indipendente da quella di ciascun altra e poi lo invia nella coda 


### **Caratteristiche del TDM statico**


### **Ritardi**


### **TDM statico: riepilogo**

>[Torna a multiplazione TDM](mezzoradio.md#Multiplazione-TDM)


