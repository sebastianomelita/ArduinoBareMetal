
## **Sviluppo di una FSM**

In linea generale, una linea guida per sviluppare una macchina a stati finiti potrebbe proporre due strade:
Prima valutare gli ingressi e poi gli stati:
-	scrivere tante funzioni (o blocchi di codice mutuamente esclusivi di una stessa funzione) quanti sono tutti i possibili ingressi
-	dentro ogni funzione (o blocco di codice) scrivere uno switch-case (o un blocco if-else-if) con tutti i soli stati raggiungibili dall’ingresso che essa rappresenta

oppure, alternativamente, prima valutare gli stati e poi gli ingressi:
-	scrivere uno switch-case con tutti gli stati possibili del sistema
-	all’interno di ogni campo dello switch-case valutare soltanto tutti i possibili ingressi che potrebbero generare una transizione da quel particolare stato

In generale, si possono considerare ingressi:
-	eventi generati dall’utente (input dati, pressione di un pulsante, ecc.)
-	eventi in risposta ad eventi generati dall’utente (invio di un messaggio su un canale, attivazione di un timer, ecc.) 
-	eventi generati dall’HW (interrupt che segnalano lo scadere di un timer, o l’arrivo di un dato sul canale)
-	polling di funzioni o di flag legati ad eventi HW o SW (verifica della disponibilità di dati in ricezione su un canale tramite Serial.available(), verifica dell’avvenuto scadere di un timeout tramite millis(), ecc.)

Si possono considerare stati:
-	situazione in cui evolve un protocollo alle quali corrisponde un ben preciso messaggio
-	situazione in cui si trova l’applicazione alla quale corrisponde un preciso elenco di azioni possibili

Nello specifico di un protocollo di comunicazione:

Si possono considerare tipici ingressi:

-	nessun messaggio. In questo caso, presumibilmente, l’evento che si attende è lo scadere del timeout di un timer (timeout di trasmissione, o timeout di un backoff, ecc.)
-	l’evento arrivo di un messaggio (messaggio dati, messaggio di controllo, messaggio errato o incompleto, ecc.), valutato in base a:
  * lunghezza del messaggio
  * valutazione checksum sul messaggio
  * riconoscimento indirizzi del messaggio
  * riconoscimento di particolari campi o flag del messaggio (ack, nack, messaggio dati, messaggio controllo, ecc.)

si possono considerare stati:
- situazione tipiche in cui evolve un protocollo alla ricezione di un certo messaggio:
 * stato di idle del canale (WAIT_STATE)
 * stato di attesa di ricezione di un messaggio di controllo entro lo scadere di un timeout (ACK_STATE, ecc.)
 * stato di attesa di trasmissione di un messaggio di controllo entro lo scadere di un timeout (BACKOFF_STATE, DIFS, ecc.)

## **Esempio di un protocollo**

Un'altra applicazione comune per un FSM è la ricezione di un flusso di caratteri dalla porta seriale.
Per questo esempio supponiamo che un messaggio sia definito come caratteri ASCII leggibili tra un carattere iniziale ("<") e un carattere finale (">").
I caratteri ricevuti tramite una porta seriale potrebbero non essere tutti disponibili contemporaneamente, quindi la funzione dell'FSM è di memorizzare nel buffer tutti i caratteri validi fino alla ricezione del messaggio completo. Inoltre, se ci troviamo nel mezzo di un messaggio e non viene ricevuto alcun carattere per 1 secondo, il messaggio scade e l'FSM dovrebbe risincronizzarsi all'inizio del messaggio successivo.

Possiamo suddividere l'FSM nelle seguenti fasi:
1.	Attesa del carattere di start (<).
2.	Buffering di tutti i caratteri fino a quando non viene rilevato il carattere finale (>).
3.	Se nessun carattere viene ricevuto in 1 secondo, torna ad aspettare il carattere iniziale.
4.	Quando viene ricevuto il carattere finale, elaborare il messaggio. In questo esempio, stamperemo solo il testo.

