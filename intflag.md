## **Variabili flag**

Una delle esigenze primarie di una ISR() è che sia molto veloce, per far ciò è opportuno che valgano entrambe le condizioni:
-	ISR corte ovvero con il minor numero di istruzioni possibile
-	Limitare al massimo l’inserimento di istruzioni lente all’interno di una ISR().

Ad esempio, le istruzioni di accesso ad una periferica sono mediamente molto più lente di quelle di accesso di una variabile in RAM 
D’altro canto però fare, nel programma principale, il polling continuo delle periferiche potrebbe comportare ritardi inutili se queste sono per la maggior parte del tempo inattive. Il polling periodico perlomeno andrebbe dilazionato con uno schedulamento poco frequente per renderlo meno costoso.
Invece, utilizzare gli interrupt delle periferiche per poterle interrogarle una tantum, all’interno di una ISR, potrebbe essere, in questo caso, una alternativa percorribile per velocizzare di molto l’esecuzione del programma principale (tecnica polled interrupt).

Le **variabili flag (bandierina)**, nel contesto degli interrupt, sono delle variabili globali comuni (cioè condivise) tra una ISR e il loop() principale che:
-	Vengono modificate dall’ISR a seguito di un interrupt
-	Vengono interrogate tramite polling nel loop() principale per determinare se si è completato un’evento secondario collegato all’interrupt della periferica.
-	Devono essere “azzerate” dopo essere state “consumate”. Cioè dopo essere state lette devono essere resettate al valore di default di evento non pronto (0, false,ecc.).

 
Il **polling di un flag** nel loop fa partire, nel programma principale, un task che è complementare a quello eseguito dalla ISR nel realizzare la gestione completa dell’evento di interrupt. Si ha adesso una **gestione mista** della periferica **polling-interrupt** che avviene in parte dentro la ISR e in parte dentro il loop principale.

Il polling in questo caso è una soluzione poco costosa perché la lettura periodica di una variabile in memoria è un’operazione molto veloce. Inoltre anche la lettura di una periferica “lenta” nel loop in questo caso sarebbe accettabile perché non verrebbe eseguita periodicamente ma, in maniera asincrona, solo quando il momento buono è segnalato da un flag (in pratica, una forma di polled interrupt, figura a sinistra).

L’interazione tra ISR e loop() mediante il flag può anche essere **bidirezionale**, cioè una modifica del flag fatta nel loop() può essere letta all’interno della ISR (figura a destra).

**Variabili volatili**

Le variabili condivise tra le funzioni ISR e le funzioni normali (come i flag), oltre ad essere eventualmente protette in scrittura da sezioni critiche, dovrebbero essere dichiarate "volatili". Questo dice al compilatore che tali variabili potrebbero cambiare in qualsiasi momento, e quindi il compilatore deve ricaricare la variabile ogni volta che si fa riferimento ad essa, piuttosto che fare affidamento su una copia che potrebbe avere in un registro del processore.

```C++
volatile boolean flag;

// Interrupt Service Routine (ISR)
void isr ()
{
 flag = true;
}  // end of isr

void setup ()
{
  attachInterrupt(digitalPinToInterrupt (2), isr, CHANGE);  
}  // end of setup

void loop ()
{
  if (flag)
    {
    // interrupt has occurred
    }
}  // end of loop
```

Per semplificare la vita agli sviluppatori, alcune ISR sono fornite direttamente dalle librerie di Arduino.  
Nell’esempio seguente viene adoperata la ISR serialEvent()che risponde all’evento arrivo di un nuovo carattere nella seriale e che può essere riempita con codice personalizzabile.

Nell’esempio la ISR viene utilizzata per creare una stringa, cioè una parola, unendo i singoli caratteri che man mano arrivano. La stringa è pronta, e quindi il flag viene attivato, quando arriva un carattere di fine linea “\n”.
Quando il loop principale si accorge del flag attivo ristampa in uscita la parola ottenuta tramite l’istruzione Serial.println.

```C++
string inputString = "";         // a String to hold incoming data
boolean volatile stringComplete = false;  // whether the string is complete
byte byteRead;

void setup() {                
    Serial.begin(9600);
    inputString.reserve(200);
}

loop(){
    // il polling su una variabile è meno lento di quello su una porta seriale 
    if (stringComplete) {
	stringComplete = false; //reset della bandierina (flag)
        Serial.println(inputString);
        // clear the string:
        inputString = "";
    }
}

// ISR (Interrupt Service Routine) che a seguito dell’arrivo di un dato. Crea  // una stringa da una sequenza di caratteri.
void serialEvent(){
  while (Serial.available()){
    // legge il nuovo byte:
    char inChar = (char) Serial.read();
    // lo aggiunge a inputString:
    inputString += inChar;
    // se il carattere in arrivo è un newline, 
    // imposta un flag in modo che il main loop 
    // possa utilizzarlo per rilevare l’evento di completamento di una stringa:
    if (inChar == '\n') {
        stringComplete = true;
    }
  }
}
```
