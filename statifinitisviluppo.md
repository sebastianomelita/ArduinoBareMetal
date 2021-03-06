
>[Torna all'indice](indexstatifiniti.md)
## **Sviluppo di una FSM**

In linea generale, una **linea guida per** sviluppare una macchina a stati finiti potrebbe proporre **due strade**: 
- **Prima valutare gli ingressi** e poi gli stati:
    - scrivere tante funzioni (o blocchi di codice mutuamente esclusivi di una stessa funzione) quanti sono tutti i possibili ingressi
    - dentro ogni funzione (o blocco di codice) scrivere uno switch-case (o un blocco if-else-if) con tutti i soli stati raggiungibili dall’ingresso che essa rappresenta

- oppure, alternativamente, **prima valutare gli stati** e poi gli ingressi:
   - scrivere uno switch-case con tutti gli stati possibili del sistema
   - all’interno di ogni campo dello switch-case valutare soltanto tutti i possibili ingressi che potrebbero generare una transizione da quel particolare stato

**In generale**, si possono considerare **ingressi**:
-	eventi generati dall’utente (input dati, pressione di un pulsante, ecc.)
-	eventi in risposta ad eventi generati dall’utente (invio di un messaggio su un canale, attivazione di un timer, ecc.) 
-	eventi generati dall’HW (interrupt che segnalano lo scadere di un timer, o l’arrivo di un dato sul canale)
-	polling di funzioni o di flag legati ad eventi HW o SW (verifica della disponibilità di dati in ricezione su un canale tramite Serial.available(), verifica dell’avvenuto scadere di un timeout tramite millis(), ecc.)

Si possono considerare **stati**:
-	situazione in cui evolve un protocollo alla quale corrisponde un ben preciso messaggio
-	situazione in cui si trova l’applicazione alla quale corrisponde un preciso elenco di azioni possibili

Nello specifico di un **protocollo di comunicazione**:

Si possono considerare tipici **ingressi**:

-	nessun messaggio. In questo caso, presumibilmente, l’evento che si attende è lo scadere del timeout di un timer (timeout di trasmissione, o timeout di un backoff, ecc.)
-	l’evento arrivo di un messaggio (messaggio dati, messaggio di controllo, messaggio errato o incompleto, ecc.), valutato in base a:
    - lunghezza del messaggio
    - valutazione checksum sul messaggio
    - riconoscimento indirizzi del messaggio
    - riconoscimento di particolari campi o flag del messaggio (ack, nack, messaggio dati, messaggio controllo, ecc.)

Si possono considerare **stati**:
- situazione tipiche in cui evolve un protocollo alla ricezione di un certo messaggio:
  * stato di idle del canale (WAIT_STATE)
  * stato di attesa di ricezione di un messaggio di controllo entro lo scadere di un timeout (ACK_STATE, ecc.)
  * stato di attesa di trasmissione di un messaggio di controllo entro lo scadere di un timeout (BACKOFF_STATE, DIFS, ecc.)

>[Torna all'indice](indexstatifiniti.md) 
