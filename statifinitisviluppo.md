
>[Torna all'indice](indexstatifiniti.md)
## **Sviluppo di una FSM**

### **Priorità stato/ingresso**

In linea generale, una **linea guida per** sviluppare una macchina a stati finiti potrebbe proporre **due strade**: 
- **Prima valutare gli ingressi** e poi gli stati:
    - scrivere tante funzioni (o blocchi di codice mutuamente esclusivi di una stessa funzione) quanti sono tutti i possibili ingressi
    - dentro ogni funzione (o blocco di codice) scrivere uno switch-case (o un blocco if-else-if) con tutti i soli stati raggiungibili dall’ingresso che essa rappresenta

- oppure, alternativamente, **prima valutare gli stati** e poi gli ingressi:
   - scrivere uno switch-case con tutti gli stati possibili del sistema
   - all’interno di ogni campo dello switch-case valutare soltanto tutti i possibili ingressi che potrebbero generare una transizione da quel particolare stato

### **Ingressi**

**In generale**, si possono considerare **ingressi**:
-	eventi generati dall’utente (input dati, pressione di un pulsante, ecc.)
-	eventi in risposta ad eventi generati dall’utente (invio di un messaggio su un canale, attivazione di un timer, ecc.) 
-	eventi generati dall’HW (interrupt che segnalano lo scadere di un timer, o l’arrivo di un dato sul canale)
-	polling di funzioni o di flag legati ad eventi HW o SW (verifica della disponibilità di dati in ricezione su un canale tramite Serial.available(), verifica dell’avvenuto scadere di un timeout tramite millis(), ecc.)

### **Stati**

Si possono considerare **stati**:
-	situazione in cui evolve un protocollo alla quale corrisponde un ben preciso messaggio
-	situazione in cui si trova l’applicazione alla quale corrisponde un preciso elenco di azioni possibili

### **Protocollo di comunicazione**

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

### **Attese**

Le **attese** ritardano le transizioni fino allo **scadere** di un certo **TIMEOUT** o in attesa di un **certo valore** su un **ingresso** o su un **flag di comunicazione** con un task/processo posto su di un altro thread. Possono essere, come sempre, **bloccanti** o **non bloccanti**:
- **```delay(TIMEOUT)```**, rappresenta un **timer bloccante** che attende per un tempo **```TIMEOUT```**.
- **```if(get() < TIMEOUT){...}```**, dopo uno start() precedente, rappresenta un **timer non bloccante** che attende per un tempo **```TIMEOUT```**.
- **```waitUntilInputLow(porta, tempo_mninimo)```**, rappresenta l'**attesa attiva bloccante** (polling) su un ingresso per un tempo minimo. Utile per separare un **prima**, fronte di salita alla **pressione** di un tasto, da un **dopo**, fronte di discesa al **rilascio** dello stesso tasto.
- **```waitUntil(flag)```**, rappresenta , rappresenta l'**attesa attiva bloccante** (polling) su un **flag** di segnalazione, in attesa che questo venga modificato da una istruzione su un altro **thread**.

### **Debug**

Se si sceglie, all'interno del codice, il criterio di organizzare prima la selezione degli stati e solo dopo, dentro ognuno di questi, la selezione degli ingressi che a loro competono, allora è possibile gestire la sua complessità in base ad alcune considerazioni:
- l'**analisi** del loop() può essere **compartimentata** stato per stato, cioè, si può esaminare il codice guardando, di volta in volta, il singolo blocco che è attivo in un certo stato. Il resto del codice che appartiene a stati diversi, pur essendo formalmente presente nel loop(), non viene eseguito in quel momento.
- quindi, per fare il **debug**, è importante sincerarsi con certezza dello **stato** in cui si trova la macchina, in modo da poter controllare, osservando il codice, che esegua correttamente le azioni previste per quello stato (**output**) in risposta a tutti gli **ingressi** che possono/non possono attivare **transizioni** su di esso.
- va valutata pure la correttezza delle **transizioni**, sia osservando **chi** le ha generate, sia controllando quale stato **raggiungono**. Il tutto deve essere **coerente** con la **tabella** delle trasnizioni e con il **diagramma** dele transizioni.
- alcuni **ingressi** non generano transizioni su certi stati. Vuol dire che non hanno effetto in quello stato e possono essere tralasciati. Nel grafo sono rappresentati con archi chiusi che tornano sullo stato stesso. Nel codice semplicemente non sono considerati in quello stato (ad esempio l'istruzione che legge la pressione di un certo pulsante).
  
>[Torna all'indice](indexstatifiniti.md) 
