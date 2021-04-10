
## **PROTOCOLLO CONFERMATO**

Esempio di protocollo confermato con due stati (WAITSTATE E ACKSTATE). 
Il protocollo ha un’unica coda, di dimensione fissa, usata come:
-	coda di trasmissione, per ospitare i dati dell’applicazione in attesa di essere trasmessi
-	coda di ricezione, per ospitare i dati ricevuti ancora non completamente arrivati.
La funzione sendMsg() vengono chiamate nel loop in modo asincrono, quando l’applicazione ha qualcosa da trasmettere
La funzione poll() è chiamata in ogni loop in modo periodico e continuativo, alla massima velocità della CPU. Il suo scopo è stare in perenne ricezione del canale cercando di riconoscere il momento in cui è stato ricevuto un messaggio completo e valido, quando ciò accade, carica il messaggio sulla coda di ricezione.

**Comportamento in ricezione.** 
-	Il protocollo notifica l‘arrivo di un messaggio chiamando una funzione di callback  rcvEventCallback(rt) che ha per parametro il record (la struct) su cui è caricato il messaggio in arrivo (header+payload) in formato parallelo (non serializzato). 
-	Ad ogni arrivo di un messaggio il protocollo risponde trasmettendo un messaggio di ack alla volta del mittente del messaggio ricevuto.

**Comportamento in trasmissione.** 
-	L’applicazione chiama la funzione sendMsg(txobj) dove txobj è il messaggio in formato non serializzato sotto forma di record (struct)
-	Ad ogni invio il protocollo fa partire un timer di trasmissione che può essere fermato all’arrivo di un messaggio di conferma (ack) da parte del ricevente
-	Se non arriva il messaggio di conferma entro lo scadere di un tempo massimo (timout) allora il protocollo riinvia il messaggio. Questo processo viene ripetuto per un certo numero di volte dopodichè il protocollo rinuncia a trasmettere.
             

**Schema di principio del codice:**

