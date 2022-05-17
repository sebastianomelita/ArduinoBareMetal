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


### **Caratteristiche del TDM statico**


### **Ritardi**


### **TDM statico: riepilogo**

>[Torna a multiplazione TDM](mezzoradio.md#Multiplazione-TDM)