La FSM è implementata come processInput() nel codice a seguire. La funzione restituisce true quando il messaggio è stato ricevuto in modo che il chiamante possa comportarsi di conseguenza.
Un paio di note su questo codice:
 -	E il carattere vuoto (nul o "\ 0") indica che non è stato ricevuto alcun carattere dalla porta seriale. È sicuro in questa situazione perché ci aspettiamo caratteri leggibili, ma in generale un byte zero potrebbe essere valido nel flusso di input. Viene utilizzato in vari punti, in particolare per limitare la quantità di debug prodotta dallo schizzo.
 -	Il carattere seriale viene ricevuto nella parte di codice comune prima dell'istruzione switch. Ciò semplifica l'aggiornamento consistente del timer tra un carattere e l’altro, ma comporta un controllo "speciale" aggiuntivo per non leggere un carattere quando lo stato corrente è PROCESS_MESG poiché poi quel carattere andrebbe perso. Il controllo speciale può essere evitato non avendo lo stato PROCESS_MESG ed elaborando il messaggio in BUFFERING (tutto subito con un ciclo while (Serial.available() ){…} e non un carattere alla volta) non appena viene ricevuto. Funzionano entrambe le soluzioni, è solo una questione di preferenze.
 -	Lo stato BUFFERING è relativamente complesso in quanto deve gestire il timeout, l'overflow del buffer, il rilevamento della fine del messaggio e il buffering dei caratteri.

```C++
#define DEBUG 1
#if DEBUG
  #define FSM_STATE(s) { Serial.print(F("n:")); Serial.print(F(s)); Serial.print(F(": ")); Serial.print(c); }
  #define FSM_MESG(s) { Serial.print(F("n->")); Serial.print(F(s)); Serial.print(F(": ")); Serial.print(cBuf); }
#else
  #define FSM_STATE(s)
  #define FSM_MESG(s)
#endif
  
bool processInput(bool reset = false)
{
  const uint32_t TIMEOUT_DELAY = 1000;
  const uint16_t MAX_LEN = 20;
  const char CH_START = '<';   const char CH_END = '>';
 
  static char cBuf[MAX_LEN+1] = "";
  static uint16_t lenBuf = 0;
  static enum { WAIT_START, BUFFERING, PROCESS_MESG } state = WAIT_START;
  static uint32_t timeLastChar = 0;
  char c = '\0';
  bool b = false;
 
  if (reset) state = WAIT_START;
  // read the next char if there is one
  // don't read if we are about to process the message
  if (state != PROCESS_MESG)
  {
    if (Serial.available())
    {
      c = Serial.read();
      timeLastChar = millis();
    }
  }
  
  // process the char based on state
  switch (state)
  {
    case WAIT_START:   // waiting for start character
      if (c != '\0')
        FSM_STATE("START");

      if (c == CH_START)
      {
         state = BUFFERING;
         lenBuf = 0;
         memset(cBuf, '\0', sizeof(char)*(MAX_LEN+1));  // clear the buffer
      }
    break;
     case BUFFERING:   // buffer up all the characters
      if (c != '\0')
        FSM_STATE("BUFFER"); 
 
      // have we timed out?
      if (millis() - timeLastChar >= TIMEOUT_DELAY)
      { 
        FSM_MESG("Timeout");
        state = WAIT_START;
        break;
      }
   
      // process the character
      if (c == CH_END)
        state = PROCESS_MESG;
      else if (c != '\0')   // not empty
      {
        cBuf[lenBuf++] = c; 
        if (lenBuf == MAX_LEN)   // buffer overflow
        {
          state = WAIT_START;
          FSM_MESG("Overflow");
        }
      }
    break;
    case PROCESS_MESG:     // do something with the message
      FSM_STATE("PROCESS_MESG");
      FSM_MESG("Received");
      state = WAIT_START;
      b = true;
    break;
 
    default:
      state = WAIT_START;
      break;
   }
 return(b);
 }
 
void setup(void) 
{
  Serial.begin(9600);
}
 
void loop(void) 
{
  if (processInput())
    Serial.print("\nYIPPEE");
  // do something else
}
```
