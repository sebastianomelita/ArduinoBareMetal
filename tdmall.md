

**Metafora 1**

Una casa vacanze in un luogo di villeggiatura esclusivo, molto ambito e caldo per 365 giorni l’anno è particolarmente costosa ed è mediamente utilizzata per una sola settimana.
Si decide di abbattere i costi di acquisto offrendola in multiproprietà, forma di acquisto che ne consente un utilizzo efficiente per tutto l’anno.
L’anno è diviso in 40 settimane utili per l’affitto, le altre sono dedicate alla manutenzione dell’immobile.
Le settimane utili all’affitto di un anno solare sono numerate dalla 1 alla 40.

Ogni famiglia prenota l’immobile per una settimana l’anno prestabilita e segnata in una tabella di allocazione che si ripete uguale ogni anno.
Tutte le famiglie attendono un anno il loro momento a casa loro (coda di ingresso diversa per ciascuna)
Ogni famiglia occupa la stessa casa vacanza (canale) una volta l’anno in una settimana prestabilita (slot).
L’anno di vacanza parte dalla settimana 1 fino alla 40 e si ripete uguale per sempre.

**Metafora 2**

In un evento informativo più interlocutori devono condividersi la scena di un auditorium con pubblico che ascolta il dibattito.
Un numero arbitrario di interlocutori parlano a turno secondo un criterio di dialogo assimilabile a quello della "tavola rotonda". Ciascun interlocutore è numerato. Il numero è un numero d'ordine all'interno della scaletta degli interlocutori abilitati a parlare.
Gli interventi sono suddivisi in round. Dentro ciascun round parlano, a turno, tutti i partecipanti al dibattito seguendo, ad ogni round, sempre la stessa scaletta.
Tutti i relatori parlano per lo stesso tempo (detto slot). La scaletta rappresentata nel tempo e limitata agli slot di uno stesso round è detta trama. 
All'interno della trama, ogni slot segue la numerazione crescente del partecipante a cui è assegnato. 
La trama viene ripetuta periodicamente in un tempo detto periodo di trama.

E' possibile riconoscere il relatore senza vederlo contando gli slot dall'inizio della trama. 
Ogni relatore ha un tempo di trama per preparare la risposta alla domanda del round ma solamente un tempo di slot per esporla.
La multiplazione statica TDM di parecchie sorgenti richiede però che tutti i relatori siano sincronizzati con precisione presso un moderatore che fa da master e detta i tempi. 

Il nodo master stabilisce la composizione della "tavola rotonda" (cioè chi può trasmettere), in che ordine (cioè la scaletta degli interventi) e per quanto tempo deve durare un intervento. 

<img src="tdmfisso.png" alt="alt text" width="700">

I messaggi arrivano simultaneamente ma su N porte di ingresso diverse (SDM) e si caricano sul buffer a valle di ciascuna dopo un tempo T.

Una volta finito il caricamento, gli N messaggi vengono inviati, in momenti diversi (TDM), nell’unica porta di uscita e con velocità maggiore di quella di arrivo in modo che la trasmissione di tutti avvenga sempre nel tempo T di arrivo di uno solo. 

Il tempo T viene detto tempo di trama o periodo:
è suddiviso in slot numerati di ugual durata sui quali è allocata la trasmissione di tutte le sorgenti in successione, una dopo l’altra. 
- Ogni slot ha la durata di T/N ed è assegnato sempre alla stessa sorgente che lo ha prenotato in fase di setup del canale.
- La trama si ripete all’infinito sempre con lo stesso ordine di trasmissione delle sorgenti e con ugual durata T.

I messaggi arrivano ad ondate successive. Mentre si trasmette quella appena ricevuta si riceve la successiva. Il ritardo tra il momento della ricezione di un messaggio e quello della sua trasmissione è esattamente T.

T è anche l’intervallo di tempo che intercorre tra slot dedicati alla trasmissione di messaggi della stessa sorgente




