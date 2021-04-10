>[Torna all'indice](indexseriale.md)
## **MODALITA DI ELABORAZIONE DEI MESSAGGI**

I **messaggi** provenienti in ricezione dalla seriale possono essere **letti** nel loop() sostanzialmente in **due maniere**: 
- **un byte alla volta** 
- **in buffering**

In entrambi i casi l’applicazione tiene un proprio buffer (**buffer dei messaggi**) dove memorizzare i messaggi ricevuti per poterli elaborare. Si tratta di un buffer **gestito direttamente dall’applicazione** e a parte rispetto a quelli di RX e TX che sono nascosti all’applicazione e gestiti in maniera ad essa trasparente da un apposito driver (funzione di libreria di Aduino).  

Nella prima modalità un messaggio viene letto un carattere alla volta ad ogni loop e ogni carattere è memorizzato in posizioni successive all’interno di un buffer di dimensioni opportune. La fine dell’accumulo è segnalato o da un contatore, o dallo scadere di un timer o dalla ricezione di un carattere “tappo”, cioè un carattere speciale che, all’interno del flusso di byte, indichi la fine della stringa. A questo punto il messaggio è completo e può essere prelevato dal buffer per essere utilizzato dall’applicazione. 

La seconda modalità, il buffering, usa il loop solo per verificare la disponibilità di dati (byte) nel buffer di ricezione, si controlla inoltre la loro quantità, che ad ogni loop() cresce con nuovi arrivi. Quando il numero di byte ricevuti è sufficiente, oppure quando scade un timer oppure quando si legge (senza prelevarlo) un carattere speciale, allora, e solo allora, i caratteri vengono letti tutti insieme e vengono caricati sul buffer dei messaggi per essere elaborati dall’applicazione. In sostanza, il messaggio, pur essendo controllato in tutti i loop, non viene letto in nessuno meno che in un loop() particolare quando, in una sola volta (solitamente tramite un ciclo while), viene letto tutto intero e caricato sul buffer dell’applicazione. 


**Esempio di caricamento ad ogni loop**

Viene considerato un messaggio tutta la parte numerica terminata dai caratteri speciali + e =

```C++
int vals[3];
unsigned count;
char p;
char buf[10];

void setup() {                
  // Attiva il protocollo seriale
  Serial.begin(9600);
  count = 0;
}

void loop() {
    // polling sul registro USR che controlla se, nel loop corrente, è in arrivo un nuovo dato
    if (Serial.available()) { // vero se c'è qualcosa da leggere
		p = Serial.read(); // leggi un carattere
		
		if(p == '+'){ //carichi il primo operando (carattere speciale)
			buf[count] = '\0'; // metti il carattere "tappo" di fine stringa, cioè crei la stringa
			count = 0;      // azzeri il buffer (dopo ci scrivi sopra)
			vals[0] = atoi(buf); // trasformi la stringa in un numero
		}else if(p == '='){ //carichi il secondo operando (carattere speciale)
			buf[count] = '\0';
			count = 0;
			vals[1] = atoi(buf);  // calcoli il segno dell'operazione di somma
			
			Serial.print("La somma di ");
			Serial.print(vals[0]);  
			Serial.print(" con ");
			Serial.print(vals[1]); 
			Serial.print(" vale: ");
			vals[2] = vals[0] + vals[1]; // calcoli la somma
			Serial.println(vals[2]); 	    // stampi il risultato (println stampa sia numeri, che caratteri)
		}else{// è un numero
			buf[count] = p; // riempi il buffer un carattere alla volta 
			count++;
		}
    }
}
```

**Esempio di caricamento in modalità buffering**			
```C++			
int vals[3];
unsigned count;
char p;
char buf[10];

void setup() {                
// Attiva il protocollo seriale
  Serial.begin(9600);
  count = 0;
}

void loop() {
    // polling sul registro USR che controlla se, nel loop corrente, è in arrivo un nuovo dato
    if(Serial.available() > 0) { // vero se c'è qualcosa da leggere
		p = Serial.peek(); // leggi un carattere ma non devi prelevarlo dal buffer di RX
		
		if(p == '+'){ //carichi il primo operando
		    	Serial.read();    // leggi il carattere speciale e saltalo
			count++; // passa all’operando successivo
		}else if(p == '='){ //carichi il secondo operando
			Serial.read();    // leggi il carattere speciale e saltalo
		   	count = 0;        // torna al primo operando
			
			Serial.print("La somma di ");
			Serial.print(vals[0]);  
			Serial.print(" con ");
			Serial.print(vals[1]); 
			Serial.print(" vale: ");
			vals[2] = vals[0] + vals[1]; // calcoli la somma
			Serial.println(vals[2]); 	    // stampi il risultato (println stampa sia numeri, che caratteri)	
		}else{
			vals[count] = Serial.parseInt(); //lettura del messaggio (valore operando)
		}
    }
}	
```

Viene considerato un messaggio tutta la parte numerica terminata dai caratteri speciali + e =

La funzione Serial.parseInt() internamente chiama la Serial.available() per effettuare il parsing dei caratteri in arrivo aggiornando di uno la posizione della testa della coda di ricezione.
	
L’esempio seguente mostra come si possa realizzare il riempimento del buffer fino al prossino carattere speciale con un ciclo while.

**Esempio di caricamento in modalità buffering**	
```C++
int vals[3];
unsigned count, v;
char p;
char buf[10];

void setup() {                
// Attiva il protocollo seriale
  Serial.begin(9600);
  count = 0;
  v = 0;
}

void loop() {
    // polling sul registro USR che controlla se, nel loop corrente, è in arrivo un nuovo dato
    if(Serial.available() > 0) { // vero se c'è qualcosa da leggere
		p = Serial.peek(); // leggi un carattere ma non devi prelevarlo dal buffer di RX
		
		if(p == '+'){ //carichi il primo operando
		   	buf[count] = '\0'; 
		    	Serial.read();    // leggi il carattere e saltalo
			vals[0] = atoi(buf); // trasformi la stringa in un numero
			count = 0;
		}else if(p == '='){ //carichi il secondo operando
		    	buf[count] = '\0'; 
			Serial.read();    // leggi il carattere e saltalo
		    	vals[1] = atoi(buf);
			count = 0;
			
			Serial.print("La somma di ");
			Serial.print(vals[0]);  
			Serial.print(" con ");
			Serial.print(vals[1]); 
			Serial.print(" vale: ");
			vals[2] = vals[0] + vals[1]; // calcoli la somma
			Serial.println(vals[2]); 	    // stampi il risultato (println stampa sia numeri, che caratteri)	
		}else{
			// caricamento dei caratteri delle cifre del numero sul buffer
			while(Serial.available()){
				uint8_t curr = Serial.read();
				if((curr != '+') && (curr != '=')){
					buf[count++] = curr;
				}else{
					break;
				}
			}
		}
    }
}
```
>[Torna all'indice](indexseriale.md)
