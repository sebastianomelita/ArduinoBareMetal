>[Torna a reti di sensori](sensornetworkshort.md#Interfaccia-radio)

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

### **Ritardo di propagazione**

Una stazione non può rilevare l’occupazione del BUS da parte di una stazione remota semplicemente ascoltando prima di trasmettere a causa di un fenomeno fisico detto ritardo di propagazione del segnale.
Il ritardo di propagazione di un segnale è il tempo finito con cui esso viaggia da una parte all’altra di un mezzo trasmissivo ed è dovuto al fatto che esso si propaga nello spazio con una velocità finita:
- Nel vuoto essa è pari alla velocità della luce cioè 300.000Km/sec
- In un conduttore metallico ha un valore inferiore pari a circa 200.000Km/sec

se due stazioni A e B trasmettono in successione separate di un tempo Δt e il tempo di propagazione tra A a B è maggiore di Δt allora, ascoltando la portante, B sente il canale libero e inizia a trasmettere il suo messaggio mentre c’è quello di A che sta per arrivare.
ciascuna stazione riterrà di essere l’unica a trasmettere sul canale fino a che la trama (corrotta dalla collisione) di A, che ha trasmesso prima, non arriverrà all’altra stazione B dopo il tempo di propagazione Δt corrispondente alla distanza tra A e B.

La rilevazione reale, da parte di una stazione, di una collisione può avvenire solo dopo l’inizio della trasmissione (o durante o alla fine).
Maggiore è la distanza tra due stazioni maggiore sarà il tempo con cui il segnale si propaga dall’una all’altra:   td = d/vP


<img src="alohacollisioni.jpg" alt="alt text" width="600">
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
/* troppi tentativi: rinuncio a trasmette![image](https://user-images.githubusercontent.com/18554803/128429136-7f470f6d-290b-4a60-8d52-f7edbf6141fb.png)
```

<img src="busnocollisioni.png" alt="alt text" width="600">

<img src="buscollisioni.png" alt="alt text" width="600">

CSMA: pseudocodice
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
		N=N+1;
	}
}
/* troppi tentativi: rinuncio a trasmettere*/	

```


>[Torna a reti di sensori](sensornetworkshort.md#Interfaccia-radio)