```C++
// offesets (spiazzamenti) dei campi del messaggio a partire dall’inizio
enum MESSAGE
{
    DA            = 0, //!< destination address position
    SA,		//!< source address position
    GROUP, 		//!< Function code position
    SI, 		//!< Service identifier position
    BYTE_CNT,  	//!< byte counter position
    PAYLOAD 	//!<  start of data position
};

// stati del protocollo
enum PROTO_STATE
{
    WAITSTATE             	  	= 1,
    ACKSTATE                  	= 2
};

// struttura dati parallela (non serializzata) che costituisce il messaggio
typedef struct
{
   uint8_t u8sof;
   uint8_t u8da;          
   uint8_t u8sa
   uint8_t u8group;         
   uint8_t u8si;    
   uint8_t *data;     
   int8_t msglen;
   bool multicast;
} telegram_t;


telegram_t txobj, rxobj, ackobj;


//invia un messaggio in formato "parallelo" sotto forma di struct 
// sull'uscita seriale
bool sendMsg(modbus_t *tosend){
	tosend->u8sa = mysa;
	tosend->u8group = mygroup;
	parallelToSerial(tosend);
	sendTxBuffer(u8Buffer[ BYTE_CNT ]); //trasmette sul canale
	return sent;
}

// carica i campi della struct nella giusta posizione nel buffer 
// secondo il formato del messaggio stabilito dal protocollo
void parallelToSerial(const modbus_t *tosend){
	//copia header
	u8Buffer[ DA ] = tosend->u8da;
	u8Buffer[ SA ] = tosend->u8sa;
	u8Buffer[ GROUP ] = tosend->u8group;
	u8Buffer[ SI ] = tosend->u8si;
	u8Buffer[ BYTE_CNT ] = tosend->msglen + PAYLOAD;
	//copia payload
	for(int i=0; i < tosend->msglen; i++){
		u8Buffer[i+PAYLOAD] = tosend->data[i];
	}
}

// ascolta in polling l'ingresso seriale
int8_t poll(modbus_t *rt)
{
	uint8_t u8current;
	
      // controlla quanti caratteri del messaggio sono arrivati
      // sulla coda di ricezione
      u8current = Serial.available(); 
    
	if (u8current == 0){ // nessun messaggio 
   //allora valuta lo scadere del timer
		if(u8state == ACKSTATE){
			if(millis()-precAck > timeoutTime){
				if(retry < MAXATTEMPTS){
					resendMsg(appobj); //trasmette sul canale
					precAck = millis();
				}else{
					retry = 0;
				}
				u8state = WAITSTATE;
			}
		}
		// rendi mutuamente esclusivo il blocco di codice
		return 0;  // se non è arrivato nulla ricontrolla al prossimo giro
	}
	
    // se arrivano nuovi caratteri rimani in ascolto
    // perchè il messaggio è ancora incompleto
    if (u8current != u8lastRec)
    {
        // aggiorna ogni volta che arriva un nuovo carattere!
	  u8lastRec = u8current;
        u32time = millis();
	  // rendi mutuamente esclusivo il blocco di codice
        return 0;
    }
    
    // non arrivano nuovi caratteri ma è troppo presto allora aspetta
    if ((unsigned long)(millis() -u32time) < (unsigned long)STOP_BIT) 
 		// rendi mutuamente esclusivo il blocco di codice
return 0;
	
    // non arrivano nuovi caratteri ma è passato il tempo di interframe
    // alllora vuol dire che la trama è completa allora bufferizza
    int8_t i8state = getRxBuffer();  
	
    // ma se è palesemente incompleta scartala!
    if (i8state < PAYLOAD + 1) 
    {
	  // rendi mutuamente esclusivo il blocco di codice
        return i8state;
    }

    if (u8Buffer[ SI ] == MSG){ // se ricevo un messaggio
		// prendi l'indirizzo di sorgente del messaggio ricevuto
		// e fallo iventare indirizzo di destinazione del messaggio di ack
	      ackobj.u8da = u8Buffer[ SA ]; 
      rcvEvent(rt, i8state); // parallelizza
	      // se ricevo un messaggio invio l'ack
		ackobj.u8si = ACK;
		sendMsg(&ackobj);  
// funzione che realizza l'azione da compiere all'arrivo del  //messaggio
		rcvEventCallback(rt);
		// rendi mutuamente esclusivo il blocco di codice
		return i8state; 
	}else if (u8Buffer[ SI ] == ACK){ // se ricevo un ack
		if(u8state == ACKSTATE){
			u8state = WAITSTATE;	//next go to WAITSTATE
			retry = 0;
		}//else messaggio di ack si perde....
	}
	return i8state;
}

void sendTxBuffer(uint8_t u8BufferSize){
    // transfer buffer to serial line
    Serial.write( u8Buffer, u8BufferSize ); 
}


int8_t getRxBuffer()
{
    boolean bBuffOverflow = false;
    uint8_t u8BufferSize = 0;
    while ( Serial.available() ) // finchè ce ne sono, leggi i caratteri
    {					  // e mettili sul buffer di ricezione
        u8Buffer[ u8BufferSize ] = Serial.read();
        u8BufferSize ++;
	  // segnala evento di buffer overflow (un attacco hacker?)
        if (u8BufferSize >= MAX_BUFFER){
			return ERR_BUFF_OVERFLOW;
	  }
    }
    return u8BufferSize;
}

// deserializzzazione in ricezione
void rcvEvent(modbus_t* rcvd, uint8_t msglen){
// converti da formato seriale (array di char) in formato parallelo   
//(non serializzato)
	// header
	rcvd->u8da = u8Buffer[ SA ];
	rcvd->u8group = u8Buffer[ GROUP ];
	rcvd->u8si = u8Buffer[ SI ];
	rcvd->msglen = u8Buffer[ BYTE_CNT ];
	// payload
	for(int i=0; i < msglen-PAYLOAD; i++){
		rcvd->data[i] = u8Buffer[i+PAYLOAD];
	}
	// notifica l'evento di ricezione all'applicazione con una callback
}

void rcvEventCallback(modbus_t* rcvd){
	Serial.print("Ricevuto:");
	Serial.println(rcvd->data);
}
void loop() // run over and over
{
	poll(&rxobj);
	
	if(millis()-prec > TBASE){
		prec = millis();
		
		txobj.data = "ciao";
		txobj.msglen = strlen(txobj.data) + 1;
		sendMsg(&txobj);
	}
}
```
