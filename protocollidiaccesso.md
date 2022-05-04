>[Torna a multiplazione TDM](mezzoradio.md#Multiplazione-TDM)

### Mezzi a BUS

Un **mezzo trasmissivo** si dice a **BUS** quando più periferiche condividono lo stesso collegamento. Si può realizzare sia con un un **unico filo** in rame che collega tutti i dispositivi, che con un mezzo radio su cui tutti i dispositivi condividono lo **stessa frequenza** (canale) di trasmissione.

I mezzi a BUS posseggono la **proprietà** che se un interlocutore parla **tutti** gli altri ascoltano perchè un messaggio inviato da uno **passa** comunque per tutti gli altri.

### Collisioni

I mezzi a BUS sono afflitti dal **fenomeno** delle **collisioni**.

Una **collisione** è l'evento di **sovrapposizione distruttiva** di due o più messaggi su un canale. E' un evento **irreversibile** perchè non permette il recupero dell'informazione trasportata originariamente dai singoli messaggi.

Una collisione su un mezzo a BUS **accade** quando due o più interlocutori **parlano contemporaneamente**. 

Nascono adesso due problemi: rilevare una collisione, evitare una collisione. 

Per **evitare** le collisioni servono i **protocolli di arbitraggio** che sostanzialmente cercano di far parlare gli interlocutori sempre **uno alla volta**. I protocolli di arbitraggio si dividono in **master/slave** o centralizzati e in **peer to peer** o distribuiti. 

### Protocolli di arbitraggio

Nei protocolli di arbitraggio  **master slave** esiste una stazione centrale di livello gerarchico superiore detta master che, per conto delle stazioni slave, decide **chi** deve parlare e **quando** deve parlare evitando sovrapposizioni nel tempo dei messaggi.  

Nei protocolli di arbitraggio  **peer to peer**, o distribuiti, le stazioni sono tutte di pari livello e sono programmate per interpretare continuamente la situazione del canale in modo da parlare sempre una alla volta. 

I protocolli di arbitraggio distribuiti si dividono ulteriormente in **deterministici** e in **a contesa**. 

Quelli **deterministici** eliminano alla radice la possibilità di collisioni realizzando un meccanismo di **cooperazione** tra le stazioni tramite messaggi di controllo con cui esse comunicano sia l'**intenzione** di trasmettere che il **diritto** di accesso al canale. Il **diritto** si acquisisce o si cede mediante il trasferimento di una informazione particolare detta **token**. Il token può essere **posseduto** quando si ha la necessità di trasmettere, ma solo da una stazione alla volta, e deve essere immediatamente **ceduto** ad un'altra stazione quando la stazione che lo deteneva ha finito di trasmettere. 

Quelli **statistici** o **a contesa** realizzano una **competizione** per l'accesso al mezzo che deve essere regolata dalle singole stazioni semplicemente osservando il canale in maniera indipendente l'una dall'altra, senza coordinamento alcuno. Questa categoria di protocolli **limitano statisticamente** il problema delle collisioni ma **non lo annullano** completamente, per cui le collisioni necessitano ancora di **essere rilevate** dalle singole stazioni.

### Come rilevare una collisione?

- Si ricorre all’utilizzo di un protocollo di trasmissione confermato
- I **protocolli confermati** sono protocolli in cui **il mittente** possiede un timer, detto **timer di ritrasmissione**, impostato ad un valore massimo di conteggio detto **timeout**.
- Il timer viene **avviato** al momento esatto dell'invio di un messaggio e viene **resettato** al momento della ricezione di un messaggio di conferma di corretto arrivo a destinazione.
- Il messaggio di conferma viene detto **ack** (acknowledgement) ed un messaggio di **controllo** (non dati) che viene inviato **dal ricevente** in direzione del mittente. Un ack è sempre inviato dal ricevente di un precedente messaggio, mai dal mittente di quel messaggio.
- Se il timer di trasmissione è resettato **prima dello scadere** del timeout la trasmissione è considerata avvenuta con **successo**. Se invece allo scadere del timeout ancora non si ricevono ack allora il messaggio viene dato per perso ed è, dal mittente, **ritrasmesso**.

### Come reagire a fronte di una collisione?

Se tutte le stazioni ritrasmettono nello stesso istante collidono immediatamente e il messaggio viene perso.

Soluzione: **Backoff**
- È un tempo **casuale**
- È calcolato in maniera indipendente, l’una dall’altra, da tutte le stazioni
- Serve a determinare la stazione che deve trasmettere **per prima** minimizzando il rischio di trasmissioni contemporanee
- È calcolato all’interno di un intervallo detto **finestra di contesa**

## **ALOHA**

### **Fasi ALOHA**
Una **stazione trasmittente**:
- al momento che ha una trama pronta, la invia senza aspettare.
- Dopo l’invio aspetta per un certo tempo (detto **timeout**) lo scadere di un timer (detto **timer di trasmissione**).
- Se essa riceve il messaggio di ack allora la trasmissione è avvenuta con successo.
- Altrimenti la stazione usa una strategia di backoff e invia nuovamente il pachetto.
- Dopo molte volte che non si ricevono conferme (acknowledgement) allora la stazione abbandona l’dea di trasmettere.

**Le collisioni graficamente:**

<img src="alohacollisioni.jpg" alt="alt text" width="600">

**Protocollo ALOHA in trasmissione pseudocodice:**

```C++

N=1;
while(N <= max){
	send(data_frame);
	waitUntil(ackOrTimeout());
	if(ack_received){
		exit while;
	}else{
		/* timeout scaduto: ritrasmissione*/
		t=random();
		wait(t);
		N=N+1;
}
/* troppi tentativi: rinuncio a trasmette

```
**Protocollo ALOHA in ricezione pseudocodice:**

```C++
While(true){
	WaitUntil(dataFrameArrived());
	if(!duplicate()){ 
		deliver(frame) 
	}
	send(ack_frame);
}

```
<img src="alohaflow.png" alt="alt text" width="400">

>[Torna a multiplazione TDM](mezzoradio.md#Multiplazione-TDM)
>
## **CSMA**

### **Fasi CSMA**

Significa Carrier Sensing Multiple Access cioè protocollo di Accesso Multiplo con Ascolto della Portante (prima della trasmissione)
Una stazione trasmittente: 
- al momento che ha una trama pronta, aspetta finchè non “sente” il canale libero (cioè nessuno trasmette).
- Appena essa rileva il canale libero invia immediatamente la trama.
- Dopo l’invio aspetta per un certo tempo.
- Se essa riceve il messaggio di ack allora la trasmissione è avvenuta con successo.
- Altrimenti la stazione usa una strategia di backoff e invia nuovamente il pachetto dopo un tempo casuale.
- Dopo molte volte che non si ricevono conferme (acknowledgement) allora la stazione abbandona l’dea di trasmettere.

### **Protocollo CSMA basico in pseudocodice**

```C++
N=1;
while(N <= max){
	waitUntil(channelFree()); 
	send(data_frame); 	
	waitUntil(ackOrTimeout()); 
	if(ack_received){ 
		exit while;
	}else{
		/* timeout scaduto: ritrasmissione*/
		/* timeout scaduto: ritrasmissione*/ 	
		t=random()*WNDW*2^n;
		wait(t);
		N=N+1;
	}
}
/* troppi tentativi: rinuncio a trasmettere*/	
```

### **Ritardo di propagazione**

Una stazione non può rilevare l’occupazione del BUS da parte di una stazione remota semplicemente ascoltando prima di trasmettere a causa di un fenomeno fisico detto **ritardo di propagazione** del segnale.
Il ritardo di propagazione di un segnale è il **tempo finito** con cui esso viaggia da una parte all’altra di un mezzo trasmissivo ed è dovuto al fatto che esso si propaga nello spazio con una **velocità finita**:
- Nel **vuoto** essa è pari alla **velocità della luce** cioè **300.000Km/sec**
- In un **conduttore metallico** ha un valore **inferiore** pari a circa **200.000Km/sec**

se **due stazioni** A e B trasmettono una di seguito all'altra separate di un tempo Δt e il tempo di propagazione tra A a B è maggiore di Δt allora, ascoltando la portante, B sente il canale libero e inizia a trasmettere il suo messaggio mentre c’è quello di A che sta per arrivare.
ciascuna stazione riterrà di essere l’unica a trasmettere sul canale fino a che la trama (corrotta dalla collisione) di A, che ha trasmesso prima, non arriverrà all’altra stazione B dopo il tempo di propagazione Δt corrispondente alla distanza tra A e B.

La **rilevazione reale**, da parte di una stazione, di **una collisione** può avvenire solo **dopo l’inizio** della trasmissione (o **durante** o **alla fine**).
Maggiore è la distanza tra due stazioni maggiore sarà il tempo con cui il segnale si propaga dall’una all’altra:   td = d/vP

**BUS senza collisioni:**

<img src="busnocollisioni.png" alt="alt text" width="700">

**BUS con collisioni:**

Una stazione non può rilevare istantaneamente l’occupazione del BUS da parte di una stazione remota a causa di un fenomeno fisico detto ritardo di propagazione.

<img src="buscollisioni.png" alt="alt text" width="700">

### **Riassumendo ALOHA e CSMA**

**Similitudini:**
- Sia ALOHA che CSMA devono implementare un meccanismo di rilevazione della collisione basato su ack
- Sia ALOHA che CSMA rilevano la collisione a seguito dell’evento mancata ricezione del messaggio di ack (allo scadere di un timeout). 

**Differenze:**
- ALOHA **trasmette senza verificare** se il canale è **occupato** o meno.
- CSMA **trasmette solo se “sente”** il canale **libero**.
- CSMA deve implementare un meccanismo di **ascolto** del canale **prima** della** trasmissione** (**CCA: Clear Channel Assesment**).

>[Torna a multiplazione TDM](mezzoradio.md#Multiplazione-TDM)

## **CSMA/CD**

### **Fasi CSMA/CD basico**

Significa **Carrier Sensing Multiple Access Collision Detection** cioè protocollo di Accesso Multiplo con **Ascolto della Portante** (**prima** della trasmissione) e **rilevazione della Collisione** (**durante** la trasmissione).

Una stazione **trasmittente**:
- al momento che ha una trama pronta, **ascolta il canale prima** di trasmettere per stabilire se esso è libero o meno.
- Appena essa rileva il canale libero invia immediatamente la trama ma ascolta anche durante la trasmissione.
- L’**ascolto durante la trasmissione** serve a stabilire se sul canale è avvenuta o meno una collisione. 
- Se non vengono rilevati **segnali di collisione** allora la trasmissione è avvenuta con successo.
- un **segnale di collisione** può essere considerato il **misurare** sul canale più energia di quanta la stazione stessa non se ne aspetti a causa della sua trasmissione. Ciò può essere rilevato mediante un dispositivo a soglia che scatti oltre un certo **valore di riferimento**.
- Altrimenti la stazione arresta la trasmissione corrente e ricomincia da zero la trasmissione della trama dopo un **tempo casuale**.
- Dopo molte volte che non si ricevono conferme (acknowledgement) allora la stazione abbandona l’dea di trasmettere.

### **Protocollo CSMA/CD basico in pseudocodice**

Soluzione **di base** per il **TX**

```C++
N=1;
while(N <= max){
	waitUntil(channelFree());
	sendAndListen(frame);
	waitUntil(endOfFrame() or collision());
	if(collision()){
		stopTransmitting();
		send(jamming);
	}else{
		/* no collision detected */
		wait(interframe_delay);
		exit while;
	}
	N=N+1;
}
/* troppi tentativi: rinuncio!*/
```

<img src="csmacdflow.jpg" alt="alt text" width="500">

### **Possiamo determinare tutte le collisioni?**

- Se non si ascolta il canale evidentemente no.
- Ma nel CSMA/CD si ascolta solo **finchè si trasmette**….

<img src="nolisten.png" alt="alt text" width="700">

### **Soluzione: trasmissione abbastanza lunga**

- **Caso peggiore**: il tramittente A è ad un capo e la collisione avviene all’altro capo quando trasmette pure B.
- Per ricevere l’eco della collisione A deve continuare ad ascoltare (cioè a trasmettere) finchè la trama corrotta non si propaga da B fino a lei.

<img src="listen.png" alt="alt text" width="700">

### **Come rilevare tutte le collisioni?**

Il tempo di propagazione da A a B per 10Mb/sec è 25.6 µsec
Il tempo **di andata** da A a B e poi **di ritorno** ad A è detto **RTT (Round Trip Time)** e alla velocità di 10Mb/sec è 51.2 µsec

**Prerequisito per rilevare tutte le collisioni**: 
- ogni stazione **deve ascoltare (cioè trasmettere)** per un **tempo minimo** pari a **un RTT** cioè 51.2 µsec
- Se la velocità di trasmissione è fissa, la **condizione sul tempo** si traduce in una **condizione sulla lunghezza minima** della trama (nel caso di Ethernet 64 byte).
- Cioè ogni stazione deve trasmettere **trame lunghe almeno 64 byte**

### **Come reagire a fronte di una collisione?**

Se tutte le stazioni ritrasmettono nello stesso istante collidono immediatamente

**Soluzione**: **Backoff esponenziale**
- È un **tempo casuale**
- È calcolato in **maniera indipendente** l’una dall’altra da tutte le stazioni
- Serve a determinare la stazione che deve **trasmettere per prima** minimizzando il rischio di **trasmissioni contemporanee**
- È calcolato all’interno di un **intervallo (finestra) di contesa**

Il **tempo di Backoff** si calcola in **multipli interi** di uno **slot** che è uguale ad un **RTT** ```(slot = 1 RTT)``` secondo la formula:
```C++
r = random(0, 2k – 1)*RTT
```
- 1-ma collisione : aspetta da 0 a 1 slot
- 2-da collisione : aspetta 0, 1,2 o 3 slots 
- i-esima collisione : aspetta 0..2i-1 slots 

### **Finestra di contesa**

La finestra di collisione o di contesa (contention Window o CW) è l’intervallo di valori all’interno del quale viene calcolato il tempo casuale di backoff

La finestra di collisione è **deterministica** (cioè ha un valore non casuale) e può essere fissa o variabile, ad es:``` [0  2k – 1]``` è variabile e dipende dal **numero k di tentativi andati a vuoto**.

Una finestra di collisione **grande** minimizza la **probabilità** di una collisione ma aumenta il **ritardo medio** prima della trasmissione che, a sua volta, diminuisce la **velocità media** di trasmissione.

Una finestra di collisione **variabile** realizza un buon compromesso tecnico: quando **non ci sono collisioni** è piccola e il ritardo è minimo, in presenza di collisioni essa si **allarga gradatamente** per diminuirne la probabilità ad ogni successivo tentativo.

Il tempo di backoff è un numero casuale calcolato tra un valore minimo ed un valore massimo che  rappresentano la finestra di collisione.
Nel caso del CSMA/CD:
```C++
	        k = min(10, N); //numero di tentativi
		r = random(0, 2k – 1)*RTT; //tempo di backoff
```
### **Procollo CSMA/CD completo in pseudocodice**

```C++
N=1;
while(N<= max){
	waitUntil(channelFree());
	sendAndListen(frame);
	waitUntil(endOfFrame() or collision()); 	
	if(collision()){
		stopTransmitting();
		send(jamming);
		k = min (10, N);
		r = random(0, 2k – 1);
		wait(r*RTT);
	}else{
		/* no collision detected */
		wait(interframe_delay);
		exit while;
	}
	N=N+1;
}
/* troppi tentativi: rinuncio!*/
```

### **Fasi CSMA/CD completo**

a stazione **trasmittente**:
- al momento che ha una trama pronta, **ascolta il canale prima** di trasmettere per stabilire se esso è libero o meno.
- Appena essa rileva il canale libero invia immediatamente la trama ma ascolta anche durante la trasmissione.
- L’**ascolto durante la trasmissione** serve a stabilire se sul canale è avvenuta o meno una collisione. 
- Se non vengono rilevati **segnali di collisione** allora la trasmissione è avvenuta con successo. Un **segnale di collisione** può essere considerato il **misurare** sul canale più energia di quanta la stazione stessa non se ne aspetti a causa della sua trasmissione. Ciò può essere rilevato mediante un dispositivo a soglia che scatti oltre un certo **valore di riferimento**.
- Altrimenti la stazione **arresta** la trasmissione corrente e **trasmette** invece una particolare sequenza, detta sequenza di jamming, che realizza una collisione con una **durata** sufficiente per assicurarne il suo rilevamento anche da parte delle stazioni trasmittenti più lontane (le due onde sovrapposte dopo un certo tempo si separano).
- Calcola un numero intero random che moltiplica per RTT ottenendo un tempo casuale di attesa (multiplo di RTT) prima di una successiva ritrasmissione della trama interrotta.
- Altrimenti la stazione arresta la trasmissione corrente e ricomincia da zero la trasmissione della trama dopo un **tempo casuale**.
- Dopo molte volte che non si ricevono conferme (acknowledgement) allora la stazione abbandona l’dea di trasmettere.

Le altre stazioni, quando ricevono la **sequenza di jamming**, sono **avvisate** della avvenuta collisione e:
- Se **ricevevano**, **scaricano dal buffer** di ricezione quanto ricevuto fino a quell momento
- Se **trasmettevano**, **arrestano immediatamente la trasmissione** e fanno partire l’**algoritmo di backoff** che stabilisce il ritardo casuale prima della ritrasmissione del messaggio interrotto.

<img src="csmacdflow2.png" alt="alt text" width="350">

### **Riassumendo CSMA e CSMA/CD**

Similitudini:
- CSMA e CSMA/CD devono implementare un meccanismo di ascolto del canale prima della trasmissione per stabilire se esso è libero o meno. 

Differenze:
- il CSMA **rileva la collisione indirettamente** dopo la trasmissione **in fase di ricezione**, a seguito del **mancato arrivo del messaggio di ack** (allo scadere di un timeout), Si tratta di una tecnica SW. Trasmissione e ricezione non devono essere contemporanee.
- Il CSMA/CD durante la trasmissione mantiene attiva anche la funzione di  ricezione che può rilevare una collisione perchè:
    - Il segnale “ascoltato” durante la trasmissione ha una potenza più elevata di quello effettivamente trasmesso. É una **tecnica HW**.
    - Il segnale “ascoltato” e decodificato durante la trasmissione ha un valore in bit diverso da quello trasmesso. É una **tecnica SW**.

>[Torna a multiplazione TDM](mezzoradio.md#Multiplazione-TDM)

## **CSMA/CA**

Non è possibile sempre rilevare le collisioni **durante la trasmissione (mezzo radio)**

Miglioramento del CSMA grazie all’introduzione degli **IFS (interframe space)**:
- Ritardo iniziale di trasmissione (**EIFS**) 
- Short Inter Frame Space (**SIFS**): ritardo tra una trama e l’invio del suo ack. È il tempo minimo per consentire ai dispositivi HW di commutare dallo stato di ricezione a qello di trasmissione.
- Distributed Inter Frame Space (**DIFS**): **ritardo minimo tra due trame successive** in trasmissione. ```DIFS =  SIFS  + (2 * Slot time)```. Serve a proteggere la trasmissione di un ack da eventuali  collisioni
- ```SlotTime = CCATime + RxTxTurnaroundTime + AirPropagationTime+ MACProcessingDelay``` 
 è il tempo per percorrere il tragitto tra due stazioni poste agli estremi più remoti della cella radio.

Vale la disuguaglianza:   
```C++
SIFS < DIFS < EIFS
```

### **Protocollo CSMA/CA sul ricevitore in psudocodice**
```C++
While (true)
{
 WaitUntil(dataFrameArrived());
	if(!duplicate())
		{ deliver(frame) }
 wait(SIFS);
 send(ack_frame);
}
```
La funzione deliver() consegna la trama al livello superiore (ad un protocollo di livello superiore o alla applicazione)

### **Fasi CSMA/CA al ricevitore**

Una **stazione ricevente**:
1. Aspetta l’arrivo di una nuova trama
2. Controlla se è una trama duplicate, cioè gia ricevuta
3. Controlla il **CRC della trama** per verificare se è stata **ricevuta correttamente**
se non lo è la consegna al livello superiore e **non esegue** i passi successivi
4. Aspetta un SIFS
5. Invia una trama di ack

### **Protocollo CSMA/CA sul trasmettitore in pseudocodice**
```C++
N=1;
while(N <= max){
    waitUntil(channelFree());
	if(receivedCorruped())
	{ 
	 wait(EIFS);
	}else
	{ 
	 wait(DIFS);
	}
	send(data_frame);
	  waitUntil(ackOrTimeout());
	if(ack_received){
		exit while;
	}else{
		/* timeout scaduto: si ritrasmette*/
		N=N+1;
	}
}
/* troppi tentativi: rinuncio!*/	
```
### **Fasi CSMA/CA al trasmettitore senza backoff**

Una **stazione trasmittente**:
- Se la trama precedentemente ricevuta era corrotta prima di trasmettere, aspetta un tempo EIFS 
- Altrimenti se la stazione sente il canale occupato aspetta **finchè è libero**, da quel momento in poi, **aspetta un tempo DIFS** 
- Trascorso il DIFS invia immediatamente la trama
- Aspetta l’**arrivo di un ack**, se non arriva in tempo, allo s**cadere di un timeout**, avvia la **ritrasmissione** della stessa trama.

Anche se C, sfortunatamente, valutasse il canale libero ascoltandolo durante il SIFS di B poichè DIFS>SIFS accadrebbe che C ritroverebbe il canale occupato durante la seguente trasmissione dell’ack di B e quindi si **fermerebbe**:

<img src="esempio.png" alt="alt # **text" width="800">

### **Significato di DIFS**

È il **tempo di attesa** che aspettano tutte le stazioni per trasmettere a partire dalla fine dell’ultima trasmissione valida (**invio ack** messaggio precedente).

Serve a **proteggere** la **trasmissione di un ack** da eventuali  **collisioni**, è sempre:       
```C++
DIFS =  SIFS  + (2 * Slot time) 
```

- **SIFS** dovrebbe essere più breve di DIFS in modo che sia possibile inviare un frame di ACK **prima** che le altre stazioni riprendano il loro backoff
- Il **DIFS** dura quanto basta affinchè una qualunque stazione si possa accorgere che il canale **è occupato** a seguito della trasmissione di un ack. Per fare in modo di essere sicuri che dopo di esso un ack non posssa più arrivare, il DIFS viene calcolato nel caso peggiore  ```(2 * Slot time )```, cioè ipotizzando che le stazioni interessate siano poste agli **estremi opposti** della cella radio, quelli che sono **reciprocamente più distanti**.

Nel grafico, DATA e ACK sono **tempi di trasmissione** (non di propagazione)

Durante il **tempo di attesa DIFS** l’ack ha il tempo di propagarsi fino alla stazione più remota senza **essere disturbato** da altre trasmissioni.

Il **tempo di attesa DIFS** non elimina in assoluto la possibilità di una **eventuale collisione** perchè può sempre accadere che qualcuno inizi a trasmettere un **messaggio** un **attimo dopo** dello scadere del DIFS senza che la stazione in attesa possa rendersene conto a causa del fenomeno del **ritardo di propagazione**. Ma la collisione può riguardare solo un **nuovo messaggio** non la trasmissione dell'ack di quello appena inviato.

Il tempo di attesa DIFS è **uguale** per tutte le stazioni e **dopo** di esso inizia una finestra di **trasmissione**, detta **finestra di contesa** (CW), in cui tutte le stazioni **competono** per l'accesso al canale avendo solo l'accortezza di **ascoltare prima di tramettere**, cautela che limita ma non annulla le probabilità di collisioni (per via del ritardo di propagazione).

<img src="difs.png" alt="alt # **text" width="600">

### **Significato di EIFS**
```C++
EIFS  = SIFS + DIFS + ACK_Tx_Time
```
- usato al posto di DIFS dalle stazioni che hanno ricevuto un frame incomprensibile
- Anche se una stazione non è stata in grado di decodificare il frame, potrebbe essere che invece il destinatario legittimo sia ancora in grado di farlo. 
- Se ciò accade, il destinatario dovrebbe avere l'opportunità di restituire un frame di ack; il ritardo EIFS garantisce che la trasmissione del frame di Ack possa procedere senza l'introduzione di eventuali interferenze da parte di chi non è stato in grado di decodificare il frame.

<img src="eifs.png" alt="alt # **text" width="600">

### **Backoff**

Se **due stazioni**, dopo averlo **ascoltato**, trovano il **canale libero** potrebbero comunque **collidere** perchè, a causa del **ritardo di propagazione**, potrebbero non rendersi conto che un'altra stazione **ha già cominciato** a trasmettere occupando il canale.
Una eventuale **collisione** dei messaggi determina la ricezione di **trame corrotte** da parte delle stazioni destinatarie che, a sua volta, causa il **mancato invio** di un ack alle stazioni trasmittenti che, allo scadere del timout di trasmissione, pianificheranno la **ritrasmissione** del messaggio non ancora confermato.

Se la **ritrasmissione** avvenisse per tutte le stazioni dopo un ugual **ritardo DIFS** dal momento in cui entrambe le stazioni sentono il **canale libero**, ciò determinerebbe presumibilmente una **trasmissione simultanea** delle due stazioni e quindi una nuova collisione.

<img src="collisionicsma.png" alt="alt # **text" width="800">

La **soluzione** è ritrasmettere sempre dopo un **tempo casuale (backoff)** all’interno di una **finestra di contesa**

<img src="backoff.png" alt="alt # **text" width="800">

Il **backoff** casuale di una stazione inizia sempre dopo un tempo di attesa fisso **DIFS**. La stazione **ascolta** sempre il canale prima di tramettere e lo fa anche **durante il backoff**. Ci sono quindi due possibilità:
- il backoff **scade** e quindi subito dopo la stazione comincia a trasmettere
- il backoff viene **interrotto** dall'ascolto del canale occupato (da una stazione con backoff più corto) e **riprende** una volta che il canale ritorna libero.
- la trasmissione di un'altra stazione potrebbe anche essere destinata alla stazione stessa per cui il backoff potrebbe anche essere interrotto dalla ricezione di un messaggio.

Nel caso del WiFi in figura, è illustrato un backoff **con prenotazione** che ha la proprietà di non azzerare, sprecandolo, il tempo di attesa **già trascorso** da una stazione sul proprio backoff. Nel momento in cui il canale **ritorna libero**, la stazione che aveva "congelato" il backoff quando lo aveva sentito occupato, ricomincia il **conteggio iniziale** dal tempo in cui questo era stato interrotto, senza ricalcolarlo daccapo. E' una maniera per non perdere la **priorità acquisita** ricalcolando il backoff da zero. E' una **ottimizzazione** per garantire una maggiore **equità (fairness)** nell'accesso alla risorsa canale che non sempre è implementata in protocolli CSMA/CA diversi dal WiFi.

**Pseudocodice protocollo CSMA/CA sul trasmettitore con backoff:**

```C++
N=1;
while(N <= max){
	waitUntil(channelFree());
	if(receivedCorruped())
	{ 
	 	wait(EIFS);
	}else
	{ 
	 	wait(DIFS);
	}
	backoff_time = int(random[0,min(255,7*2N-1)])*T;
        waitUntil(channelFreeDuringBackoff());
	send(data_frame);
        waitUntil(ackOrTimeout());
	if(ack_received){
		 exit while;
	}else{ 
		 /* timeout scaduto: si ritrasmette*/
		 N=N+1; 
	}
}
/* troppi tentativi: rinuncio!*/ 

```

### **Finestra di contesa variabile**

Se accade una collisione durante il backoff spesso ciò significa che due stazioni hanno scelto lo stesso slot nella finestra di backoff.
Ad ogni collisione, prima di ritentare la trasmissione, la stazione **raddoppia la dimensione** della finestra di backoff CW (Content Window o finestra di contesa).
Lo scopo di tale raddoppio è quello di **adattare** la dimensione della finestra al **numero di contendenti**, in considerazione del fatto che le **collisioni** sono un **indice di “affollamento”**.

<img src="finestra.png" alt="alt # **text" width="600">

>[Torna a multiplazione TDM](mezzoradio.md#Multiplazione-TDM)

## **Ottimizzazioni**

## **Accesso immediato e differito**

In alcune implementazioni (802.11) il DIFS può creare un accesso senza contesa: 
- Se il canale è libero dopo questo tempo si può trasmettere con un accesso immediato, cioè senza attendere alcun backoff.
- Se il canale è occupato, prima o dopo questo tempo, si esegue un accesso differito, cioè un DIFS dopo la fine della trasmissione corrente, scegliendo uno slot casuale all’interno di una finestra di contesa (tramite backoff).

Un accesso senza contesa non vuol dire un accesso senza collisioni, queste sono sempre possibili se, dopo un DIFS, due stazioni iniziano a parlare contemporaneamente.

<img src="immediato.png" alt="alt # **text" width="600">

## **Problema della stazione nascosta**

**Problema:**
- Le stazioni A e C non sono visibili una all’altra per cui l’ascolto del canale non è in grado di rilevare se una di loro ha occupato il canale prima dell’altra. 
- Se entrambe devono trasmettere a B, sia A che C sentiranno il canale libero e inizieranno la trasmissione delle rispettive trame. 
- B riceverà i messaggi provenienti da entrambe le stazioni che, a seguito della collisione, saranno inintelleggibili.

<img src="hidden.png" alt="alt # **text" width="600">

**Soluzione:**
- Il trasmettitore prenota dei time slot tramite ll messaggio di controllo RTS (Request To Send) avente per **argomento** il tempo di trasmissione richiesto, adatto alla lunghezza del messaggio dati.
- Il ricevente conferma la prenotazione tramite il messaggio di controllo CTS (Clear To Send) avente per **argomento** il tempo prenotato al trasmettitore
La stazione B è visibile da tutti per cui sia dalla stazione A che ha prenotato il canale sia dalle altre, come la C, che potenzialmente potrebbero trasmettere di li a poco
- Il messagio CTS dichiara l’avvenuta assegnazione del canale ad A per cui tutti gli altri, come B e C, si asterranno dal trasmettere per tutta la durata del tempo dichiarato nel CTS.
- Spesso la gestione delle prenotazioni è centralizzata ed è effettuata dallo stesso AP che in genere è in una **posizione baricentrica** rispetto a tutte le stazioni ed è pertanto da queste sempre visibile.

<img src="rtscts.png" alt="alt # **text" width="800">

## **Prenotazione del canale mediante NAV**

### **CCA Reale e CCA Virtuale**

**CCA Reale:** è il Clear Channel Assesment realizzato mediante l’ascolto effettivo del canale fisico prima della trasmissione. Presuppone che la stazione sia in stato di ricezione e che un **dispositivo a soglia** sia pronto a rilevare eventuale energia trasmessa da altre stazioni. 

**CCA Virtuale: il Clear Channel Assesment virtuale** è invece realizzato mediante l’osservazione della variabile di conteggio NAV. Il NAV è decrementato ad intervalli di tempo fissi e in ciascuno di essi può accadere:
- Il NAV > 0 allora il canale virtuale è considerato occupato
- Il NAV = 0 allora il canale virtuale è considerato libero

I due CCA sono del tutto indipendenti e possono portare a risultati diversi.  Lo stato del canale è stabilito dalla lettura di entrambi.
- Quando entrambi rilevano che il canale è libero allora il trasmettitore inizia effettivamente la trasmissione.
- Il contatore NAV viene impostato da tutte le stazioni al momento in cui queste ricevono un messaggio CTS con il valore di tempo in esso contenuto.

Il valore contenuto nel CTS rappresenta il tempo prenotato dalla stazione autorizzata a trasmettere ed è il tempo che essa ritiene necessario per trasmettere il suo messaggio alla velocità corrente del sistema.

### **Prenotazione del canale**

Il **trasmettitore** prenota dei **time slot** tramite un messaggio di controllo **RTS** (Request To Send) avente per **argomento** il tempo di trasmissione necessario per inviare i suoi dati . 

Il **ricevente** conferma la prenotazione tramite il messaggio di controllo **CTS** (Clear To Send) avente per **argomento** il tempo prenotato dal trasmettitore.
Il messaggio **CTS** è ricevuto da **tutte le stazioni** che, col tempo in esso riportato, impostano il proprio contantore di Carrier Sensing detto **NAV** con il quale tengono conto della prenotazione.

Il NAV viene decrementato nel tempo, fino a 0; quando il NAV è diverso da zero, vuol dire che una trasmissione è in atto nelle vicinanze e quindi la stazione si astiene dal trasmettere.

La stazione che ha prenotato il canale durante il NAV può effettuare in sicurezza la trasmissione dei suoi dati perchè il tempo di NAV è, di fatto, **al riparo** dalle collisioni.

Le collisioni in pratica **non possono** colpire i messaggi di **ack** e i **dati**, gli unici messaggi soggetti a collisione sono le trame di controllo RTS e CTS che però sono **molto corte**.

<img src="prenotazione.png" alt="alt # **text" width="600">

 ### **802.11 CSMA/CA: flowchart**
 
 <img src="802.11flow.png" alt="alt # **text" width="600">
 
 ### **802.11 CSMA/CA: fasi**
 
Il protocollo CSMA/CA (Carrier Sense Medium Access with Collision Avoidance) funziona nel seguente modo, per una trasmissione da A a B:
1. La stazione trasmittente A cerca di determinare lo stato del mezzo valutando il contenuto di NAV (CCA Virtuale) ed ascoltando il mezzo (CCA Reale). Il canale è considerato libero, quando sia il CCA Virtuale che il CCA Reale non rilevano attività (operazione di AND logico tra i due valori). I casi sono due:
    1. Se il canale rimane libero per un intervallo di tempo DIFS, salta al punto 3.
    2. Se invece il canale è occupato (o viene occupato durante l’intervallo DIFS), prosegue al punto 2.
2. A avvia la procedura di backoff.
3. A emette un RTS.
4. Se entro un intervallo di tempo ben definito, A non riceve il CTS da B, vuol dire, molto probabilmente, che l’RTS ha colliso con un altro frame; 
5. Quando B riceve l’RTS, risponde con un CTS.
6. Ricevuto il CTS, A può cominciare a trasmettere il frame contenente i dati veri e propri.
7. Se entro un intervallo di tempo ben definito, A non riceve un ACK da B, vuol dire che il frame dati non è stato ricevuto correttamente, e quindi A deve ritrasmetterlo ripetendo tutta la procedura.
8. Una volta che B ha ricevuto correttamente il frame Dati, risponde con un ACK concludendo il protocollo.


**Sitografia**:
- https://teoriadeisegnali.it/story/802.11/on_desk/accesso.html
- http://ecomputernotes.com/computernetworkingnotes/communication-networks/what-is-aloha
- https://it.pinterest.com/pin/368943394449072984/
- http://www.mathcs.emory.edu/~cheung/Courses/558/Syllabus/00/CSMA/csmacd.html
- http://www.benve.org/Download/Wireless.pdf
- http://infocom.uniroma1.it/alef/802.11/on_desk/accesso.html#Distributed_Coordination_Function_
- https://www.saylor.org/site/wp-content/uploads/2011/10/SAYLOR.ORG-CS402-CSMACOLLISIONAVOIDANCE.pdf
- https://www.slideshare.net/obonaventure/5-sharingapp
- http://www.opentextbooks.org.hk/ditatopic/3611
- https://mrncciew.com/2014/10/12/cwap-802-11-medium-contention/
- https://en.wikipedia.org/wiki/Extended_interframe_space
- https://www.hamilton.ie/publications/Thesis_tianji.pdf


>[Torna a multiplazione TDM](mezzoradio.md#Multiplazione-TDM)
